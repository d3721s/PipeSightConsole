#include "camera_viewmodel.h"

namespace pipesight::viewmodels {

using services::CameraService;

CameraViewModel::CameraViewModel(QObject *parent)
    : QObject(parent)
{
    connect(&service_, &CameraService::activeChannelChanged,
            this, &CameraViewModel::activeChannelChanged);
    connect(&service_, &CameraService::activeChannelChanged,
            this, &CameraViewModel::streamUrlChanged);
    connect(&service_, &CameraService::streamUrlChanged,
            this, [this](CameraService::Channel ch, const QUrl &) {
                if (ch == service_.activeChannel())
                    emit streamUrlChanged();
            });
    connect(&service_, &CameraService::configChanged,
            this, [this](CameraService::Channel ch) {
                emit cameraConfigChanged(static_cast<int>(ch));
            });
    connect(&recording_, &services::RecordingService::recordingStateChanged,
            this, [this](bool on) {
                emit recordingChanged();
                emit recordingNotification(on ? QStringLiteral("录像已开始")
                                              : QStringLiteral("录像已停止"),
                                           false);
            });
    connect(&recording_, &services::RecordingService::errorOccurred,
            this, [this](const QString &msg) {
                emit recordingNotification(msg, true);
            });
    connect(&recording_, &services::RecordingService::snapshotSaved,
            this, [this](const QString &filePath) {
                emit recordingNotification(QStringLiteral("拍照已保存：%1").arg(filePath), false);
            });
}

CameraViewModel::~CameraViewModel() = default;

QUrl CameraViewModel::streamUrl() const
{
    return service_.streamUrl(service_.activeChannel());
}

void CameraViewModel::startRecording()
{
    recording_.startRecording(streamUrl());
}

void CameraViewModel::stopRecording()
{
    recording_.stopRecording();
}

void CameraViewModel::snapshot()
{
    recording_.takeSnapshot(streamUrl());
}

bool CameraViewModel::configureCamera(int channel,
                                      const QString &username,
                                      const QString &password,
                                      const QString &ip,
                                      int rtspPort,
                                      int onvifPort,
                                      int rtspChannel,
                                      int subtype,
                                      const QString &mainResolution,
                                      int mainFps,
                                      bool sub1Enabled,
                                      const QString &sub1Resolution,
                                      int sub1Fps,
                                      bool sub2Enabled,
                                      const QString &sub2Resolution,
                                      int sub2Fps)
{
    if (channel < 0 || channel > 1)
        return false;

    services::CameraConfig cfg;
    cfg.username       = username;
    cfg.password       = password;
    cfg.ip             = ip;
    cfg.rtspPort       = static_cast<quint16>(rtspPort);
    cfg.onvifPort      = static_cast<quint16>(onvifPort);
    cfg.channel        = rtspChannel;
    cfg.subtype        = subtype;
    cfg.mainResolution = mainResolution;
    cfg.mainFps        = mainFps;
    cfg.sub1Enabled    = sub1Enabled;
    cfg.sub1Resolution = sub1Resolution;
    cfg.sub1Fps        = sub1Fps;
    cfg.sub2Enabled    = sub2Enabled;
    cfg.sub2Resolution = sub2Resolution;
    cfg.sub2Fps        = sub2Fps;
    service_.configureCamera(static_cast<CameraService::Channel>(channel), cfg);
    return true;
}

QVariantMap CameraViewModel::cameraConfig(int channel) const
{
    const auto cfg = service_.config(static_cast<CameraService::Channel>(channel));
    QVariantMap m;
    m.insert(QStringLiteral("username"),       cfg.username);
    m.insert(QStringLiteral("password"),       cfg.password);
    m.insert(QStringLiteral("ip"),             cfg.ip);
    m.insert(QStringLiteral("rtspPort"),       cfg.rtspPort);
    m.insert(QStringLiteral("onvifPort"),      cfg.onvifPort);
    m.insert(QStringLiteral("channel"),        cfg.channel);
    m.insert(QStringLiteral("subtype"),        cfg.subtype);
    m.insert(QStringLiteral("mainResolution"), cfg.mainResolution);
    m.insert(QStringLiteral("mainFps"),        cfg.mainFps);
    m.insert(QStringLiteral("sub1Enabled"),    cfg.sub1Enabled);
    m.insert(QStringLiteral("sub1Resolution"), cfg.sub1Resolution);
    m.insert(QStringLiteral("sub1Fps"),        cfg.sub1Fps);
    m.insert(QStringLiteral("sub2Enabled"),    cfg.sub2Enabled);
    m.insert(QStringLiteral("sub2Resolution"), cfg.sub2Resolution);
    m.insert(QStringLiteral("sub2Fps"),        cfg.sub2Fps);
    return m;
}

QString CameraViewModel::cameraStreamUrl(int channel) const
{
    return service_.streamUrl(static_cast<CameraService::Channel>(channel)).toString();
}

} // namespace pipesight::viewmodels
