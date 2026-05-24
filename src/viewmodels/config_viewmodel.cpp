#include "config_viewmodel.h"

namespace pipesight::viewmodels {

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

void ConfigViewModel::configureCamera(int channel, const QString &ip,
                                      const QString &main,
                                      const QString &sub,
                                      const QString &third)
{
    emit cameraConfigured(channel, ip, main, sub, third);
}

void ConfigViewModel::setStereoParam(const QString &key, const QVariant &v)
{
    emit stereoParamChanged(key, v);
}

void ConfigViewModel::setRadarParam(const QString &key, const QVariant &v)
{
    emit radarParamChanged(key, v);
}

} // namespace pipesight::viewmodels
