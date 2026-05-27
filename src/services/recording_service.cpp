#include "recording_service.h"
#include "data/app_settings.h"

#include <QDateTime>
#include <QDir>
#include <QFileInfo>
#include <QProcess>
#include <QStandardPaths>
#include <QStringList>

namespace pipesight::services {

using pipesight::data::AppSettings;

namespace {

constexpr auto kSegmentMinutes = "recording/segmentMinutes";
constexpr auto kStoragePath = "recording/storagePath";
constexpr auto kCyclicEnabled = "recording/cyclicEnabled";

} // namespace

RecordingService::RecordingService(QObject *parent)
    : QObject(parent)
    , storagePath_(QStandardPaths::writableLocation(QStandardPaths::MoviesLocation))
    , vehicle_(VehicleService::instance())
{
    loadSettings();

    osdTimer_.setInterval(1000);
    connect(&osdTimer_, &QTimer::timeout, this, [this]() {
        updateRecordingOsdText();
    });

    connect(&AppSettings::instance(), &AppSettings::valueChanged,
            this, [this](const QString &key) {
                if (key.startsWith(QStringLiteral("recording/")))
                    loadSettings();
            });
}

RecordingService::~RecordingService()
{
    stopRecording();
}

void RecordingService::startRecording()
{
    emit errorOccurred(QStringLiteral("未提供录像视频源"));
}

void RecordingService::startRecording(const QUrl &sourceUrl)
{
    if (recording_) return;
    if (!sourceUrl.isValid() || sourceUrl.isEmpty()) {
        emit errorOccurred(QStringLiteral("RTSP 视频源为空，无法录像"));
        return;
    }

    const QString ffmpeg = QStandardPaths::findExecutable(QStringLiteral("ffmpeg"));
    if (ffmpeg.isEmpty()) {
        emit errorOccurred(QStringLiteral("未找到 ffmpeg.exe，无法烧录 OSD 录像"));
        return;
    }

    loadSettings();

    outputFile_ = buildOutputFilePath();
    osdTextFile_ = OsdRenderer::buildTextFilePath(QStringLiteral("recording_osd"));
    const QString outputDir = QFileInfo(outputFile_).absolutePath();
    if (!QDir().mkpath(outputDir)) {
        emit errorOccurred(QStringLiteral("无法创建录像存储目录：%1").arg(outputDir));
        return;
    }
    if (!updateRecordingOsdText())
        return;

    auto *proc = new QProcess(this);
    proc->setProgram(ffmpeg);
    proc->setArguments(buildFfmpegArguments(sourceUrl, outputFile_));
    proc->setProcessChannelMode(QProcess::SeparateChannels);

    connect(proc, &QProcess::readyReadStandardError, proc, [proc]() {
        proc->readAllStandardError();
    });
    connect(proc, &QProcess::readyReadStandardOutput, proc, [proc]() {
        proc->readAllStandardOutput();
    });
    connect(proc, &QProcess::errorOccurred, this, [this](QProcess::ProcessError) {
        emit errorOccurred(QStringLiteral("FFmpeg 录像进程启动失败"));
    });
    connect(proc, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this, [this, proc](int, QProcess::ExitStatus) {
                osdTimer_.stop();
                const bool wasRecording = recording_;
                recording_ = false;
                ffmpeg_ = nullptr;
                proc->deleteLater();
                if (wasRecording) {
                    emit recordingStateChanged(false);
                    emit segmentRolled(outputFile_);
                }
            });

    ffmpeg_ = proc;
    proc->start();
    if (!proc->waitForStarted(3000)) {
        ffmpeg_ = nullptr;
        proc->deleteLater();
        emit errorOccurred(QStringLiteral("FFmpeg 录像进程启动超时"));
        return;
    }

    recording_ = true;
    osdTimer_.start();
    emit recordingStateChanged(true);
}

void RecordingService::stopRecording()
{
    if (!recording_ && !ffmpeg_) return;

    osdTimer_.stop();

    QProcess *proc = ffmpeg_;
    if (proc && proc->state() != QProcess::NotRunning) {
        proc->write("q");
        proc->closeWriteChannel();
        if (!proc->waitForFinished(3000)) {
            proc->terminate();
            if (!proc->waitForFinished(1500))
                proc->kill();
        }
    }

    if (recording_) {
        recording_ = false;
        emit recordingStateChanged(false);
    }
}

void RecordingService::takeSnapshot()
{
    // TODO: capture current frame from CameraService's QVideoSink and save as JPEG
    emit snapshotSaved(storagePath_ + QStringLiteral("/snapshot.jpg"));
}

void RecordingService::setResolution(int w, int h)
{
    width_ = w;
    height_ = h;
}

void RecordingService::setCodec(Codec c)
{
    codec_ = c;
}

void RecordingService::setSegmentMinutes(int m)
{
    m = qMax(1, m);
    if (segmentMinutes_ == m) return;
    segmentMinutes_ = m;
    AppSettings::instance().setValue(QString::fromLatin1(kSegmentMinutes), m);
}

void RecordingService::setStoragePath(const QString &p)
{
    if (storagePath_ == p) return;
    storagePath_ = p;
    AppSettings::instance().setValue(QString::fromLatin1(kStoragePath), p);
}

void RecordingService::setCyclicEnabled(bool on)
{
    if (cyclic_ == on) return;
    cyclic_ = on;
    AppSettings::instance().setValue(QString::fromLatin1(kCyclicEnabled), on);
}

void RecordingService::loadSettings()
{
    auto &s = AppSettings::instance();
    segmentMinutes_ = s.value(QString::fromLatin1(kSegmentMinutes), segmentMinutes_).toInt();
    cyclic_ = s.value(QString::fromLatin1(kCyclicEnabled), cyclic_).toBool();
    storagePath_ = s.value(QString::fromLatin1(kStoragePath), storagePath_).toString();
    if (storagePath_.isEmpty())
        storagePath_ = QStandardPaths::writableLocation(QStandardPaths::MoviesLocation);
}

bool RecordingService::updateRecordingOsdText()
{
    QString error;
    if (osdRenderer_.writeTextFile(osdTextFile_, currentOsdTelemetry(), &error))
        return true;

    emit errorOccurred(error);
    return false;
}

OsdTelemetry RecordingService::currentOsdTelemetry() const
{
    const auto telemetry = vehicle_.latest();
    return OsdTelemetry{
        telemetry.odometerM,
        telemetry.speedMps,
    };
}

QString RecordingService::buildOutputFilePath() const
{
    const QString name = QStringLiteral("PipeSight_%1.mp4")
                             .arg(QDateTime::currentDateTime().toString(QStringLiteral("yyyyMMdd_HHmmss")));
    return QDir(storagePath_).filePath(name);
}

QStringList RecordingService::buildFfmpegArguments(const QUrl &sourceUrl, const QString &outputFile) const
{
    const QString drawText = osdRenderer_.drawTextFilter(osdTextFile_);

    QStringList args;
    args << QStringLiteral("-hide_banner")
         << QStringLiteral("-y")
         << QStringLiteral("-rtsp_transport") << QStringLiteral("tcp")
         << QStringLiteral("-i") << sourceUrl.toString();
    if (!drawText.isEmpty())
        args << QStringLiteral("-vf") << drawText;
    args
         << QStringLiteral("-c:v") << QStringLiteral("libx264")
         << QStringLiteral("-preset") << QStringLiteral("veryfast")
         << QStringLiteral("-crf") << QStringLiteral("23")
         << QStringLiteral("-pix_fmt") << QStringLiteral("yuv420p")
         << QStringLiteral("-c:a") << QStringLiteral("aac")
         << QStringLiteral("-b:a") << QStringLiteral("128k")
         << QStringLiteral("-movflags") << QStringLiteral("+faststart")
         << outputFile;
    return args;
}

} // namespace pipesight::services
