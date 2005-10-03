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
#include "kis_paint_device_impl.h"
#include "kis_painter.h"
#include "kis_types.h"
#include "kis_iterators_pixel.h"
#include "kis_paintop.h"

#include "kis_duplicateop.h"

KisPaintOp * KisDuplicateOpFactory::createOp(KisPainter * painter)
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
    
    KisPaintDeviceImplSP device = m_painter -> device();
    if (m_source) device = m_source;
    if (!device) return;

    KisBrush * brush = m_painter->brush();
    if (!brush) return;
    if (! brush -> canPaintFor(info) )
        return;

    KisPoint hotSpot = brush -> hotSpot(info);
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

    KisPaintDeviceImplSP dab = 0;

    if (brush -> brushType() == IMAGE || 
        brush -> brushType() == PIPE_IMAGE) {
        dab = brush -> image(device -> colorSpace(), info, xFraction, yFraction);
    }
    else {
        KisAlphaMaskSP mask = brush -> mask(info, xFraction, yFraction);
        dab = computeDab(mask);
    }
    
    m_painter -> setPressure(info.pressure);

    QPoint srcPoint = QPoint(x - static_cast<Q_INT32>(m_painter -> duplicateOffset().x()),
                             y - static_cast<Q_INT32>(m_painter -> duplicateOffset().y()));

        
    Q_INT32 sw = dab -> extent().width();
    Q_INT32 sh = dab -> extent().height();

    if (srcPoint.x() < 0 )
        srcPoint.setX(0);

    if( srcPoint.y() < 0)
        srcPoint.setY(0);

    KisPaintDeviceImplSP srcdev = new KisPaintDeviceImpl(dab.data() -> colorSpace(), "duplicate srcdev");
    Q_CHECK_PTR(srcdev);
    int srcY = 0;

    while ( srcY < sh )
    {
        KisHLineIterator srcLit = srcdev -> createHLineIterator(0, srcY, sw - 1, true);
        KisHLineIterator dabLit = dab.data() -> createHLineIterator(0, srcY, sw - 1, false);
        KisHLineIterator devLit = device -> createHLineIterator(srcPoint.x(), srcPoint.y() + srcY, srcPoint.x() + sw - 1, false);
        
        while( !srcLit.isDone() )
        {
            // XXX: This is 8-bit broken
            for( Q_INT32 i = 0; i < device -> colorSpace() -> nColorChannels(); i++) {
                Q_UINT8 devQ = (Q_UINT8) devLit.rawData()[ i ];
                Q_UINT8 dabQ = (Q_UINT8) dabLit.rawData()[ i ];
                srcLit.rawData()[ i ] = (Q_UINT8) (((Q_UINT8_MAX - dabQ) * (devQ) ) / Q_UINT8_MAX);
            }
            for( Q_INT32 i = device -> colorSpace() -> nColorChannels(); i < device -> colorSpace() -> nChannels(); i++) {
                Q_UINT8 devQ = (Q_UINT8) devLit.rawData()[ i ];
                Q_UINT8 dabQ = (Q_UINT8) dabLit.rawData()[ i ];
                srcLit.rawData()[ i ] = (Q_UINT8) ((dabQ * devQ) / Q_UINT8_MAX);
            }
            
            ++srcLit; ++dabLit; ++devLit;
        }
        srcY++;
    }

    QRect dabRect = QRect(0, 0, brush -> maskWidth(info), brush -> maskHeight(info));
    QRect dstRect = QRect(x, y, dabRect.width(), dabRect.height());

    KisImage * image = device -> image();

    if (image != 0) {
        dstRect &= image -> bounds();
    }

    if (dstRect.isNull() || dstRect.isEmpty() || !dstRect.isValid()) return;

    Q_INT32 sx = dstRect.x() - x;
    Q_INT32 sy = dstRect.y() - y;
    sw = dstRect.width();
    sh = dstRect.height();

    m_painter -> bltSelection(dstRect.x(), dstRect.y(), m_painter -> compositeOp(), srcdev.data(), m_painter -> opacity(), sx, sy, sw, sh);
    m_painter -> addDirtyRect(dstRect);
}
