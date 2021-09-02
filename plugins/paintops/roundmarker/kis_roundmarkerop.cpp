/*
 *  SPDX-FileCopyrightText: 2016 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_roundmarkerop.h"

#include <cmath>
#include <memory>
#include <QRect>

#include <KoColorSpaceRegistry.h>
#include <KoColor.h>
#include <KoColorProfile.h>
#include <KoCompositeOpRegistry.h>

#include <kis_brush.h>
#include <kis_global.h>
#include <kis_paint_device.h>
#include <kis_painter.h>
#include <kis_image.h>
#include <kis_selection.h>
#include <kis_brush_based_paintop_settings.h>
#include <kis_cross_device_color_sampler.h>
#include <kis_fixed_paint_device.h>
#include <kis_lod_transform.h>
#include <kis_spacing_information.h>
#include "kis_marker_painter.h"
#include "kis_paintop_utils.h"



KisRoundMarkerOp::KisRoundMarkerOp(KisPaintOpSettingsSP settings, KisPainter* painter, KisNodeSP node, KisImageSP /*image*/)
    : KisPaintOp(painter)
    , m_firstRun(true)
    , m_lastRadius(1.0)
{
    Q_UNUSED(node);

    Q_ASSERT(settings);
    Q_ASSERT(painter);

    m_markerOption.readOptionSetting(*settings);
    m_sizeOption.readOptionSetting(settings);
    m_spacingOption.readOptionSetting(settings);

    m_sizeOption.resetAllSensors();
    m_spacingOption.resetAllSensors();
}

KisRoundMarkerOp::~KisRoundMarkerOp()
{
}

KisSpacingInformation KisRoundMarkerOp::paintAt(const KisPaintInformation& info)
{
    // Simple error catching
    if (!painter()->device()) {
        return KisSpacingInformation(1.0);
    }

    // get the scaling factor calculated by the size option
    const qreal lodScale = KisLodTransform::lodToScale(painter()->device());
    const qreal scale = m_sizeOption.apply(info) * lodScale;
    const qreal rotation = 0; // TODO

    const qreal diameter = m_markerOption.diameter * scale;
    qreal radius = 0.5 * diameter;

    if (KisPaintOpUtils::checkSizeTooSmall(scale, diameter, diameter)) return KisSpacingInformation();
    KisDabShape shape(scale, 1.0, rotation);

    // The position will need subtracting 0.5, but you cannot do this here
    // because then the mirroring tools get wrong position to mirror
    // and the mirroring doesn't work well.
    // Subtracting must happen just before the painting.
    QPointF pos = info.pos();

    KisMarkerPainter gc(painter()->device(), painter()->paintColor());

    if (m_firstRun) {
        const QVector<QPointF> points =
            painter()->calculateAllMirroredPoints(pos);

        Q_FOREACH(const QPointF &pt, points) {
            // Subtracting .5 from both dimensions, because the final dab tends to exaggerate towards the lower right.
            // This aligns it with the brush cursor.
            gc.fillFullCircle(pt - QPointF(0.5, 0.5), radius);
        }
    } else {
        const QVector<QPair<QPointF, QPointF>> pairs =
            painter()->calculateAllMirroredPoints(qMakePair(m_lastPaintPos, pos));

        Q_FOREACH(const auto &pair, pairs) {
            // Subtracting .5 from both dimensions, because the final dab tends to exaggerate towards the lower right.
            // This aligns it with the brush cursor.
            gc.fillCirclesDiff(pair.first - QPointF(0.5, 0.5), m_lastRadius,
                               pair.second - QPointF(0.5, 0.5), radius);
        }
    }

    m_firstRun = false;
    m_lastPaintPos = pos;
    m_lastRadius = radius;

    QRectF dirtyRect(pos.x() - radius, pos.y() - radius,
                     2 * radius, 2 * radius);
    dirtyRect = kisGrowRect(dirtyRect, 1);

    const QVector<QRect> allDirtyRects =
        painter()->calculateAllMirroredRects(dirtyRect.toAlignedRect());

    painter()->addDirtyRects(allDirtyRects);

    // QPointF scatteredPos =
    //     m_scatterOption.apply(info,
    //                           brush->maskWidth(shape, 0, 0, info),
    //                           brush->maskHeight(shape, 0, 0, info));



    //updateMask(info, scale, rotation, scatteredPos);

    //QPointF newCenterPos = QRectF(m_dstDabRect).center();
    /**
     * Save the center of the current dab to know where to read the
     * data during the next pass. We do not save scatteredPos here,
     * because it may differ slightly from the real center of the
     * brush (due to rounding effects), which will result in a
     * really weird quality.
     */
    //QRect srcDabRect = m_dstDabRect.translated((m_lastPaintPos - newCenterPos).toPoint());

    //m_lastPaintPos = newCenterPos;

    KisSpacingInformation spacingInfo = computeSpacing(info, diameter);

    if (m_firstRun) {
        m_firstRun = false;
        return spacingInfo;
    }


    return spacingInfo;
}

KisSpacingInformation KisRoundMarkerOp::updateSpacingImpl(const KisPaintInformation &info) const
{
    const qreal lodScale = KisLodTransform::lodToScale(painter()->device());
    const qreal diameter = m_markerOption.diameter * m_sizeOption.apply(info) * lodScale;

    return computeSpacing(info, diameter);
}

KisSpacingInformation KisRoundMarkerOp::computeSpacing(const KisPaintInformation &info,
                                                       qreal diameter) const
{
    const qreal rotation = 0; // TODO
    const bool axesFlipped = false; // TODO

    qreal extraSpacingScale = 1.0;
    if (m_spacingOption.isChecked()) {
        extraSpacingScale = m_spacingOption.apply(info);
    }

    return KisPaintOpUtils::effectiveSpacing(diameter, diameter,
                                             extraSpacingScale, true, true, rotation, axesFlipped,
                                             m_markerOption.spacing,
                                             m_markerOption.use_auto_spacing,
                                             m_markerOption.auto_spacing_coeff,
                                             KisLodTransform::lodToScale(painter()->device()));
}
