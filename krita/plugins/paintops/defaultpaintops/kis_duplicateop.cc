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

#include <qrect.h>

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

    bool heal = m_painter->duplicateHealing();
    int healradius = m_painter->duplicateHealingRadius();

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
    Q_INT32 x;
    double xFraction;
    Q_INT32 y;
    double yFraction;

    splitCoordinate(pt.x(), &x, &xFraction);
    splitCoordinate(pt.y(), &y, &yFraction);
    xFraction = yFraction = 0.0;

    KisPaintDeviceSP dab = 0;

    if (brush->brushType() == IMAGE ||
        brush->brushType() == PIPE_IMAGE) {
        dab = brush->image(device->colorSpace(), info, xFraction, yFraction);
        dab->convertTo(KisMetaRegistry::instance()->csRegistry()->getAlpha8());
    }
    else {
        KisAlphaMaskSP mask = brush->mask(info, xFraction, yFraction);
        dab = computeDab(mask, KisMetaRegistry::instance()->csRegistry()->getAlpha8());
    }

    m_painter->setPressure(info.pressure);

    QPoint srcPoint = QPoint(x - static_cast<Q_INT32>(m_painter->duplicateOffset().x()),
                             y - static_cast<Q_INT32>(m_painter->duplicateOffset().y()));


    Q_INT32 sw = dab->extent().width();
    Q_INT32 sh = dab->extent().height();

    if (srcPoint.x() < 0 )
        srcPoint.setX(0);

    if( srcPoint.y() < 0)
        srcPoint.setY(0);

    KisPaintDeviceSP srcdev = new KisPaintDevice(device->colorSpace(), "duplicate source dev");
    Q_CHECK_PTR(srcdev);

    // First, copy the source data on the temporary device:
    KisPainter copyPainter(srcdev);
    copyPainter.bitBlt(0, 0, COMPOSITE_COPY, device, srcPoint.x(), srcPoint.y(), sw, sh);
    copyPainter.end();

    // heal ?
    
    if(heal)
    {
        Q_UINT16 data[3];
        // Compute color info from around source
        double meanL_src = 0., meanA_src = 0., meanB_src = 0.;
        double sigmaL_src = 0., sigmaA_src = 0., sigmaB_src = 0.;
        KisRectIteratorPixel deviceIt = device->createRectIterator(srcPoint.x() - healradius / 2 +hotSpot.x(), srcPoint.y() - healradius / 2+hotSpot.y(), healradius, healradius, false );
        KisColorSpace* deviceCs = device->colorSpace();
        while(!deviceIt.isDone())
        {
            deviceCs->toLabA16(deviceIt.rawData(), (Q_UINT8*)data, 1);
            Q_UINT32 L = data[0];
            Q_UINT32 A = data[1];
            Q_UINT32 B = data[2];
            meanL_src += L;
            meanA_src += A;
            meanB_src += B;
            sigmaL_src += L*L;
            sigmaA_src += A*A;
            sigmaB_src += B*B;
            ++deviceIt;
        }
        double size = 1. / ( healradius * healradius );
        meanL_src *= size;
        meanA_src *= size;
        meanB_src *= size;
        sigmaL_src *= size;
        sigmaA_src *= size;
        sigmaB_src *= size;
        // Compute color info from around dst
        double meanL_ref = 0., meanA_ref = 0., meanB_ref = 0.;
        double sigmaL_ref = 0., sigmaA_ref = 0., sigmaB_ref = 0.;
        deviceIt = device->createRectIterator(pos.x() - healradius / 2, pos.y() - healradius / 2, healradius, healradius, false );
        while(!deviceIt.isDone())
        {
            deviceCs->toLabA16(deviceIt.rawData(), (Q_UINT8*)data, 1);
            Q_UINT32 L = data[0];
            Q_UINT32 A = data[1];
            Q_UINT32 B = data[2];
            meanL_ref += L;
            meanA_ref += A;
            meanB_ref += B;
            sigmaL_ref += L*L;
            sigmaA_ref += A*A;
            sigmaB_ref += B*B;
            ++deviceIt;
        }
        meanL_ref *= size;
        meanA_ref *= size;
        meanB_ref *= size;
        sigmaL_ref *= size;
        sigmaA_ref *= size;
        sigmaB_ref *= size;
        // Apply the color transformation
        KisRectIteratorPixel dstIt = srcdev->createRectIterator(0, 0, sw, sh, false );
        KisColorSpace* srcdevCs = srcdev->colorSpace();
        double coefL = sqrt((sigmaL_ref - meanL_ref * meanL_ref) / (sigmaL_src - meanL_src * meanL_src));
        double coefA = sqrt((sigmaA_ref - meanA_ref * meanA_ref) / (sigmaA_src - meanA_src * meanA_src));
        double coefB = sqrt((sigmaB_ref - meanB_ref * meanB_ref) / (sigmaB_src - meanB_src * meanB_src));
        kdDebug() << coefL << " " << coefA << " " << coefB << endl;
        while(!dstIt.isDone())
        {
            srcdevCs->toLabA16(dstIt.rawData(), (Q_UINT8*)data, 1);
            data[0] = (Q_UINT16)CLAMP( ( (double)data[0] - meanL_src) * coefL + meanL_ref, 0., 65535.);
            data[1] = (Q_UINT16)CLAMP( ( (double)data[1] - meanA_src) * coefA + meanA_ref, 0., 65535.);
            data[2] = (Q_UINT16)CLAMP( ( (double)data[2] - meanB_src) * coefB + meanB_ref, 0., 65535.);
            srcdevCs->fromLabA16((Q_UINT8*)data, dstIt.rawData(), 1);
            ++dstIt;
        }
    }
    
    // Add the dab as selection to the srcdev
    KisPainter copySelection(srcdev->selection().data());
    copySelection.bitBlt(0, 0, COMPOSITE_OVER, dab, 0, 0, sw, sh);
    copySelection.end();

    // copy the srcdev onto a new device, after applying the dab selection
    KisPaintDeviceSP target = new KisPaintDevice(srcdev->colorSpace(), "duplicate target dev");
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

    Q_INT32 sx = dstRect.x() - x;
    Q_INT32 sy = dstRect.y() - y;
    sw = dstRect.width();
    sh = dstRect.height();
    
    if (m_source->hasSelection()) {
        m_painter->bltSelection(dstRect.x(), dstRect.y(), m_painter->compositeOp(), target,
                                m_source->selection(), m_painter->opacity(), sx, sy, sw, sh);
    }
    else {
        m_painter->bitBlt(dstRect.x(), dstRect.y(), m_painter->compositeOp(), target, m_painter->opacity(), sx, sy, sw, sh);
    }


    m_painter->addDirtyRect(dstRect);
}
