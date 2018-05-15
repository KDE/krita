/*
 *  Copyright (c) 2010 Lukáš Tvrdý <lukast.dev@gmail.com>
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

#ifndef KIS_SKETCH_PAINTOP_H_
#define KIS_SKETCH_PAINTOP_H_

#include <brushengine/kis_paintop.h>
#include <kis_types.h>

#include "kis_density_option.h"
#include "kis_sketchop_option.h"
#include "kis_sketch_paintop_settings.h"

#include "kis_painter.h"
#include <kis_airbrush_option_widget.h>
#include <kis_pressure_size_option.h>
#include <kis_brush_option.h>
#include <kis_pressure_rotation_option.h>
#include <kis_pressure_rate_option.h>
#include "kis_linewidth_option.h"
#include "kis_offset_scale_option.h"

class KisDabCache;


class KisSketchPaintOp : public KisPaintOp
{

public:

    KisSketchPaintOp(const KisPaintOpSettingsSP settings, KisPainter *painter, KisNodeSP node, KisImageSP image);
    ~KisSketchPaintOp() override;

    void paintLine(const KisPaintInformation &pi1, const KisPaintInformation &pi2, KisDistanceInformation *currentDistance) override;

protected:
    KisSpacingInformation paintAt(const KisPaintInformation& info) override;

    KisSpacingInformation updateSpacingImpl(const KisPaintInformation &info) const override;

    KisTimingInformation updateTimingImpl(const KisPaintInformation &info) const override;

private:
    // pixel buffer
    KisPaintDeviceSP m_dab;

    // mask detection area
    KisFixedPaintDeviceSP m_maskDab;
    QRectF m_brushBoundingBox;
    QPointF m_hotSpot;

    // simple mode
    qreal m_radius;

    KisPressureOpacityOption m_opacityOption;
    KisPressureSizeOption m_sizeOption;
    KisPressureRotationOption m_rotationOption;
    KisPressureRateOption m_rateOption;
    KisDensityOption m_densityOption;
    KisLineWidthOption m_lineWidthOption;
    KisOffsetScaleOption m_offsetScaleOption;
    KisAirbrushOptionProperties m_airbrushOption;

    KisBrushOptionProperties m_brushOption;
    SketchProperties m_sketchProperties;

    QVector<QPointF> m_points;
    int m_count;
    KisPainter * m_painter;
    KisBrushSP m_brush;
    KisDabCache *m_dabCache;

private:
    void drawConnection(const QPointF &start, const QPointF &end, double lineWidth);
    void updateBrushMask(const KisPaintInformation& info, qreal scale, qreal rotation);
    void doPaintLine(const KisPaintInformation &pi1, const KisPaintInformation &pi2);
};

#endif // KIS_SKETCH_PAINTOP_H_
