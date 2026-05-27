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
}

CameraViewModel::~CameraViewModel() = default;

QUrl CameraViewModel::streamUrl() const
{
    return service_.streamUrl(service_.activeChannel());
}

void CameraViewModel::startRecording()
{
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

void CameraViewModel::configureCamera(int channel,
                                      const QString &username,
                                      const QString &password,
                                      const QString &ip,
                                      int rtspPort,
                                      int onvifPort,
                                      int rtspChannel,
                                      int subtype,
                                      const QString &mainResolution,
                                      int mainFps,
                                      const QString &subResolution,
                                      int subFps)
{
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
    cfg.subResolution  = subResolution;
    cfg.subFps         = subFps;
    service_.configureCamera(static_cast<CameraService::Channel>(channel), cfg);
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
    m.insert(QStringLiteral("subResolution"),  cfg.subResolution);
    m.insert(QStringLiteral("subFps"),         cfg.subFps);
    return m;
}

QString CameraViewModel::cameraStreamUrl(int channel) const
{
    return service_.streamUrl(static_cast<CameraService::Channel>(channel)).toString();
}

} // namespace pipesight::viewmodels
