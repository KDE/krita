/*
 *  SPDX-FileCopyrightText: 2008-2011 Lukáš Tvrdý <lukast.dev@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KIS_CURVEPAINTOP_H_
#define KIS_CURVEPAINTOP_H_

#include <brushengine/kis_paintop.h>
#include <kis_types.h>
#include <KisCurveStandardOptions.h>
#include <KisCurveOpOptionData.h>
#include <KisOpacityOption.h>

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

    KisCurveOpOptionData m_curveOpOption;
    KisOpacityOption m_opacityOption;
    KisLineWidthOption m_lineWidthOption;
    KisCurvesOpacityOption m_curvesOpacityOption;

    QList<QPointF> m_points;
    KisPainter * m_painter;

};

#endif // KIS_CURVEPAINTOP_H_
