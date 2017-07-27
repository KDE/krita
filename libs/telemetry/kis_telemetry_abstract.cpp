#include "kis_telemetry_abstract.h"

void KisTelemetryAbstract::notifyToolAcion(KisTelemetryAbstract::Actions action, QString id)
{
    switch (action) {
    case ToolActivate: {
        id = getToolId(id, UseMode::Activate);
        putTimeTicket(id);
        break;
    }
    case ToolDeactivate: {
        id = getToolId(id, UseMode::Activate);
        getTimeTicket(id);
        break;
    }
    case ToolsStartUse: {
        id = getToolId(id, UseMode::Use);
        putTimeTicket(id);
        break;
    }
    case ToolsStopUse: {
        id = getToolId(id, UseMode::Use);
        getTimeTicket(id);
        break;
    }
    default:
        break;
    }
}

void KisTelemetryAbstract::notifySaveImageProperties(KisImagePropertiesTicket::ImageInfo imageInfo, QString id)
{
    saveImageProperites(id, imageInfo);
}

void KisTelemetryAbstract::notifySaveActionInfo(KisActionInfoTicket::ActionInfo actionInfo, QString id)
{
    saveActionInfo(id, actionInfo);
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
