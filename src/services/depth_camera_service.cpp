#include "depth_camera_service.h"
#include "comm/connection_manager.h"
#include "comm/tcp_client.h"

namespace pipesight::services {

DepthCameraService::DepthCameraService(QObject *parent)
    : QObject(parent)
    , client_(comm::ConnectionManager::instance().acquire(QStringLiteral("depth-camera")))
{
    connect(client_, &comm::TcpClient::frameReceived,
            this, &DepthCameraService::onFrameReceived);
}

DepthCameraService::~DepthCameraService() = default;

void DepthCameraService::captureDepthSnapshot()
{
    QMetaObject::invokeMethod(client_, "sendFrame", Qt::QueuedConnection,
                              Q_ARG(QByteArray, QByteArray()),
                              Q_ARG(quint8, 0x40));
}

void DepthCameraService::addDepthMarker(const QString &category, const QString &note)
{
    DepthMarker m{ nextMarkerId_++, 0.0, category, note };
    emit depthMarkerAdded(m);
}

void DepthCameraService::removeDepthMarker(int id)
{
    emit depthMarkerRemoved(id);
}

void DepthCameraService::setDepthCloudEnabled(bool enabled)
{
    QByteArray payload(1, enabled ? 1 : 0);
    QMetaObject::invokeMethod(client_, "sendFrame", Qt::QueuedConnection,
                              Q_ARG(QByteArray, payload),
                              Q_ARG(quint8, 0x41));
}

void DepthCameraService::setStereoParam(const QString &key, const QVariant &value)
{
    Q_UNUSED(key); Q_UNUSED(value);
}

void DepthCameraService::onFrameReceived(quint8 type, const QByteArray &payload)
{
    switch (type) {
    case 0x40: break;
    case 0x42: emit depthCloudFrame(payload); break;
    default: break;
    }
}

} // namespace pipesight::services
