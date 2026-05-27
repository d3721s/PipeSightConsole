#pragma once

#include <QObject>
#include <QString>
#include <QTimer>
#include <QUrl>

#include "services/osd_renderer.h"
#include "services/vehicle_service.h"

class QProcess;

namespace pipesight::services {

/**
 * Recording configuration + lifecycle.
 *
 * Owns the writer (Qt Multimedia QMediaRecorder or FFmpeg-based) and
 * exposes file/segment policy. Snapshots also flow through here so that
 * markers can reference the file timestamp/segment.
 */
class RecordingService : public QObject
{
    Q_OBJECT
public:
    enum class Codec { H264, H265, MJPEG };
    Q_ENUM(Codec)

    explicit RecordingService(QObject *parent = nullptr);
    ~RecordingService() override;

    bool    isRecording()        const { return recording_; }
    QString storagePath()        const { return storagePath_; }
    int     segmentMinutes()     const { return segmentMinutes_; }
    bool    cyclicEnabled()      const { return cyclic_; }

public slots:
    void startRecording();
    void startRecording(const QUrl &sourceUrl);
    void stopRecording();
    void takeSnapshot();
    void takeSnapshot(const QUrl &sourceUrl);

    void setResolution(int width, int height);
    void setCodec(Codec c);
    void setSegmentMinutes(int minutes);
    void setStoragePath(const QString &path);
    void setCyclicEnabled(bool on);

signals:
    void recordingStateChanged(bool on);
    void snapshotSaved(const QString &filePath);
    void segmentRolled(const QString &filePath);
    void errorOccurred(const QString &msg);

private:
    void loadSettings();
    bool updateRecordingOsdText();
    OsdTelemetry currentOsdTelemetry() const;
    QString buildOutputFilePath() const;
    QString buildSnapshotFilePath() const;
    QStringList buildFfmpegArguments(const QUrl &sourceUrl, const QString &outputFile) const;

    bool    recording_       = false;
    QString storagePath_;
    QString outputFile_;
    QString osdTextFile_;
    int     segmentMinutes_  = 30;
    bool    cyclic_          = true;
    int     width_  = 1920;
    int     height_ = 1080;
    Codec   codec_  = Codec::H264;

    QProcess      *ffmpeg_ = nullptr;
    QTimer         osdTimer_;
    OsdRenderer    osdRenderer_;
    VehicleService &vehicle_;
};

} // namespace pipesight::services
