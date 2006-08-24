/*
 * This file is part of Krita
 *
 *  Copyright (c) 2006 Cyrille Berger <cberger@cberger.net>
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

#include "kis_perspectivetransform_worker.h"

#include "kis_iterators_pixel.h"
#include "kis_paint_device.h"
#include "kis_perspective_math.h"
#include "kis_random_sub_accessor.h"
#include "kis_selection.h"


KisPerspectiveTransformWorker::KisPerspectiveTransformWorker(KisPaintDeviceSP dev, const KisPoint& topLeft, const KisPoint& topRight, const KisPoint& bottomLeft, const KisPoint& bottomRight)
    : KisProgressSubject(), m_dev(dev)
{
    // Do some maths here :)
    kdDebug() << "topLeft = " << topLeft <<  " topRight = " << topRight << " bottomLeft = " << bottomLeft << " bottomRight = " << bottomRight << endl;
    // Computing vanishing points
    KisPerspectiveMath::LineEquation left = KisPerspectiveMath::computeLineEquation( &topLeft, &bottomLeft);
    kdDebug() << "left: " << left.a << " " << left.b << endl;
    KisPerspectiveMath::LineEquation right = KisPerspectiveMath::computeLineEquation( &topRight, &bottomRight);
    kdDebug() << "right: " << right.a << " " << right.b << endl;
    KisPerspectiveMath::LineEquation top = KisPerspectiveMath::computeLineEquation( &topLeft, &topRight);
    kdDebug() << "top: " << top.a << " " << top.b << endl;
    KisPerspectiveMath::LineEquation bottom = KisPerspectiveMath::computeLineEquation( &bottomLeft, &bottomRight);
    kdDebug() << "bottom: " << bottom.a << " " << bottom.b << endl;
    KisPoint  horizontalVP = KisPerspectiveMath::computeIntersection( top, bottom);
    KisPoint verticalVP = KisPerspectiveMath::computeIntersection( left, right);
    // Compute the center
    m_xcenter = verticalVP.x();
    m_ycenter = horizontalVP.y();
    // Compute p and q
    m_q = 1/verticalVP.y();
    m_p = 1/horizontalVP.x();
    kdDebug() << m_xcenter << " "  << verticalVP.y() << " " << horizontalVP.x() << " " << m_ycenter << " " << m_q << " " << m_p << endl;
}


KisPerspectiveTransformWorker::~KisPerspectiveTransformWorker()
{
}

void KisPerspectiveTransformWorker::run()
{
    KisPerspectiveMath pm(m_p, m_q);
    KisColorSpace * cs = m_dev->colorSpace();
    Q_UINT8 pixelSize = m_dev->pixelSize();
    QRect r;
    if(m_dev->hasSelection())
        r = m_dev->selection()->selectedExactRect();
    else
        r = m_dev->exactBounds();
    if(m_dev->hasSelection())
        m_dev->selection()->clear();
    kdDebug() << "r = " << r << endl;
    KisRectIteratorPixel dstIt = m_dev->createRectIterator(r.x(), r.y(), r.width(), r.height(), true); 
    KisPaintDeviceSP srcdev = new KisPaintDevice(*m_dev.data());

    KisRandomSubAccessorPixel srcAcc = srcdev->createRandomSubAccessor();
    KisPoint center(m_xcenter, m_ycenter);
    while(!dstIt.isDone())
    {
        if(dstIt.isSelected())
        {
//         kdDebug() << KisPoint(dstIt.x(), dstIt.y() ) << " " << pm.fromPerspectiveCoordinate( KisPoint(dstIt.x(), dstIt.y() ) -center ) + center << endl;
            srcAcc.moveTo( pm.fromPerspectiveCoordinate( KisPoint(dstIt.x(), dstIt.y() ) -center ) + center );
            srcAcc.sampledRawData( dstIt.rawData() );
            // TODO: Should set alpha = alpha*(1-selectedness)
            cs->setAlpha( dstIt.rawData(), 255, 1);
        } else {
            cs->setAlpha( dstIt.rawData(), 0, 1);
        }
        ++dstIt;
    }
}
