#include "kis_telemetry_abstruct.h"

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
