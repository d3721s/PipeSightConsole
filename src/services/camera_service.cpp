#include "camera_service.h"
#include "comm/connection_manager.h"
#include "comm/tcp_client.h"
#include "data/app_settings.h"

#include <QRegularExpression>
#include <QUrlQuery>

namespace pipesight::services {

using comm::ConnectionManager;
using comm::TcpClient;
using pipesight::data::AppSettings;

namespace {

constexpr int kDefaultRtspPort = 554;

QString settingsStringOrDefault(AppSettings &settings, const QString &key, const QString &defaultValue)
{
    const QString value = settings.value(key, defaultValue).toString();
    return value.isEmpty() ? defaultValue : value;
}

QString normalizedResolution(QString value)
{
    value.replace(QRegularExpression(QStringLiteral("\\s*[xX]\\s*")), QStringLiteral(" x "));
    value.replace(QRegularExpression(QStringLiteral("\\s*\\*\\s*")), QStringLiteral(" x "));
    return value;
}

} // namespace

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
    if ((cfg.subtype == 1 && !cfg.sub1Enabled) ||
        (cfg.subtype == 2 && !cfg.sub2Enabled)) {
        return {};
    }

    QUrl url;
    url.setScheme(QStringLiteral("rtsp"));
    if (!cfg.username.isEmpty()) {
        url.setUserName(cfg.username);
        if (!cfg.password.isEmpty())
            url.setPassword(cfg.password);
    }
    url.setHost(cfg.ip);
    url.setPort(cfg.rtspPort > 0 ? cfg.rtspPort : kDefaultRtspPort);
    url.setPath(QStringLiteral("/cam/realmonitor"));

    QUrlQuery q;
    q.addQueryItem(QStringLiteral("channel"), QString::number(cfg.channel));
    q.addQueryItem(QStringLiteral("subtype"), QString::number(cfg.subtype));
    q.addQueryItem(QStringLiteral("unicast"), QStringLiteral("false"));
    q.addQueryItem(QStringLiteral("proto"), QStringLiteral("Onvif"));
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
    cfg.username       = settingsStringOrDefault(s, p + QStringLiteral("username"), QStringLiteral("admin"));
    cfg.password       = settingsStringOrDefault(s, p + QStringLiteral("password"), QStringLiteral("L26FDBCF"));
    cfg.ip             = settingsStringOrDefault(s, p + QStringLiteral("ip"), QStringLiteral("192.168.71.21"));
    cfg.rtspPort       = s.value(p + QStringLiteral("rtspPort"),
                                 s.value(p + QStringLiteral("port"), kDefaultRtspPort)).toInt();
    if (cfg.rtspPort <= 0) cfg.rtspPort = kDefaultRtspPort;
    cfg.channel        = s.value(p + QStringLiteral("channel"), 1).toInt();
    cfg.subtype        = s.value(p + QStringLiteral("subtype"), 0).toInt();
    if (cfg.subtype < 0 || cfg.subtype > 2) cfg.subtype = 0;
    cfg.mainResolution = normalizedResolution(s.value(p + QStringLiteral("mainResolution"), QStringLiteral("1920 x 1080")).toString());
    cfg.mainFps        = s.value(p + QStringLiteral("mainFps"), 25).toInt();
    cfg.sub1Enabled    = s.value(p + QStringLiteral("sub1Enabled"), true).toBool();
    cfg.sub1Resolution = normalizedResolution(s.value(p + QStringLiteral("sub1Resolution"),
                                                      s.value(p + QStringLiteral("subResolution"), QStringLiteral("704 x 576"))).toString());
    cfg.sub1Fps        = s.value(p + QStringLiteral("sub1Fps"),
                                 s.value(p + QStringLiteral("subFps"), 25)).toInt();
    cfg.sub2Enabled    = s.value(p + QStringLiteral("sub2Enabled"), false).toBool();
    cfg.sub2Resolution = normalizedResolution(s.value(p + QStringLiteral("sub2Resolution"), QStringLiteral("640 x 480")).toString());
    cfg.sub2Fps        = s.value(p + QStringLiteral("sub2Fps"), 25).toInt();
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
    s.setValue(p + QStringLiteral("port"),           cfg.rtspPort);
    s.setValue(p + QStringLiteral("channel"),        cfg.channel);
    s.setValue(p + QStringLiteral("subtype"),        cfg.subtype);
    s.setValue(p + QStringLiteral("mainResolution"), cfg.mainResolution);
    s.setValue(p + QStringLiteral("mainFps"),        cfg.mainFps);
    s.setValue(p + QStringLiteral("sub1Enabled"),    cfg.sub1Enabled);
    s.setValue(p + QStringLiteral("sub1Resolution"), cfg.sub1Resolution);
    s.setValue(p + QStringLiteral("sub1Fps"),        cfg.sub1Fps);
    s.setValue(p + QStringLiteral("sub2Enabled"),    cfg.sub2Enabled);
    s.setValue(p + QStringLiteral("sub2Resolution"), cfg.sub2Resolution);
    s.setValue(p + QStringLiteral("sub2Fps"),        cfg.sub2Fps);
    s.setValue(p + QStringLiteral("subResolution"),  cfg.sub1Resolution);
    s.setValue(p + QStringLiteral("subFps"),         cfg.sub1Fps);
}

void CameraService::onFrameReceived(quint8 type, const QByteArray &payload)
{
    Q_UNUSED(payload);
    if (type == 0x10 || type == 0x11)
        emit ptzAcknowledged();
}

} // namespace pipesight::services
