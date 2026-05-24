#include "laser_service.h"
#include "comm/connection_manager.h"
#include "comm/tcp_client.h"

namespace pipesight::services {

LaserService::LaserService(QObject *parent)
    : QObject(parent)
    , client_(comm::ConnectionManager::instance().acquire(QStringLiteral("laser")))
{
    connect(client_, &comm::TcpClient::frameReceived,
            this, &LaserService::onFrameReceived);
}

LaserService::~LaserService() = default;

void LaserService::captureSnapshot()
{
    QMetaObject::invokeMethod(client_, "sendFrame", Qt::QueuedConnection,
                              Q_ARG(QByteArray, QByteArray()),
                              Q_ARG(quint8, 0x20));
}

void LaserService::addMarker(const QString &category, const QString &note)
{
    Marker m{ nextMarkerId_++, /*odometer*/ 0.0, category, note };
    // TODO: pull current odometer from VehicleService
    // TODO: persist to Database
    emit markerAdded(m);
}

void LaserService::removeMarker(int id)
{
    emit markerRemoved(id);
}

void LaserService::setPointCloudEnabled(bool enabled)
{
    QByteArray payload(1, enabled ? 1 : 0);
    QMetaObject::invokeMethod(client_, "sendFrame", Qt::QueuedConnection,
                              Q_ARG(QByteArray, payload),
                              Q_ARG(quint8, 0x21));
}

void LaserService::setStereoParam(const QString &key, const QVariant &value)
{
    Q_UNUSED(key); Q_UNUSED(value);
    // TODO: encode key/value pair, send TYPE=0x30
}

void LaserService::setRadarParam(const QString &key, const QVariant &value)
{
    Q_UNUSED(key); Q_UNUSED(value);
    // TODO: encode key/value pair, send TYPE=0x31
}

void LaserService::onFrameReceived(quint8 type, const QByteArray &payload)
{
    switch (type) {
    case 0x20: /* snapshot bytes → decode JPEG */ break;
    case 0x22: emit pointCloudFrame(payload); break;
    default: break;
    }
}

} // namespace pipesight::services
