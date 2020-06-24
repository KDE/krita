#include "kis_my_paintop.h"

#include "kis_paintop.h"
#include "kis_spacing_information.h"
#include "kis_my_paintop_settings.h"
#include <kis_paint_information.h>
#include <kis_brush_based_paintop_settings.h>
#include <kis_painter.h>
#include <KoColorConversions.h>
#include <kis_paintop_plugin_utils.h>

#include <libmypaint/mypaint-brush.h>
#include <QDebug>
#include <QtMath>


KisMyPaintOp::KisMyPaintOp(const KisPaintOpSettingsSP settings, KisPainter * painter, KisNodeSP node, KisImageSP image)
    : KisPaintOp (painter) {

    m_node = node;
    m_brush = new KisMyPaintBrush();
    m_surface = new KisMyPaintSurface(this->painter(), m_node);

    m_brush->apply(settings);

    if(qRound(mypaint_brush_get_base_value(m_brush->brush(), MYPAINT_BRUSH_SETTING_ERASER))) {

        m_brush->setColor(this->painter()->backgroundColor());
        mypaint_brush_set_base_value(m_brush->brush(), MYPAINT_BRUSH_SETTING_ERASER, false);
    }
    else {
        m_brush->setColor(this->painter()->paintColor());
    }

    m_settings = settings;

    dtime = -1;
    m_radius = exp(3.3);
}

KisMyPaintOp::~KisMyPaintOp() {

    delete m_brush;
    delete m_surface;
}

KisSpacingInformation KisMyPaintOp::paintAt(const KisPaintInformation& info) {

    if(!painter()) {
        return KisSpacingInformation(1.0);
    }

    if(dtime < 0) {
        mypaint_brush_stroke_to(m_brush->brush(), m_surface->surface(), info.pos().x(), info.pos().y(), info.pressure(),
                               info.xTilt(), info.yTilt(), 1.0f);

        dtime = 0.015;
    }
    else {
        dtime = (info.currentTime() - dtime)*0.001;
    }

    mypaint_brush_stroke_to(m_brush->brush(), m_surface->surface(), info.pos().x(), info.pos().y(), info.pressure(),
                           info.xTilt(), info.yTilt(), dtime);

    dtime = info.currentTime();

    const qreal lodScale = KisLodTransform::lodToScale(painter()->device());
    m_radius = exp(3.3)*lodScale;
    return computeSpacing(info, lodScale);
}

KisSpacingInformation KisMyPaintOp::updateSpacingImpl(const KisPaintInformation &info) const
{
    KisSpacingInformation spacingInfo = computeSpacing(info, m_radius*KisLodTransform::lodToScale(painter()->device()));
    return spacingInfo;
}

KisSpacingInformation KisMyPaintOp::computeSpacing(const KisPaintInformation &info, qreal lodScale) const {

    return KisPaintOpPluginUtils::effectiveSpacing(m_radius*2, m_radius*2,
                                                   false, 0.0, false, m_radius*2,
                                                   true, 1, lodScale, nullptr, nullptr, info);
}
