#include "osd_service.h"

namespace pipesight::services {

OsdService::OsdService(QObject *parent) : QObject(parent) {}
OsdService::~OsdService() = default;

void OsdService::setShowProjectInfo(bool on) { if (showProject_   != on) { showProject_   = on; emit osdChanged(); } }
void OsdService::setShowTime(bool on)        { if (showTime_      != on) { showTime_      = on; emit osdChanged(); } }
void OsdService::setShowPosition(bool on)    { if (showPosition_  != on) { showPosition_  = on; emit osdChanged(); } }

void OsdService::setProjectName(const QString &name)
{
    if (projectName_ != name) { projectName_ = name; emit osdChanged(); }
}

void OsdService::setInspectionUnit(const QString &unit)
{
    if (inspectionUnit_ != unit) { inspectionUnit_ = unit; emit osdChanged(); }
}

} // namespace pipesight::services
