#include "advanced_viewmodel.h"

namespace pipesight::viewmodels {

AdvancedViewModel::AdvancedViewModel(QObject *parent) : QObject(parent)
{
    connect(&service_, &services::ReportService::reportProgress,
            this, [this](int p) { reportProgress_ = p; emit reportProgressChanged(); });
    connect(&service_, &services::ReportService::firmwareProgress,
            this, [this](int p) { firmwareProgress_ = p; emit firmwareProgressChanged(); });
    connect(&service_, &services::ReportService::reportReady,
            this, &AdvancedViewModel::reportReady);
    connect(&service_, &services::ReportService::defectFound,
            this, &AdvancedViewModel::defectFound);
    connect(&service_, &services::ReportService::firmwareDone,
            this, &AdvancedViewModel::firmwareDone);
}

AdvancedViewModel::~AdvancedViewModel() = default;

} // namespace pipesight::viewmodels
