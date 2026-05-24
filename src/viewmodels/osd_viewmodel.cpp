#include "osd_viewmodel.h"

namespace pipesight::viewmodels {

OsdViewModel::OsdViewModel(QObject *parent) : QObject(parent)
{
    connect(&service_, &services::OsdService::osdChanged,
            this, &OsdViewModel::osdChanged);
}

OsdViewModel::~OsdViewModel() = default;

} // namespace pipesight::viewmodels
