#include "camera_viewmodel.h"

namespace pipesight::viewmodels {

CameraViewModel::CameraViewModel(QObject *parent)
    : QObject(parent)
{
    connect(&service_, &services::CameraService::activeChannelChanged,
            this, &CameraViewModel::activeChannelChanged);
    connect(&service_, &services::CameraService::streamUrlChanged,
            this, &CameraViewModel::streamUrlChanged);
}

CameraViewModel::~CameraViewModel() = default;

QUrl CameraViewModel::streamUrl() const
{
    return service_.streamUrl(service_.activeChannel());
}

void CameraViewModel::startRecording()
{
    // TODO: route to RecordingService once it's wired into AppContext
    if (!recording_) { recording_ = true; emit recordingChanged(); }
}

void CameraViewModel::stopRecording()
{
    if (recording_) { recording_ = false; emit recordingChanged(); }
}

void CameraViewModel::snapshot()
{
    // TODO: route to RecordingService::takeSnapshot()
}

} // namespace pipesight::viewmodels
