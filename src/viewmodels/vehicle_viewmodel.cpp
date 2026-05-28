#include "vehicle_viewmodel.h"

namespace pipesight::viewmodels {

VehicleViewModel::VehicleViewModel(QObject *parent)
    : QObject(parent)
    , service_(services::VehicleService::instance())
{
    connect(&service_, &services::VehicleService::telemetryUpdated,
            this, [this](const services::VehicleService::Telemetry &t) {
                latest_ = t;
                emit telemetryChanged();
            });

    refreshTimer_.setInterval(qMax(250, service_.infoRefreshMs()));
    refreshTimer_.setSingleShot(false);
    connect(&refreshTimer_, &QTimer::timeout, this, [this]() {
        latest_ = service_.latest();
        emit telemetryChanged();
    });
    refreshTimer_.start();
}

VehicleViewModel::~VehicleViewModel() = default;

void VehicleViewModel::setFrontLight(int v)
{
    v = qBound(0, v, 100);
    if (frontLight_ == v) return;
    frontLight_ = v;
    service_.setFrontLight(v);
    emit frontLightChanged();
}

void VehicleViewModel::setRearLight(int v)
{
    v = qBound(0, v, 100);
    if (rearLight_ == v) return;
    rearLight_ = v;
    service_.setRearLight(v);
    emit rearLightChanged();
}

void VehicleViewModel::setRefreshMs(int ms)
{
    ms = qBound(250, ms, 10000);
    if (refreshTimer_.interval() == ms) return;
    refreshTimer_.setInterval(ms);
    latest_ = service_.latest();
    emit telemetryChanged();
    emit refreshMsChanged();
}

} // namespace pipesight::viewmodels
