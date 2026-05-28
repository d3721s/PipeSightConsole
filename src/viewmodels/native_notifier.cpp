#include "native_notifier.h"

#include <QCoreApplication>

#ifdef Q_OS_ANDROID
#include <QtCore/QJniObject>
#include <QtCore/qcoreapplication_platform.h>
#endif

#ifdef Q_OS_WIN
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <qt_windows.h>
#include <shellapi.h>
#include <cwchar>
#include <string>
#endif

namespace pipesight::viewmodels {

namespace {

QString fallbackTitle()
{
    const QString appName = QCoreApplication::applicationName();
    return appName.isEmpty() ? QStringLiteral("PipeSight Console") : appName;
}

QString fallbackMessage(const QString &title, const QString &message)
{
    if (message.trimmed().isEmpty())
        return title;
    return QStringLiteral("%1: %2").arg(title, message);
}

#ifdef Q_OS_ANDROID
constexpr auto kNativeNotificationClass = "org/pipesight/console/NativeNotification";
constexpr int kAndroidOngoingNotificationId = 1000;

QJniObject androidContext()
{
    return QNativeInterface::QAndroidApplication::context();
}
#endif

#ifdef Q_OS_WIN
constexpr UINT kTrayIconId = 1;
constexpr UINT kTrayCallbackMessage = WM_APP + 0x512;

template <size_t N>
void copyWide(wchar_t (&target)[N], const QString &source)
{
    const std::wstring text = source.left(static_cast<int>(N - 1)).toStdWString();
    std::wmemset(target, 0, N);
    std::wmemcpy(target, text.c_str(), text.size());
}

HWND notificationWindow()
{
    static HWND window = nullptr;
    if (window)
        return window;

    const wchar_t className[] = L"PipeSightNativeNotifierWindow";
    HINSTANCE instance = GetModuleHandleW(nullptr);

    WNDCLASSW windowClass{};
    windowClass.lpfnWndProc = DefWindowProcW;
    windowClass.hInstance = instance;
    windowClass.lpszClassName = className;

    if (!RegisterClassW(&windowClass) && GetLastError() != ERROR_CLASS_ALREADY_EXISTS)
        return nullptr;

    window = CreateWindowExW(0, className, L"PipeSight Native Notifier",
                             0, 0, 0, 0, 0, HWND_MESSAGE, nullptr, instance, nullptr);
    return window;
}

bool ensureTrayIcon(HWND window)
{
    static bool trayIconAdded = false;
    if (trayIconAdded)
        return true;

    NOTIFYICONDATAW data{};
    data.cbSize = sizeof(data);
    data.hWnd = window;
    data.uID = kTrayIconId;
    data.uFlags = NIF_MESSAGE | NIF_ICON | NIF_TIP;
    data.uCallbackMessage = kTrayCallbackMessage;
    data.hIcon = LoadIconW(nullptr, IDI_APPLICATION);
    copyWide(data.szTip, fallbackTitle());

    trayIconAdded = Shell_NotifyIconW(NIM_ADD, &data);
    return trayIconAdded;
}

bool showWindowsNotification(const QString &title, const QString &message, bool isError)
{
    HWND window = notificationWindow();
    if (!window || !ensureTrayIcon(window))
        return false;

    NOTIFYICONDATAW data{};
    data.cbSize = sizeof(data);
    data.hWnd = window;
    data.uID = kTrayIconId;
    data.uFlags = NIF_INFO;
    data.dwInfoFlags = isError ? NIIF_ERROR : NIIF_INFO;
    data.uTimeout = 5000;
    copyWide(data.szInfoTitle, title);
    copyWide(data.szInfo, message);

    return Shell_NotifyIconW(NIM_MODIFY, &data);
}
#endif

} // namespace

NativeNotifier::NativeNotifier(QObject *parent) : QObject(parent) {}
NativeNotifier::~NativeNotifier() = default;

bool NativeNotifier::available() const
{
#if defined(Q_OS_ANDROID) || defined(Q_OS_WIN)
    return true;
#else
    return false;
#endif
}

bool NativeNotifier::requestPermission()
{
#ifdef Q_OS_ANDROID
    const QJniObject context = androidContext();
    if (!context.isValid()) {
        emit notificationFailed(QStringLiteral("Android 上下文不可用，无法请求系统通知权限"));
        return false;
    }

    return QJniObject::callStaticMethod<jboolean>(
        kNativeNotificationClass,
        "requestPermission",
        "(Landroid/content/Context;)Z",
        context.object<jobject>());
#elif defined(Q_OS_WIN)
    return true;
#else
    emit notificationFailed(QStringLiteral("当前平台不支持原生通知"));
    return false;
#endif
}

bool NativeNotifier::notificationsEnabled() const
{
#ifdef Q_OS_ANDROID
    const QJniObject context = androidContext();
    if (!context.isValid())
        return false;

    return QJniObject::callStaticMethod<jboolean>(
        kNativeNotificationClass,
        "notificationsEnabled",
        "(Landroid/content/Context;)Z",
        context.object<jobject>());
#elif defined(Q_OS_WIN)
    return true;
#else
    return false;
#endif
}

bool NativeNotifier::notify(const QString &title, const QString &message)
{
    return sendNotification(title, message, false);
}

bool NativeNotifier::notifyError(const QString &title, const QString &message)
{
    return sendNotification(title, message, true);
}

bool NativeNotifier::sendNotification(const QString &title, const QString &message, bool isError)
{
    const QString safeTitle = title.trimmed().isEmpty() ? fallbackTitle() : title.trimmed();
    const QString safeMessage = message.trimmed();

#ifdef Q_OS_ANDROID
    Q_UNUSED(isError);

    const QJniObject context = androidContext();
    if (!context.isValid()) {
        emit notificationFailed(QStringLiteral("Android 上下文不可用，系统通知未发送"));
        return false;
    }

    const QJniObject jTitle = QJniObject::fromString(safeTitle);
    const QJniObject jMessage = QJniObject::fromString(safeMessage);
    const jboolean delivered = QJniObject::callStaticMethod<jboolean>(
        kNativeNotificationClass,
        "show",
        "(Landroid/content/Context;Ljava/lang/String;Ljava/lang/String;I)Z",
        context.object<jobject>(),
        jTitle.object<jstring>(),
        jMessage.object<jstring>(),
        kAndroidOngoingNotificationId);

    if (!delivered)
        emit notificationFailed(QStringLiteral("系统通知未发送，请检查 Android 通知权限"));
    return delivered;
#elif defined(Q_OS_WIN)
    const bool delivered = showWindowsNotification(safeTitle, safeMessage, isError);
    if (!delivered)
        emit notificationFailed(fallbackMessage(safeTitle, safeMessage));
    return delivered;
#else
    Q_UNUSED(isError);
    emit notificationFailed(fallbackMessage(safeTitle, safeMessage));
    return false;
#endif
}

} // namespace pipesight::viewmodels
