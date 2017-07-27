#include "kis_telemetry_abstract.h"

void KisTelemetryAbstract::notify(KisToolsActivate action, QString id)
{
    Q_UNUSED(action);
    id = getToolId(id, UseMode::Activate);
    putTimeTicket(id);
}



void KisTelemetryAbstract::notify(KisToolsDeactivate action, QString id)
{
    Q_UNUSED(action);
    id = getToolId(id, UseMode::Activate);
    getTimeTicket(id);
}

void KisTelemetryAbstract::notify(KisToolsStartUse action, QString id)
{
    Q_UNUSED(action);
    id = getToolId(id, UseMode::Use);
    putTimeTicket(id);
}

void KisTelemetryAbstract::notify(KisToolsStopUse action, QString id)
{
    Q_UNUSED(action);
    id = getToolId(id, UseMode::Use);
    getTimeTicket(id);
}

void KisTelemetryAbstract::notify(KisSaveImageProperties action, QString id)
{
    saveImageProperites(id, action.imageInfo());
}

void KisTelemetryAbstract::notify(KisSaveActionInfo action, QString id)
{
    saveActionInfo(id, action.actionInfo());
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
