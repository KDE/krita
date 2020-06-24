#include <cmath>

#include <kis_paint_action_type_option.h>
#include <kis_color_option.h>

#include "kis_my_paintop_settings.h"
#include "kis_my_paintop_option.h"

struct KisMyPaintOpSettings::Private
{
    //QList<KisUniformPaintOpPropertyWSP> uniformProperties;
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
    KisMyPaintOptionProperties op;
    op.readOptionSettingImpl(this);
    op.diameter = value;
    op.writeOptionSettingImpl(this);
}

qreal KisMyPaintOpSettings::paintOpSize() const
{
    KisMyPaintOptionProperties op;
    op.readOptionSettingImpl(this);
    return op.diameter;
}

bool KisMyPaintOpSettings::paintIncremental()
{
    return false;
}


QPainterPath KisMyPaintOpSettings::brushOutline(const KisPaintInformation &info, const OutlineMode &mode, qreal alignForZoom)
{    
    QPainterPath path;

    if (mode.isVisible) {
        qreal finalScale = 1.0;

        KisMyPaintOptionProperties op;
        op.readOptionSettingImpl(this);
        const qreal radius = 0.5 * op.diameter;

        QPainterPath realOutline;
        realOutline.addEllipse(QPointF(), radius, radius);

        path = outlineFetcher()->fetchOutline(info, this, realOutline, mode, alignForZoom, finalScale);

        if (mode.showTiltDecoration) {
            QPainterPath tiltLine = makeTiltIndicator(info,
                realOutline.boundingRect().center(),
                realOutline.boundingRect().width() * 0.5,
                3.0);
            path.addPath(outlineFetcher()->fetchOutline(info, this, tiltLine, mode, alignForZoom, finalScale, 0.0, true, realOutline.boundingRect().center().x(), realOutline.boundingRect().center().y()));
        }
    }

    return path;
}

#include <brushengine/kis_slider_based_paintop_property.h>
#include "kis_paintop_preset.h"
#include "kis_paintop_settings_update_proxy.h"
#include "kis_standard_uniform_properties_factory.h"


QList<KisUniformPaintOpPropertySP> KisMyPaintOpSettings::uniformProperties(KisPaintOpSettingsSP settings)
{
    QList<KisUniformPaintOpPropertySP> props;
        //listWeakToStrong(m_d->uniformProperties);
    return props;
}
