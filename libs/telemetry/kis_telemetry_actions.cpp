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

KisSaveImageProperties::KisSaveImageProperties(KisImageSP &image): m_image(image)
{

}

void KisSaveImageProperties::doAction(KisTelemetryAbstract* provider, QString id)
{
    provider->doTicket(*this, id);
}

QString KisSaveImageProperties::fileName() const
{
    return m_fileName;
}

KisImageSP& KisSaveImageProperties::image()
{
    return m_image;
}

