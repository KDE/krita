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
#include "kis_marker_painter.h"


KisRoundMarkerOp::KisRoundMarkerOp(const KisBrushBasedPaintOpSettings* settings, KisPainter* painter, KisNodeSP node, KisImageSP image)
    : KisBrushBasedPaintOp(settings, painter)
    , m_firstRun(true)
    , m_image(image)
    , m_lastRadius(1.0)
{
    Q_UNUSED(node);

    Q_ASSERT(settings);
    Q_ASSERT(painter);

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
    KisBrushSP brush = m_brush;

    // Simple error catching
    if (!painter()->device() || !brush || !brush->canPaintFor(info)) {
        return KisSpacingInformation(1.0);
    }

    // get the scaling factor calculated by the size option
    qreal scale    = m_sizeOption.apply(info);
    scale *= KisLodTransform::lodToScale(painter()->device());
    const qreal rotation = 0;
    if (checkSizeTooSmall(scale)) return KisSpacingInformation();
    KisDabShape shape(scale, 1.0, rotation);

    qreal radius = 0.5 * brush->maskWidth(shape, 0, 0, info);
    QPointF pos = info.pos();

    KisMarkerPainter gc(painter()->device(), painter()->paintColor());

    if (m_firstRun) {
        gc.fillFullCircle(pos, radius);
    } else {
        gc.fillCirclesDiff(m_lastPaintPos, m_lastRadius,
                           pos, radius);
    }

    m_firstRun = false;
    m_lastPaintPos = pos;
    m_lastRadius = radius;

    QRectF dirtyRect(pos.x() - radius, pos.y() - radius,
                     2 * radius, 2 * radius);
    painter()->addDirtyRect(dirtyRect.toAlignedRect());

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

    KisSpacingInformation spacingInfo =
        effectiveSpacing(scale, rotation,
                         m_spacingOption, info);

    if (m_firstRun) {
        m_firstRun = false;
        return spacingInfo;
    }


    return spacingInfo;
}
