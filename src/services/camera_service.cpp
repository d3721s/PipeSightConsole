#include "camera_service.h"
#include "comm/connection_manager.h"
#include "comm/tcp_client.h"
#include "data/app_settings.h"

#include <QUrlQuery>

namespace pipesight::services {

using comm::ConnectionManager;
using comm::TcpClient;
using pipesight::data::AppSettings;

CameraService::CameraService(QObject *parent)
    : QObject(parent)
    , ptzClient_(ConnectionManager::instance().acquire(QStringLiteral("camera-ptz")))
{
    connect(ptzClient_, &TcpClient::frameReceived,
            this, &CameraService::onFrameReceived);

    frontCfg_ = loadConfig(Channel::Front);
    rearCfg_  = loadConfig(Channel::Rear);
}

CameraService::~CameraService() = default;

QUrl CameraService::streamUrl(Channel ch) const
{
    return buildUrl(ch == Channel::Front ? frontCfg_ : rearCfg_);
}

CameraConfig CameraService::config(Channel ch) const
{
    return ch == Channel::Front ? frontCfg_ : rearCfg_;
}

void CameraService::pan(qreal dx, qreal dy)
{
    // TODO: pack dx/dy into vendor PTZ frame; placeholder = 4 bytes float pair
    QByteArray payload;
    payload.reserve(8);
    const float fx = static_cast<float>(dx), fy = static_cast<float>(dy);
    payload.append(reinterpret_cast<const char*>(&fx), sizeof(fx));
    payload.append(reinterpret_cast<const char*>(&fy), sizeof(fy));
    QMetaObject::invokeMethod(ptzClient_, "sendFrame", Qt::QueuedConnection,
                              Q_ARG(QByteArray, payload), Q_ARG(quint8, 0x10));
}

void CameraService::zoom(qreal delta)
{
    QByteArray payload(reinterpret_cast<const char*>(&delta), sizeof(qreal));
    QMetaObject::invokeMethod(ptzClient_, "sendFrame", Qt::QueuedConnection,
                              Q_ARG(QByteArray, payload), Q_ARG(quint8, 0x11));
}

void CameraService::switchChannel(Channel ch)
{
    if (active_ == ch) return;
    active_ = ch;
    emit activeChannelChanged(ch);
}

void CameraService::configureCamera(Channel ch, const CameraConfig &cfg)
{
    (ch == Channel::Front ? frontCfg_ : rearCfg_) = cfg;
    saveConfig(ch, cfg);
    emit configChanged(ch);
    emit streamUrlChanged(ch, buildUrl(cfg));
}

QUrl CameraService::buildUrl(const CameraConfig &cfg)
{
    if (!cfg.isComplete()) return {};

    QUrl url;
    url.setScheme(QStringLiteral("rtsp"));
    if (!cfg.username.isEmpty()) {
        url.setUserName(cfg.username);
        if (!cfg.password.isEmpty())
            url.setPassword(cfg.password);
    }
    url.setHost(cfg.ip);
    url.setPort(cfg.rtspPort);
    url.setPath(QStringLiteral("/cam/realmonitor"));

    QUrlQuery q;
    q.addQueryItem(QStringLiteral("channel"), QString::number(cfg.channel));
    q.addQueryItem(QStringLiteral("subtype"), QString::number(cfg.subtype));
    url.setQuery(q);
    return url;
}

QString CameraService::settingsPrefix(Channel ch)
{
    return ch == Channel::Front
        ? QStringLiteral("camera/front/")
        : QStringLiteral("camera/rear/");
}

CameraConfig CameraService::loadConfig(Channel ch) const
{
    auto &s = AppSettings::instance();
    const QString p = settingsPrefix(ch);
    CameraConfig cfg;
    cfg.username       = s.value(p + QStringLiteral("username")).toString();
    cfg.password       = s.value(p + QStringLiteral("password")).toString();
    cfg.ip             = s.value(p + QStringLiteral("ip")).toString();
    cfg.rtspPort       = static_cast<quint16>(s.value(p + QStringLiteral("rtspPort"), 554).toUInt());
    cfg.onvifPort      = static_cast<quint16>(s.value(p + QStringLiteral("onvifPort"), 80).toUInt());
    cfg.channel        = s.value(p + QStringLiteral("channel"), 1).toInt();
    cfg.subtype        = s.value(p + QStringLiteral("subtype"), 0).toInt();
    cfg.mainResolution = s.value(p + QStringLiteral("mainResolution"), QStringLiteral("1920x1080")).toString();
    cfg.mainFps        = s.value(p + QStringLiteral("mainFps"), 25).toInt();
    cfg.subResolution  = s.value(p + QStringLiteral("subResolution"), QStringLiteral("704x576")).toString();
    cfg.subFps         = s.value(p + QStringLiteral("subFps"), 25).toInt();
    return cfg;
}

void CameraService::saveConfig(Channel ch, const CameraConfig &cfg) const
{
    auto &s = AppSettings::instance();
    const QString p = settingsPrefix(ch);
    s.setValue(p + QStringLiteral("username"),       cfg.username);
    s.setValue(p + QStringLiteral("password"),       cfg.password);
    s.setValue(p + QStringLiteral("ip"),             cfg.ip);
    s.setValue(p + QStringLiteral("rtspPort"),       cfg.rtspPort);
    s.setValue(p + QStringLiteral("onvifPort"),      cfg.onvifPort);
    s.setValue(p + QStringLiteral("channel"),        cfg.channel);
    s.setValue(p + QStringLiteral("subtype"),        cfg.subtype);
    s.setValue(p + QStringLiteral("mainResolution"), cfg.mainResolution);
    s.setValue(p + QStringLiteral("mainFps"),        cfg.mainFps);
    s.setValue(p + QStringLiteral("subResolution"),  cfg.subResolution);
    s.setValue(p + QStringLiteral("subFps"),         cfg.subFps);
}

void CameraService::onFrameReceived(quint8 type, const QByteArray &payload)
{
    Q_UNUSED(payload);
    if (type == 0x10 || type == 0x11)
        emit ptzAcknowledged();
}

} // namespace pipesight::services
