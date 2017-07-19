#include "kis_telemetry_abstruct.h"

void KisTelemetryAbstruct::doTicket(KisToolsActivate &action, QString id)
{
    Q_UNUSED(action);
    id = getToolId(id, UseMode::Activate);
    putTimeTicket(id);
}

void KisTelemetryAbstruct::doTicket(KisToolsDeactivate &action, QString id)
{
    Q_UNUSED(action);
    id = getToolId(id, UseMode::Activate);
    getTimeTicket(id);
}

void KisTelemetryAbstruct::doTicket(KisToolsStartUse &action, QString id)
{
    Q_UNUSED(action);
    id = getToolId(id, UseMode::Use);
    putTimeTicket(id);
}

void KisTelemetryAbstruct::doTicket(KisToolsStopUse &action, QString id)
{
    Q_UNUSED(action);
    id = getToolId(id, UseMode::Use);
    getTimeTicket(id);
}

void KisTelemetryAbstruct::doTicket(KisSaveImageProperties &action, QString id)
{
    saveImageProperites(id, action.image());
}

QString KisTelemetryAbstruct::getToolId(QString id, KisTelemetryAbstruct::UseMode mode)
{
    QString toolId = "Tool" + getUseMode(mode);
    toolId += id;
    return toolId;
}

QString KisTelemetryAbstruct::getUseMode(KisTelemetryAbstruct::UseMode mode)
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
