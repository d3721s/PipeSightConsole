#include "recording_service.h"
#include "data/app_settings.h"
#include "services/osd_service.h"

#include <QDateTime>
#include <QDir>
#include <QFileInfo>
#include <QMediaPlayer>
#include <QProcess>
#include <QSharedPointer>
#include <QStandardPaths>
#include <QStringList>
#include <QVideoFrame>
#include <QVideoSink>

namespace pipesight::services {

using pipesight::data::AppSettings;

namespace {

constexpr auto kSegmentMinutes = "recording/segmentMinutes";
constexpr auto kStoragePath = "recording/storagePath";
constexpr auto kCodec = "recording/codec";
constexpr auto kEncodingMode = "recording/encodingMode";

} // namespace

RecordingService::RecordingService(QObject *parent)
    : QObject(parent)
    , storagePath_(QStandardPaths::writableLocation(QStandardPaths::MoviesLocation))
    , vehicle_(VehicleService::instance())
{
    loadSettings();

    osdTimer_.setInterval(OsdService::instance().refreshMs());
    connect(&osdTimer_, &QTimer::timeout, this, [this]() {
        updateRecordingOsdText();
    });
    connect(&OsdService::instance(), &OsdService::osdChanged, this, [this]() {
        osdTimer_.setInterval(OsdService::instance().refreshMs());
        if (recording_)
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
    emit errorOccurred(QStringLiteral("未提供拍照视频源"));
}

void RecordingService::takeSnapshot(const QUrl &sourceUrl)
{
    if (!sourceUrl.isValid() || sourceUrl.isEmpty()) {
        emit errorOccurred(QStringLiteral("RTSP 视频源为空，无法拍照"));
        return;
    }

    loadSettings();

    auto *player = new QMediaPlayer(this);
    auto *sink = new QVideoSink(player);
    auto *timeout = new QTimer(player);
    timeout->setSingleShot(true);
    timeout->setInterval(4000);

    const auto finished = QSharedPointer<bool>::create(false);
    auto finish = [player, timeout, finished]() {
        if (*finished)
            return false;

        *finished = true;
        timeout->stop();
        player->stop();
        player->deleteLater();
        return true;
    };

    connect(timeout, &QTimer::timeout, this, [this, finish]() mutable {
        if (finish())
            emit errorOccurred(QStringLiteral("拍照超时，未获取到视频帧"));
    });

    connect(player, &QMediaPlayer::errorOccurred,
            this, [this, finish](QMediaPlayer::Error error, const QString &errorString) mutable {
                if (error == QMediaPlayer::NoError || !finish())
                    return;

                emit errorOccurred(errorString.isEmpty()
                                       ? QStringLiteral("拍照视频源打开失败")
                                       : QStringLiteral("拍照视频源打开失败：%1").arg(errorString));
            });

    connect(sink, &QVideoSink::videoFrameChanged,
            this, [this, finish](const QVideoFrame &frame) mutable {
                if (!frame.isValid())
                    return;
                if (!finish())
                    return;

                QImage image = frame.toImage();
                if (image.isNull()) {
                    emit errorOccurred(QStringLiteral("拍照失败：视频帧无法转换为图片"));
                    return;
                }

                image = osdRenderer_.renderImage(image, currentOsdTelemetry());

                const QString filePath = buildSnapshotFilePath();
                const QString dir = QFileInfo(filePath).absolutePath();
                if (!QDir().mkpath(dir)) {
                    emit errorOccurred(QStringLiteral("无法创建拍照存储目录：%1").arg(dir));
                    return;
                }

                if (!image.save(filePath, "PNG")) {
                    emit errorOccurred(QStringLiteral("拍照保存失败：%1").arg(filePath));
                    return;
                }

                emit snapshotSaved(filePath);
            });

    player->setVideoSink(sink);
    player->setSource(sourceUrl);
    timeout->start();
    player->play();
}

void RecordingService::setResolution(int w, int h)
{
    width_ = w;
    height_ = h;
}

void RecordingService::setCodec(Codec c)
{
    if (codec_ == c) return;
    codec_ = c;
    AppSettings::instance().setValue(QString::fromLatin1(kCodec), static_cast<int>(c));
}

void RecordingService::setEncodingMode(EncodingMode mode)
{
    if (encodingMode_ == mode) return;
    encodingMode_ = mode;
    AppSettings::instance().setValue(QString::fromLatin1(kEncodingMode), static_cast<int>(mode));
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

void RecordingService::loadSettings()
{
    auto &s = AppSettings::instance();
    segmentMinutes_ = s.value(QString::fromLatin1(kSegmentMinutes), segmentMinutes_).toInt();
    storagePath_ = s.value(QString::fromLatin1(kStoragePath), storagePath_).toString();
    codec_ = static_cast<Codec>(qBound(0, s.value(QString::fromLatin1(kCodec), static_cast<int>(codec_)).toInt(), 2));
    encodingMode_ = static_cast<EncodingMode>(qBound(0, s.value(QString::fromLatin1(kEncodingMode), static_cast<int>(encodingMode_)).toInt(), 1));
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
    const QString extension = codec_ == Codec::MJPEG ? QStringLiteral("avi") : QStringLiteral("mp4");
    const QString name = QStringLiteral("PipeSight_%1.%2")
                             .arg(QDateTime::currentDateTime().toString(QStringLiteral("yyyyMMdd_HHmmss")),
                                  extension);
    return QDir(storagePath_).filePath(name);
}

QString RecordingService::buildSnapshotFilePath() const
{
    const QString name = QStringLiteral("PipeSight_snapshot_%1.png")
                             .arg(QDateTime::currentDateTime().toString(
                                 QStringLiteral("yyyyMMdd_HHmmss_zzz")));
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

    const bool quality = encodingMode_ == EncodingMode::Quality;
    switch (codec_) {
    case Codec::H265:
        args << QStringLiteral("-c:v") << QStringLiteral("libx265")
             << QStringLiteral("-preset") << (quality ? QStringLiteral("slow") : QStringLiteral("veryfast"))
             << QStringLiteral("-crf") << (quality ? QStringLiteral("20") : QStringLiteral("28"))
             << QStringLiteral("-pix_fmt") << QStringLiteral("yuv420p")
             << QStringLiteral("-tag:v") << QStringLiteral("hvc1")
             << QStringLiteral("-c:a") << QStringLiteral("aac")
             << QStringLiteral("-b:a") << (quality ? QStringLiteral("320k") : QStringLiteral("128k"))
             << QStringLiteral("-movflags") << QStringLiteral("+faststart");
        break;
    case Codec::MJPEG:
        args << QStringLiteral("-c:v") << QStringLiteral("mjpeg")
             << QStringLiteral("-q:v") << (quality ? QStringLiteral("2") : QStringLiteral("5"))
             << QStringLiteral("-pix_fmt") << QStringLiteral("yuvj420p")
             << QStringLiteral("-c:a") << QStringLiteral("pcm_s16le");
        break;
    case Codec::H264:
        args << QStringLiteral("-c:v") << QStringLiteral("libx264")
             << QStringLiteral("-preset") << (quality ? QStringLiteral("slow") : QStringLiteral("veryfast"))
             << QStringLiteral("-crf") << (quality ? QStringLiteral("16") : QStringLiteral("23"))
             << QStringLiteral("-profile:v") << (quality ? QStringLiteral("high") : QStringLiteral("main"))
             << QStringLiteral("-pix_fmt") << QStringLiteral("yuv420p")
             << QStringLiteral("-c:a") << QStringLiteral("aac")
             << QStringLiteral("-b:a") << (quality ? QStringLiteral("320k") : QStringLiteral("128k"))
             << QStringLiteral("-movflags") << QStringLiteral("+faststart");
        break;
    }
    args << outputFile;
    return args;
}

} // namespace pipesight::services
