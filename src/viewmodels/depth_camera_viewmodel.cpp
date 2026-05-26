#include "depth_camera_viewmodel.h"

namespace pipesight::viewmodels {

DepthCameraViewModel::DepthCameraViewModel(QObject *parent) : QObject(parent)
{
    connect(&service_, &services::DepthCameraService::depthMarkerAdded,
            this, [this](const services::DepthCameraService::DepthMarker &) {
                ++markerCount_; emit depthMarkerCountChanged();
            });
    connect(&service_, &services::DepthCameraService::depthMarkerRemoved,
            this, [this](int) {
                if (markerCount_ > 0) { --markerCount_; emit depthMarkerCountChanged(); }
            });
}

DepthCameraViewModel::~DepthCameraViewModel() = default;

void DepthCameraViewModel::setDepthCloudEnabled(bool on)
{
    if (depthCloud_ == on) return;
    depthCloud_ = on;
    service_.setDepthCloudEnabled(on);
    emit depthCloudEnabledChanged();
}

} // namespace pipesight::viewmodels
