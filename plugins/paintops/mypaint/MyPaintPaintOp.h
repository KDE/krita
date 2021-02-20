/*
 *  SPDX-FileCopyrightText: 2020 Ashwin Dhakaita <ashwingpdhakaita@gmail.com>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */

#ifndef KIS_MY_PAINTOP_H_
#define KIS_MY_PAINTOP_H_

#include <kis_types.h>
#include <brushengine/kis_paintop.h>

#include <libmypaint/mypaint-brush.h>
#include <kis_airbrush_option_widget.h>

#include "MyPaintPaintOpPreset.h"
#include "MyPaintSurface.h"

class KisPainter;


class KisMyPaintPaintOp : public KisPaintOp
{

public:

    KisMyPaintPaintOp(const KisPaintOpSettingsSP settings, KisPainter * painter, KisNodeSP node, KisImageSP image);
    ~KisMyPaintPaintOp() override;

protected:

    KisSpacingInformation paintAt(const KisPaintInformation& info) override;

    KisSpacingInformation updateSpacingImpl(const KisPaintInformation &info) const override;

    KisTimingInformation updateTimingImpl(const KisPaintInformation &info) const override;

private:
    KisSpacingInformation computeSpacing(const KisPaintInformation &info, qreal lodScale) const;

private:
    QScopedPointer<KisMyPaintPaintOpPreset> m_brush;
    QScopedPointer<KisMyPaintSurface> m_surface;
    KisPaintOpSettingsSP m_settings;
    KisAirbrushOptionProperties m_airBrushOption;
    KisImageWSP m_image;
    double m_dtime, m_radius, m_previousTime = 0;
    bool m_isStrokeStarted;
};

#endif // KIS_MY_PAINTOP_H_
