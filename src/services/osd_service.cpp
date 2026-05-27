#include "osd_service.h"
#include "data/app_settings.h"

namespace pipesight::services {

using pipesight::data::AppSettings;

namespace {

constexpr auto kShowProjectInfo = "osd/showProjectInfo";
constexpr auto kShowTime = "osd/showTime";
constexpr auto kShowPosition = "osd/showPosition";
constexpr auto kProjectName = "osd/projectName";
constexpr auto kInspectionUnit = "osd/inspectionUnit";

} // namespace

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

                loadSettings();
                if (oldShowProject != showProject_ ||
                    oldShowTime != showTime_ ||
                    oldShowPosition != showPosition_ ||
                    oldProjectName != projectName_ ||
                    oldInspectionUnit != inspectionUnit_) {
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

void OsdService::loadSettings()
{
    auto &s = AppSettings::instance();
    showProject_ = s.value(QString::fromLatin1(kShowProjectInfo), true).toBool();
    showTime_ = s.value(QString::fromLatin1(kShowTime), true).toBool();
    showPosition_ = s.value(QString::fromLatin1(kShowPosition), true).toBool();
    projectName_ = s.value(QString::fromLatin1(kProjectName)).toString();
    inspectionUnit_ = s.value(QString::fromLatin1(kInspectionUnit)).toString();
}

} // namespace pipesight::services
