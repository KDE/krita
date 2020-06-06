#include <cmath>

#include <kis_paint_action_type_option.h>
#include <kis_color_option.h>

#include "kis_my_paintop_settings.h"
///#include "kis_sprayop_option.h"

struct KisMyPaintOpSettings::Private
{
    QList<KisUniformPaintOpPropertyWSP> uniformProperties;
};


KisMyPaintOpSettings::KisMyPaintOpSettings()
    : KisOutlineGenerationPolicy<KisPaintOpSettings>(KisCurrentOutlineFetcher::SIZE_OPTION |
                                                     KisCurrentOutlineFetcher::ROTATION_OPTION),
    m_d(new Private)
{
}

KisMyPaintOpSettings::~KisMyPaintOpSettings()
{
}

void KisMyPaintOpSettings::setPaintOpSize(qreal value)
{
}

qreal KisMyPaintOpSettings::paintOpSize() const
{
}

bool KisMyPaintOpSettings::paintIncremental()
{
    return (enumPaintActionType)getInt("PaintOpAction", WASH) == BUILDUP;
}


QPainterPath KisMyPaintOpSettings::brushOutline(const KisPaintInformation &info, const OutlineMode &mode, qreal alignForZoom)
{
    QPainterPath path;
    return path;
}

#include <brushengine/kis_slider_based_paintop_property.h>
#include "kis_paintop_preset.h"
#include "kis_paintop_settings_update_proxy.h"
#include "kis_standard_uniform_properties_factory.h"


QList<KisUniformPaintOpPropertySP> KisMyPaintOpSettings::uniformProperties(KisPaintOpSettingsSP settings)
{
    QList<KisUniformPaintOpPropertySP> props =
        listWeakToStrong(m_d->uniformProperties);
    return props;
}
