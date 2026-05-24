#include "vehicle_service.h"
#include "comm/connection_manager.h"
#include "comm/tcp_client.h"

#include <QDataStream>

namespace pipesight::services {

VehicleService::VehicleService(QObject *parent)
    : QObject(parent)
    , client_(comm::ConnectionManager::instance().acquire(QStringLiteral("vehicle")))
{
    connect(client_, &comm::TcpClient::frameReceived,
            this, &VehicleService::onFrameReceived);
}

VehicleService::~VehicleService() = default;

void VehicleService::drive(qreal throttle, qreal steer)
{
    QByteArray payload;
    payload.reserve(8);
    const float ft = static_cast<float>(qBound(-1.0, throttle, 1.0));
    const float fs = static_cast<float>(qBound(-1.0, steer, 1.0));
    payload.append(reinterpret_cast<const char*>(&ft), sizeof(ft));
    payload.append(reinterpret_cast<const char*>(&fs), sizeof(fs));
    QMetaObject::invokeMethod(client_, "sendFrame", Qt::QueuedConnection,
                              Q_ARG(QByteArray, payload), Q_ARG(quint8, 0x40));
}

void VehicleService::setTargetSpeed(qreal mps)
{
    const float f = static_cast<float>(mps);
    QByteArray payload(reinterpret_cast<const char*>(&f), sizeof(f));
    QMetaObject::invokeMethod(client_, "sendFrame", Qt::QueuedConnection,
                              Q_ARG(QByteArray, payload), Q_ARG(quint8, 0x41));
}

void VehicleService::stop()
{
    drive(0.0, 0.0);
}

void VehicleService::setFrontLight(int brightness)
{
    QByteArray payload(1, static_cast<char>(qBound(0, brightness, 100)));
    QMetaObject::invokeMethod(client_, "sendFrame", Qt::QueuedConnection,
                              Q_ARG(QByteArray, payload), Q_ARG(quint8, 0x50));
}

void VehicleService::setRearLight(int brightness)
{
    QByteArray payload(1, static_cast<char>(qBound(0, brightness, 100)));
    QMetaObject::invokeMethod(client_, "sendFrame", Qt::QueuedConnection,
                              Q_ARG(QByteArray, payload), Q_ARG(quint8, 0x51));
}

void VehicleService::resetHome()
{
    QMetaObject::invokeMethod(client_, "sendFrame", Qt::QueuedConnection,
                              Q_ARG(QByteArray, QByteArray()), Q_ARG(quint8, 0x60));
}

void VehicleService::onFrameReceived(quint8 type, const QByteArray &payload)
{
    if (type == 0x80 && payload.size() >= 8 * 4 + 2) {
        // Telemetry frame layout:
        //   float pitch, roll, yaw, speed, odometer, batteryV;
        //   uint8 batteryPct, signalPct;
        QDataStream s(payload);
        s.setByteOrder(QDataStream::BigEndian);
        s.setFloatingPointPrecision(QDataStream::SinglePrecision);
        float pitch, roll, yaw, speed, odo, batV;
        quint8 batPct, sigPct;
        s >> pitch >> roll >> yaw >> speed >> odo >> batV >> batPct >> sigPct;
        latest_ = Telemetry{pitch, roll, yaw, speed, odo, batPct, batV, sigPct};
        emit telemetryUpdated(latest_);
    } else {
        emit commandAcknowledged(type);
    }
}

} // namespace pipesight::services
