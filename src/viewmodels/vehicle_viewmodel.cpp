#include "vehicle_viewmodel.h"

namespace pipesight::viewmodels {

VehicleViewModel::VehicleViewModel(QObject *parent) : QObject(parent)
{
    connect(&service_, &services::VehicleService::telemetryUpdated,
            this, [this](const services::VehicleService::Telemetry &t) {
                latest_ = t;
                emit telemetryChanged();
            });
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

} // namespace pipesight::viewmodels
