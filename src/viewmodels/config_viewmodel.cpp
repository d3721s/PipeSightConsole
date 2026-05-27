#include "config_viewmodel.h"
#include "data/app_settings.h"

#include <QDir>

namespace pipesight::viewmodels {

using pipesight::data::AppSettings;

namespace {

QVariant defaultStereoParam(const QString &key)
{
    if (key == QStringLiteral("exposure")) return 50;
    if (key == QStringLiteral("whiteBalance")) return QStringLiteral("Auto");
    if (key == QStringLiteral("sync")) return false;
    return {};
}

QVariant defaultRadarParam(const QString &key)
{
    if (key == QStringLiteral("scanHz")) return 10;
    if (key == QStringLiteral("angleDeg")) return 270;
    if (key == QStringLiteral("rangeM")) return 30;
    return {};
}

} // namespace

ConfigViewModel::ConfigViewModel(QObject *parent) : QObject(parent) {}
ConfigViewModel::~ConfigViewModel() = default;

void ConfigViewModel::setSegmentMinutes(int m)
{
    if (recording_.segmentMinutes() == m) return;
    recording_.setSegmentMinutes(m);
    emit configChanged();
}

void ConfigViewModel::setCyclicRecord(bool on)
{
    if (recording_.cyclicEnabled() == on) return;
    recording_.setCyclicEnabled(on);
    emit configChanged();
}

void ConfigViewModel::setStoragePath(const QString &p)
{
    if (recording_.storagePath() == p) return;
    recording_.setStoragePath(p);
    emit configChanged();
}

bool ConfigViewModel::applyRecordingConfig(int segmentMinutes, bool cyclicRecord, const QString &storagePath)
{
    const QString path = storagePath.trimmed();
    if (path.isEmpty())
        return false;
    if (!QDir().mkpath(path))
        return false;

    setSegmentMinutes(segmentMinutes);
    setCyclicRecord(cyclicRecord);
    setStoragePath(path);
    return true;
}

void ConfigViewModel::configureCamera(int channel, const QString &ip,
                                      const QString &main,
                                      const QString &sub,
                                      const QString &third)
{
    emit cameraConfigured(channel, ip, main, sub, third);
}

QVariant ConfigViewModel::stereoParam(const QString &key) const
{
    return AppSettings::instance().value(QStringLiteral("stereo/") + key, defaultStereoParam(key));
}

QVariant ConfigViewModel::radarParam(const QString &key) const
{
    return AppSettings::instance().value(QStringLiteral("radar/") + key, defaultRadarParam(key));
}

void ConfigViewModel::setStereoParam(const QString &key, const QVariant &v)
{
    AppSettings::instance().setValue(QStringLiteral("stereo/") + key, v);
    emit stereoParamChanged(key, v);
}

void ConfigViewModel::setRadarParam(const QString &key, const QVariant &v)
{
    AppSettings::instance().setValue(QStringLiteral("radar/") + key, v);
    emit radarParamChanged(key, v);
}

bool ConfigViewModel::readStereoParams()
{
    return false;
}

bool ConfigViewModel::applyStereoParams(int exposure, const QString &whiteBalance, bool sync)
{
    Q_UNUSED(exposure);
    Q_UNUSED(whiteBalance);
    Q_UNUSED(sync);
    return false;
}

bool ConfigViewModel::readRadarParams()
{
    return false;
}

bool ConfigViewModel::applyRadarParams(int scanHz, int angleDeg, int rangeM)
{
    Q_UNUSED(scanHz);
    Q_UNUSED(angleDeg);
    Q_UNUSED(rangeM);
    return false;
}

} // namespace pipesight::viewmodels
