/*
 *  SPDX-FileCopyrightText: 2008-2011 Lukáš Tvrdý <lukast.dev@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KIS_CURVEPAINTOP_H_
#define KIS_CURVEPAINTOP_H_

#include <brushengine/kis_paintop.h>
#include <kis_types.h>
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
