#include "camera_service.h"
#include "comm/connection_manager.h"
#include "comm/tcp_client.h"

namespace pipesight::services {

using comm::ConnectionManager;
using comm::TcpClient;

CameraService::CameraService(QObject *parent)
    : QObject(parent)
    , ptzClient_(ConnectionManager::instance().acquire(QStringLiteral("camera-ptz")))
{
    connect(ptzClient_, &TcpClient::frameReceived,
            this, &CameraService::onFrameReceived);
}

CameraService::~CameraService() = default;

QUrl CameraService::streamUrl(Channel ch) const
{
    return ch == Channel::Front ? frontUrl_ : rearUrl_;
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

void CameraService::setStreamUrl(Channel ch, const QUrl &rtsp)
{
    (ch == Channel::Front ? frontUrl_ : rearUrl_) = rtsp;
    emit streamUrlChanged(ch, rtsp);
}

void CameraService::configureCamera(Channel ch, const QString &ip,
                                    const QString &mainStream,
                                    const QString &subStream,
                                    const QString &thirdStream)
{
    // TODO: persist via AppSettings; here we just compose the main RTSP URL
    Q_UNUSED(subStream); Q_UNUSED(thirdStream);
    const QString url = QStringLiteral("rtsp://%1/%2").arg(ip, mainStream);
    setStreamUrl(ch, QUrl(url));
}

void CameraService::onFrameReceived(quint8 type, const QByteArray &payload)
{
    Q_UNUSED(payload);
    if (type == 0x10 || type == 0x11)
        emit ptzAcknowledged();
}

} // namespace pipesight::services
