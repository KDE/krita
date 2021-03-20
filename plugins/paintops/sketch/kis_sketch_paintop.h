/*
 *  SPDX-FileCopyrightText: 2010 Lukáš Tvrdý <lukast.dev@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
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

    static QList<KoResourceSP> prepareLinkedResources(const KisPaintOpSettingsSP settings, KisResourcesInterfaceSP resourcesInterface);

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
