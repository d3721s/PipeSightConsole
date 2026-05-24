#include "laser_viewmodel.h"

namespace pipesight::viewmodels {

LaserViewModel::LaserViewModel(QObject *parent) : QObject(parent)
{
    connect(&service_, &services::LaserService::markerAdded,
            this, [this](const services::LaserService::Marker &) {
                ++markerCount_; emit markerCountChanged();
            });
    connect(&service_, &services::LaserService::markerRemoved,
            this, [this](int) {
                if (markerCount_ > 0) { --markerCount_; emit markerCountChanged(); }
            });
}

LaserViewModel::~LaserViewModel() = default;

void LaserViewModel::setPointCloudEnabled(bool on)
{
    if (pointCloud_ == on) return;
    pointCloud_ = on;
    service_.setPointCloudEnabled(on);
    emit pointCloudEnabledChanged();
}

} // namespace pipesight::viewmodels
