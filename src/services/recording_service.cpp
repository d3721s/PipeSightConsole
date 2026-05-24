#include "recording_service.h"
#include <QStandardPaths>

namespace pipesight::services {

RecordingService::RecordingService(QObject *parent)
    : QObject(parent)
    , storagePath_(QStandardPaths::writableLocation(QStandardPaths::MoviesLocation))
{}

RecordingService::~RecordingService() = default;

void RecordingService::startRecording()
{
    if (recording_) return;
    // TODO: bind QMediaRecorder to the active video source from CameraService
    recording_ = true;
    emit recordingStateChanged(true);
}

void RecordingService::stopRecording()
{
    if (!recording_) return;
    recording_ = false;
    emit recordingStateChanged(false);
}

void RecordingService::takeSnapshot()
{
    // TODO: capture current frame from CameraService's QVideoSink and save as JPEG
    emit snapshotSaved(storagePath_ + QStringLiteral("/snapshot.jpg"));
}

void RecordingService::setResolution(int w, int h) { width_ = w; height_ = h; }
void RecordingService::setCodec(Codec c)           { codec_ = c; }
void RecordingService::setSegmentMinutes(int m)    { segmentMinutes_ = m; }
void RecordingService::setStoragePath(const QString &p) { storagePath_ = p; }
void RecordingService::setCyclicEnabled(bool on)   { cyclic_ = on; }

} // namespace pipesight::services
