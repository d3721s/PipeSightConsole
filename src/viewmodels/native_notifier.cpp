#include "native_notifier.h"

#include <QCoreApplication>

#ifdef Q_OS_ANDROID
#include <QtCore/QJniObject>
#include <QtCore/qcoreapplication_platform.h>
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

QJniObject androidContext()
{
    return QNativeInterface::QAndroidApplication::context();
}
#endif

} // namespace

NativeNotifier::NativeNotifier(QObject *parent) : QObject(parent) {}
NativeNotifier::~NativeNotifier() = default;

bool NativeNotifier::available() const
{
#ifdef Q_OS_ANDROID
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
#else
    emit notificationFailed(QStringLiteral("当前平台不支持 Android 原生通知"));
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
#else
    return false;
#endif
}

bool NativeNotifier::notify(const QString &title, const QString &message)
{
    const QString safeTitle = title.trimmed().isEmpty() ? fallbackTitle() : title.trimmed();
    const QString safeMessage = message.trimmed();

#ifdef Q_OS_ANDROID
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
        nextNotificationId());

    if (!delivered)
        emit notificationFailed(QStringLiteral("系统通知未发送，请检查 Android 通知权限"));
    return delivered;
#else
    emit notificationFailed(fallbackMessage(safeTitle, safeMessage));
    return false;
#endif
}

int NativeNotifier::nextNotificationId()
{
    if (nextId_ == 999999)
        nextId_ = 1000;
    return nextId_++;
}

} // namespace pipesight::viewmodels
