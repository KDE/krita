#include "kis_my_paintop.h"

#include "kis_paintop.h"
#include "kis_spacing_information.h"
#include "kis_my_paintop_settings.h"
#include <kis_paint_information.h>
#include <kis_brush_based_paintop_settings.h>

#include <libmypaint/mypaint-brush.h>


KisMyPaintOp::KisMyPaintOp(const KisPaintOpSettingsSP settings, KisPainter * painter, KisNodeSP node, KisImageSP image)
    : KisPaintOp (painter) {

    MyPaintBrush *brush = mypaint_brush_new();
}

KisMyPaintOp::~KisMyPaintOp() {

}

KisSpacingInformation KisMyPaintOp::paintAt(const KisPaintInformation& info) {

    KisSpacingInformation spacingInfo;
    return spacingInfo;
}

KisSpacingInformation KisMyPaintOp::updateSpacingImpl(const KisPaintInformation &info) const
{
    KisSpacingInformation spacingInfo;
    return spacingInfo;
}
