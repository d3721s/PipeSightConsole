#include "osd_viewmodel.h"

namespace pipesight::viewmodels {

OsdViewModel::OsdViewModel(QObject *parent)
    : QObject(parent)
    , service_(services::OsdService::instance())
{
    connect(&service_, &services::OsdService::osdChanged,
            this, &OsdViewModel::osdChanged);
}

OsdViewModel::~OsdViewModel() = default;

} // namespace pipesight::viewmodels
