/*
 *  Copyright (c) 2016 Dmitry Kazakov <dimula73@gmail.com>
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
#include <kis_cross_device_color_picker.h>
#include <kis_fixed_paint_device.h>
#include <kis_lod_transform.h>
#include <kis_spacing_information.h>
#include "kis_marker_painter.h"
#include "kis_paintop_utils.h"



KisRoundMarkerOp::KisRoundMarkerOp(KisPaintOpSettingsSP settings, KisPainter* painter, KisNodeSP node, KisImageSP image)
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


    QPointF pos = info.pos();

    KisMarkerPainter gc(painter()->device(), painter()->paintColor());

    if (m_firstRun) {
        const QVector<QPointF> points =
            painter()->calculateAllMirroredPoints(pos);

        Q_FOREACH(const QPointF &pt, points) {
            gc.fillFullCircle(pt, radius);
        }
    } else {
        const QVector<QPair<QPointF, QPointF>> pairs =
            painter()->calculateAllMirroredPoints(qMakePair(m_lastPaintPos, pos));

        Q_FOREACH(const auto &pair, pairs) {
            gc.fillCirclesDiff(pair.first, m_lastRadius,
                               pair.second, radius);
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
