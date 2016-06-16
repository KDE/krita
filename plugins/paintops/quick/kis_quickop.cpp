/*
 *  Copyright (c) 2015 Dmitry Kazakov <dimula73@gmail.com>
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

#include "kis_quickop.h"

#include <cmath>
#include <QRect>

#include <KoColorSpaceRegistry.h>
#include <KoColor.h>
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


KisQuickOp::KisQuickOp(const KisBrushBasedPaintOpSettings* settings, KisPainter* painter, KisNodeSP node, KisImageSP image)
    : KisBrushBasedPaintOp(settings, painter)
{
    Q_UNUSED(node);

    Q_ASSERT(settings);
    Q_ASSERT(painter);

    m_device = new KisPaintDevice(painter->device()->colorSpace());
    m_color = painter->paintColor();

    m_sizeOption.readOptionSetting(settings);
    m_opacityOption.readOptionSetting(settings);
    m_spacingOption.readOptionSetting(settings);
    m_scatterOption.readOptionSetting(settings);

    m_sizeOption.resetAllSensors();
    m_opacityOption.resetAllSensors();
    m_spacingOption.resetAllSensors();
    m_scatterOption.resetAllSensors();
}

KisQuickOp::~KisQuickOp()
{
}

KisSpacingInformation KisQuickOp::paintAt(const KisPaintInformation& info)
{
    KisBrushSP brush = m_brush;

    // Simple error catching
    if (!painter()->device() || !brush || !brush->canPaintFor(info)) {
        return KisSpacingInformation(1.0);
    }

    // get the scaling factor calculated by the size option
    qreal scale    = m_sizeOption.apply(info);

    if (checkSizeTooSmall(scale)) return KisSpacingInformation();

    const qreal rotation = 0;

    QPointF scatteredPos =
        m_scatterOption.apply(info,
                              brush->maskWidth(scale, rotation, 0, 0, info),
                              brush->maskHeight(scale, rotation, 0, 0, info));

    //qDebug() << ppVar(scatteredPos);

    QPointF hotSpot = brush->hotSpot(scale, scale, rotation, info);


    qreal rx = 0.5 * scale * brush->width();
    qreal ry = 0.5 * scale * brush->height();
    qreal opacity = m_opacityOption.getOpacityf(info);

    m_points << KisMultipointPainter::Point(scatteredPos, rx, ry, opacity);

    static int onePointTime = 1;

    const int expectedJobTime = onePointTime * m_points.size();

    if (m_lastPaintTime.isNull() ||
        m_lastPaintTime.elapsed() > 30 ||
        expectedJobTime > 60 ||
        m_points.size() > 15) {

    //if (m_points.size() > 5) {

        QTime initTime;
        initTime.start();
        m_multipointPainter.setPoints(m_points);
        m_multipointPainter.calcMergedAreas();
        qDebug() << ppVar(initTime.elapsed());

        qDebug() << ppVar(m_points.size());

        QTime pointsTime;
        pointsTime.start();

        m_multipointPainter.paintPoints3(painter()->device(), m_color);

        int elapsedCycleTime = pointsTime.elapsed();



        onePointTime = elapsedCycleTime / m_points.size();

        qDebug() << ppVar(elapsedCycleTime) << ppVar(onePointTime);

        m_lastPaintTime.restart();
        m_points.clear();


        //foreach (const QRect &rc, 

        const QRect &dirtyRect = m_multipointPainter.boundingRect();
        painter()->addDirtyRect(dirtyRect);

        //KIS_DUMP_DEVICE_2(m_device, QRect(0,0,1000,1000), "dev", "dd");

        //qDebug() << ppVar(painter()->compositeOp()->id());

        //painter()->setCompositeOp(COMPOSITE_OVER);
        //painter()->bitBlt(dirtyRect.topLeft(), m_device, dirtyRect);
        //m_device->clear();
    }


    KisSpacingInformation spacingInfo =
        effectiveSpacing(scale, rotation,
                         m_spacingOption, info);

    return spacingInfo;
}
