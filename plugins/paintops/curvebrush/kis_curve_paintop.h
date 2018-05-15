/*
 *  Copyright (c) 2008-2011 Lukáš Tvrdý <lukast.dev@gmail.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef KIS_CURVEPAINTOP_H_
#define KIS_CURVEPAINTOP_H_

#include <brushengine/kis_paintop.h>
#include <kis_types.h>
#include "curve_brush.h"
#include "kis_curve_line_option.h"
#include "kis_curve_paintop_settings.h"
#include <kis_pressure_opacity_option.h>
#include "kis_linewidth_option.h"
#include "kis_curves_opacity_option.h"

class KisPainter;

class KisCurvePaintOp : public KisPaintOp
{

public:
    KisCurvePaintOp(const KisPaintOpSettingsSP settings, KisPainter * painter, KisNodeSP node, KisImageSP image);
    ~KisCurvePaintOp() override;

    void paintLine(const KisPaintInformation &pi1, const KisPaintInformation &pi2, KisDistanceInformation *currentDistance) override;

protected:
    KisSpacingInformation paintAt(const KisPaintInformation& info) override;

    KisSpacingInformation updateSpacingImpl(const KisPaintInformation &info) const override;

private:
    void paintLine(KisPaintDeviceSP dab, const KisPaintInformation &pi1, const KisPaintInformation &pi2);

private:
    KisPaintDeviceSP m_dab;
    KisPaintDeviceSP m_dev;

    KisCurveOptionProperties m_curveProperties;
    KisPressureOpacityOption m_opacityOption;
    KisLineWidthOption m_lineWidthOption;
    KisCurvesOpacityOption m_curvesOpacityOption;

    QList<QPointF> m_points;
    KisPainter * m_painter;

};

#endif // KIS_CURVEPAINTOP_H_
