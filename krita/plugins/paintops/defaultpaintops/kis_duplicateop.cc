/*
 *  Copyright (c) 2002 Patrick Julien <freak@codepimps.org>
 *  Copyright (c) 2004 Boudewijn Rempt <boud@valdyas.org>
 *  Copyright (c) 2004 Clarence Dang <dang@kde.org>
 *  Copyright (c) 2004 Adrian Page <adrian@pagenet.plus.com>
 *  Copyright (c) 2004 Cyrille Berger <cberger@cberger.net>
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

#include <QRect>

#include <kdebug.h>

#include "kis_brush.h"
#include "kis_global.h"
#include "kis_paint_device.h"
#include "kis_layer.h"
#include "kis_painter.h"
#include "kis_types.h"
#include "kis_meta_registry.h"
#include "kis_colorspace_factory_registry.h"
#include "kis_paintop.h"
#include "kis_iterators_pixel.h"
#include "kis_selection.h"

#include "kis_duplicateop.h"

KisPaintOp * KisDuplicateOpFactory::createOp(const KisPaintOpSettings */*settings*/, KisPainter * painter)
{
    KisPaintOp * op = new KisDuplicateOp(painter);
    Q_CHECK_PTR(op);
    return op;
}


KisDuplicateOp::KisDuplicateOp(KisPainter * painter)
    : super(painter)
{
}

KisDuplicateOp::~KisDuplicateOp()
{
}

void KisDuplicateOp::paintAt(const KisPoint &pos, const KisPaintInformation& info)
{
    if (!m_painter) return;

    KisPaintDeviceSP device = m_painter->device();
    if (m_source) device = m_source;
    if (!device) return;

    KisBrush * brush = m_painter->brush();
    if (!brush) return;
    if (! brush->canPaintFor(info) )
        return;

    KisPoint hotSpot = brush->hotSpot(info);
    KisPoint pt = pos - hotSpot;

    // Split the coordinates into integer plus fractional parts. The integer
    // is where the dab will be positioned and the fractional part determines
    // the sub-pixel positioning.
    qint32 x;
    double xFraction;
    qint32 y;
    double yFraction;

    splitCoordinate(pt.x(), &x, &xFraction);
    splitCoordinate(pt.y(), &y, &yFraction);
    xFraction = yFraction = 0.0;

    KisPaintDeviceSP dab = KisPaintDeviceSP(0);

    if (brush->brushType() == IMAGE ||
        brush->brushType() == PIPE_IMAGE) {
        dab = brush->image(device->colorSpace(), info, xFraction, yFraction);
    }
    else {
        KisAlphaMaskSP mask = brush->mask(info, xFraction, yFraction);
        dab = computeDab(mask);
    }

    m_painter->setPressure(info.pressure);

    QPoint srcPoint = QPoint(x - static_cast<qint32>(m_painter->duplicateOffset().x()),
                             y - static_cast<qint32>(m_painter->duplicateOffset().y()));


    qint32 sw = dab->extent().width();
    qint32 sh = dab->extent().height();

    if (srcPoint.x() < 0 )
        srcPoint.setX(0);

    if( srcPoint.y() < 0)
        srcPoint.setY(0);

    KisPaintDeviceSP srcdev = KisPaintDeviceSP(new KisPaintDevice(dab->colorSpace(), "duplicate source dev"));
    Q_CHECK_PTR(srcdev);

    // First, copy the source data on the temporary device:
    KisPainter copyPainter(srcdev);
    copyPainter.bitBlt(0, 0, COMPOSITE_COPY, device, srcPoint.x(), srcPoint.y(), sw, sh);
    copyPainter.end();

    // Convert the dab to the colorspace of a selection
    dab->convertTo(KisMetaRegistry::instance()->csRegistry()->getAlpha8());

    // Add the dab as selection to the srcdev
    KisPainter copySelection(KisPaintDeviceSP(srcdev->selection().data()));
    copySelection.bitBlt(0, 0, COMPOSITE_OVER, dab, 0, 0, sw, sh);
    copySelection.end();

    // copy the srcdev onto a new device, after applying the dab selection
    KisPaintDeviceSP target = KisPaintDeviceSP(new KisPaintDevice(srcdev->colorSpace(), "duplicate target dev"));
    copyPainter.begin(target);

    copyPainter.bltSelection(0, 0, COMPOSITE_OVER, srcdev, srcdev->selection(),
                             OPACITY_OPAQUE, 0, 0, sw, sh);
    copyPainter.end();

    QRect dabRect = QRect(0, 0, brush->maskWidth(info), brush->maskHeight(info));
    QRect dstRect = QRect(x, y, dabRect.width(), dabRect.height());

    KisImage * image = device->image();

    if (image != 0) {
        dstRect &= image->bounds();
    }

    if (dstRect.isNull() || dstRect.isEmpty() || !dstRect.isValid()) return;

    qint32 sx = dstRect.x() - x;
    qint32 sy = dstRect.y() - y;
    sw = dstRect.width();
    sh = dstRect.height();
    
    if (m_source->hasSelection()) {
        m_painter->bltSelection(dstRect.x(), dstRect.y(), m_painter->compositeOp(), dab,
                                m_source->selection(), m_painter->opacity(), sx, sy, sw, sh);
    }
    else {
        m_painter->bitBlt(dstRect.x(), dstRect.y(), m_painter->compositeOp(), dab, m_painter->opacity(), sx, sy, sw, sh);
    }

    m_painter->addDirtyRect(dstRect);
}
