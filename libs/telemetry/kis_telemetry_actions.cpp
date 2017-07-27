#include "kis_telemetry_actions.h"
#include "kis_telemetry_abstract.h"

KisSaveImageProperties::KisSaveImageProperties(KisSaveImageProperties::ImageInfo imageInfo)
    : m_imageInfo(imageInfo)
{
}

KisSaveImageProperties::ImageInfo KisSaveImageProperties::imageInfo() const
{
    return m_imageInfo;
}

KisSaveActionInfo::KisSaveActionInfo(KisSaveActionInfo::ActionInfo actionInfo)
    : m_actionInfo(actionInfo)
{
}

KisSaveActionInfo::ActionInfo KisSaveActionInfo::actionInfo() const
{
    return m_actionInfo;
}
