#ifndef KIS_MY_PAINTOP_H_
#define KIS_MY_PAINTOP_H_

#include <brushengine/kis_paintop.h>
#include <kis_types.h>
#include "kis_mypaint_surface.h"
#include "kis_mypaint_brush.h"

#include <libmypaint/mypaint-brush.h>

class KisPainter;


class KisMyPaintOp : public KisPaintOp
{

public:

    KisMyPaintOp(const KisPaintOpSettingsSP settings, KisPainter * painter, KisNodeSP node, KisImageSP image);
    ~KisMyPaintOp() override;

protected:

    KisSpacingInformation paintAt(const KisPaintInformation& info) override;

    KisSpacingInformation updateSpacingImpl(const KisPaintInformation &info) const override;

    //KisTimingInformation updateTimingImpl(const KisPaintInformation &info) const override;

private:
    KisSpacingInformation computeSpacing(const KisPaintInformation &info, qreal lodScale) const;

private:
    KisMyPaintBrush *m_brush;
    KisMyPaintSurface *m_surface;
    KisPaintOpSettingsSP m_settings;
    KisNodeSP m_node;
    double dtime, m_radius;
};

#endif // KIS_MY_PAINTOP_H_
