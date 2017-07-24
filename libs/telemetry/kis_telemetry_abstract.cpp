#include "kis_telemetry_abstract.h"

void KisTelemetryAbstract::doTicket(KisToolsActivate &action, QString id)
{
    Q_UNUSED(action);
    id = getToolId(id, UseMode::Activate);
    putTimeTicket(id);
}

void KisTelemetryAbstract::doTicket(KisToolsDeactivate &action, QString id)
{
    Q_UNUSED(action);
    id = getToolId(id, UseMode::Activate);
    getTimeTicket(id);
}

void KisTelemetryAbstract::doTicket(KisToolsStartUse &action, QString id)
{
    Q_UNUSED(action);
    id = getToolId(id, UseMode::Use);
    putTimeTicket(id);
}

void KisTelemetryAbstract::doTicket(KisToolsStopUse &action, QString id)
{
    Q_UNUSED(action);
    id = getToolId(id, UseMode::Use);
    getTimeTicket(id);
}

void KisTelemetryAbstract::doTicket(KisSaveImageProperties &action, QString id)
{
    saveImageProperites(id, action.image());
}

QString KisTelemetryAbstract::getToolId(QString id, KisTelemetryAbstract::UseMode mode)
{
    QString toolId = "Tool" + getUseMode(mode);
    toolId += id;
    return toolId;
}

QString KisTelemetryAbstract::getUseMode(KisTelemetryAbstract::UseMode mode)
{
    switch (mode) {
    case Activate:
        return "/Activate/";
    case Use:
        return "/Use/";
    default:
        return "/Activate/";
    }
}
