#ifndef KIS_MY_PAINTOP_H_
#define KIS_MY_PAINTOP_H_

#include <brushengine/kis_paintop.h>
#include <kis_types.h>

//#include "kis_spray_paintop_settings.h"
//#include "kis_brush_option.h"
// #include <kis_airbrush_option_widget.h>
// #include <kis_pressure_rotation_option.h>
// #include <kis_pressure_opacity_option.h>
// #include <kis_pressure_size_option.h>
// #include <kis_pressure_rate_option.h>

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
    //KisSpacingInformation computeSpacing(const KisPaintInformation &info, qreal lodScale) const;

private:
};

#endif // KIS_MY_PAINTOP_H_
