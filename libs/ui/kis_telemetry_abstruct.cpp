#include "kis_telemetry_abstruct.h"

QString KisTelemetryAbstruct::getToolId(QString id)
{
    QString toolId = "Tool/";
    toolId += id;
    return toolId;
}
