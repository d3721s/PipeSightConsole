#include "osd_service.h"
#include "data/app_settings.h"

#include <QtGlobal>

namespace pipesight::services {

using pipesight::data::AppSettings;

namespace {

constexpr auto kShowProjectInfo = "osd/showProjectInfo";
constexpr auto kShowTime = "osd/showTime";
constexpr auto kShowPosition = "osd/showPosition";
constexpr auto kProjectName = "osd/projectName";
constexpr auto kInspectionUnit = "osd/inspectionUnit";
constexpr auto kFontFamily = "osd/fontFamily";
constexpr auto kFontSize = "osd/fontSize";
constexpr auto kPosition = "osd/position";
constexpr auto kRefreshMs = "osd/refreshMs";

constexpr int kDefaultFontSize = 24;
constexpr int kDefaultPosition = 0;
constexpr int kDefaultRefreshMs = 1000;

QString normalizedFontFamily(QString family)
{
    family = family.trimmed();
    return family.isEmpty() ? QStringLiteral("Microsoft YaHei") : family;
}

int clampedFontSize(int size)
{
    return qBound(12, size, 96);
}

int clampedPosition(int position)
{
    return qBound(0, position, 3);
}

int clampedRefreshMs(int ms)
{
    return qBound(250, ms, 10000);
}

} // namespace

OsdService &OsdService::instance()
{
    static OsdService service;
    return service;
}

OsdService::OsdService(QObject *parent) : QObject(parent)
{
    loadSettings();

    connect(&AppSettings::instance(), &AppSettings::valueChanged,
            this, [this](const QString &key) {
                if (!key.startsWith(QStringLiteral("osd/"))) return;

                const bool oldShowProject = showProject_;
                const bool oldShowTime = showTime_;
                const bool oldShowPosition = showPosition_;
                const QString oldProjectName = projectName_;
                const QString oldInspectionUnit = inspectionUnit_;
                const QString oldFontFamily = fontFamily_;
                const int oldFontSize = fontSize_;
                const int oldPosition = position_;
                const int oldRefreshMs = refreshMs_;

                loadSettings();
                if (oldShowProject != showProject_ ||
                    oldShowTime != showTime_ ||
                    oldShowPosition != showPosition_ ||
                    oldProjectName != projectName_ ||
                    oldInspectionUnit != inspectionUnit_ ||
                    oldFontFamily != fontFamily_ ||
                    oldFontSize != fontSize_ ||
                    oldPosition != position_ ||
                    oldRefreshMs != refreshMs_) {
                    emit osdChanged();
                }
            });
}
OsdService::~OsdService() = default;

void OsdService::setShowProjectInfo(bool on)
{
    if (showProject_ == on) return;
    showProject_ = on;
    AppSettings::instance().setValue(QString::fromLatin1(kShowProjectInfo), on);
    emit osdChanged();
}

void OsdService::setShowTime(bool on)
{
    if (showTime_ == on) return;
    showTime_ = on;
    AppSettings::instance().setValue(QString::fromLatin1(kShowTime), on);
    emit osdChanged();
}

void OsdService::setShowPosition(bool on)
{
    if (showPosition_ == on) return;
    showPosition_ = on;
    AppSettings::instance().setValue(QString::fromLatin1(kShowPosition), on);
    emit osdChanged();
}

void OsdService::setProjectName(const QString &name)
{
    if (projectName_ == name) return;
    projectName_ = name;
    AppSettings::instance().setValue(QString::fromLatin1(kProjectName), name);
    emit osdChanged();
}

void OsdService::setInspectionUnit(const QString &unit)
{
    if (inspectionUnit_ == unit) return;
    inspectionUnit_ = unit;
    AppSettings::instance().setValue(QString::fromLatin1(kInspectionUnit), unit);
    emit osdChanged();
}

void OsdService::setFontFamily(const QString &family)
{
    const QString value = normalizedFontFamily(family);
    if (fontFamily_ == value) return;
    fontFamily_ = value;
    AppSettings::instance().setValue(QString::fromLatin1(kFontFamily), value);
    emit osdChanged();
}

void OsdService::setFontSize(int size)
{
    const int value = clampedFontSize(size);
    if (fontSize_ == value) return;
    fontSize_ = value;
    AppSettings::instance().setValue(QString::fromLatin1(kFontSize), value);
    emit osdChanged();
}

void OsdService::setPosition(int position)
{
    const int value = clampedPosition(position);
    if (position_ == value) return;
    position_ = value;
    AppSettings::instance().setValue(QString::fromLatin1(kPosition), value);
    emit osdChanged();
}

void OsdService::setRefreshMs(int ms)
{
    const int value = clampedRefreshMs(ms);
    if (refreshMs_ == value) return;
    refreshMs_ = value;
    AppSettings::instance().setValue(QString::fromLatin1(kRefreshMs), value);
    emit osdChanged();
}

void OsdService::loadSettings()
{
    auto &s = AppSettings::instance();
    showProject_ = s.value(QString::fromLatin1(kShowProjectInfo), true).toBool();
    showTime_ = s.value(QString::fromLatin1(kShowTime), true).toBool();
    showPosition_ = s.value(QString::fromLatin1(kShowPosition), true).toBool();
    projectName_ = s.value(QString::fromLatin1(kProjectName)).toString();
    inspectionUnit_ = s.value(QString::fromLatin1(kInspectionUnit)).toString();
    fontFamily_ = normalizedFontFamily(s.value(QString::fromLatin1(kFontFamily),
                                               QStringLiteral("Microsoft YaHei")).toString());
    fontSize_ = clampedFontSize(s.value(QString::fromLatin1(kFontSize), kDefaultFontSize).toInt());
    position_ = clampedPosition(s.value(QString::fromLatin1(kPosition), kDefaultPosition).toInt());
    refreshMs_ = clampedRefreshMs(s.value(QString::fromLatin1(kRefreshMs), kDefaultRefreshMs).toInt());
}

} // namespace pipesight::services
