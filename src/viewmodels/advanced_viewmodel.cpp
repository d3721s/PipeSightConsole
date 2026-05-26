#include "advanced_viewmodel.h"

namespace pipesight::viewmodels {

AdvancedViewModel::AdvancedViewModel(QObject *parent) : QObject(parent)
{
    connect(&service_, &services::ReportService::reportReady,
            this, &AdvancedViewModel::reportReady);
    connect(&service_, &services::ReportService::calibrationDone,
            this, &AdvancedViewModel::calibrationDone);
}

AdvancedViewModel::~AdvancedViewModel() = default;

} // namespace pipesight::viewmodels
