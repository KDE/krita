#include "kis_telemetry_actions.h"
#include "kis_telemetry_abstract.h"

void KisToolsActivate::doAction(KisTelemetryAbstract* provider, QString id)
{
    provider->doTicket(*this, id);
}

void KisToolsStartUse::doAction(KisTelemetryAbstract* provider, QString id)
{
    provider->doTicket(*this, id);
}

void KisToolsStopUse::doAction(KisTelemetryAbstract* provider, QString id)
{
    provider->doTicket(*this, id);
}

void KisToolsDeactivate::doAction(KisTelemetryAbstract* provider, QString id)
{
    provider->doTicket(*this, id);
}

KisSaveImageProperties::KisSaveImageProperties(KisSaveImageProperties::ImageInfo imageInfo)
    : m_imageInfo(imageInfo)
{
}

void KisSaveImageProperties::doAction(KisTelemetryAbstract* provider, QString id)
{
    provider->doTicket(*this, id);
}

KisSaveImageProperties::ImageInfo KisSaveImageProperties::imageInfo() const
{
    return m_imageInfo;
}

KisSaveActionInfo::KisSaveActionInfo(KisSaveActionInfo::ActionInfo actionInfo)
    : m_actionInfo(actionInfo)
{
}

void KisSaveActionInfo::doAction(KisTelemetryAbstract* provider, QString id)
{
    provider->doTicket(*this, id);
}

KisSaveActionInfo::ActionInfo KisSaveActionInfo::actionInfo() const
{
    return m_actionInfo;
}
