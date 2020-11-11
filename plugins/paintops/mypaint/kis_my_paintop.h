#ifndef KIS_MY_PAINTOP_H_
#define KIS_MY_PAINTOP_H_

#include <kis_types.h>
#include <brushengine/kis_paintop.h>

#include <libmypaint/mypaint-brush.h>
#include <kis_airbrush_option_widget.h>

#include "kis_mypaint_brush.h"
#include "kis_mypaint_surface.h"

class KisPainter;


class KisMyPaintOp : public KisPaintOp
{

public:

    KisMyPaintOp(const KisPaintOpSettingsSP settings, KisPainter * painter, KisNodeSP node, KisImageSP image);
    ~KisMyPaintOp() override;

protected:

    KisSpacingInformation paintAt(const KisPaintInformation& info) override;

    KisSpacingInformation updateSpacingImpl(const KisPaintInformation &info) const override;

    KisTimingInformation updateTimingImpl(const KisPaintInformation &info) const override;

private:
    KisSpacingInformation computeSpacing(const KisPaintInformation &info, qreal lodScale) const;

private:
    QScopedPointer<KisMyPaintBrush> m_brush;
    QScopedPointer<KisMyPaintSurface> m_surface;
    KisPaintOpSettingsSP m_settings;
    KisAirbrushOptionProperties m_airBrushOption;
    KisImageWSP m_image;
    double m_dtime, m_radius, m_previousTime = 0;
    bool m_isStrokeStarted;
};

#endif // KIS_MY_PAINTOP_H_
