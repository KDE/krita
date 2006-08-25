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
    kdDebug() << topLeft << " " << topRight << " " << bottomLeft << " " << bottomRight << endl;
    {
        double dx1, dx2, dx3, dy1, dy2, dy3;

        dx1 = topRight.x() - bottomRight.x();
        dx2 = bottomLeft.x() - bottomRight.x();
        dx3 = topLeft.x() - topRight.x() + bottomRight.x() - bottomLeft.x();

        dy1 = topRight.y() - bottomRight.y();
        dy2 = bottomLeft.y() - bottomRight.y();
        dy3 = topLeft.y() - topRight.y() + bottomRight.y() - bottomLeft.y();

        /*  Is the mapping affine?  */
        if ((dx3 == 0.0) && (dy3 == 0.0))
        {
            m_matrix[0][0] = topRight.x() - topLeft.x();
            m_matrix[0][1] = bottomRight.x() - topRight.x();
            m_matrix[0][2] = topLeft.x();
            m_matrix[1][0] = topRight.y() - topLeft.y();
            m_matrix[1][1] = bottomRight.y() - topRight.y();
            m_matrix[1][2] = topLeft.y();
            m_matrix[2][0] = 0.0;
            m_matrix[2][1] = 0.0;
        }
        else
        {
            double det1, det2;

            det1 = dx3 * dy2 - dy3 * dx2;
            det2 = dx1 * dy2 - dy1 * dx2;

            if (det1 == 0.0 && det2 == 0.0)
                m_matrix[2][0] = 1.0;
            else
                m_matrix[2][0] = det1 / det2;

            det1 = dx1 * dy3 - dy1 * dx3;

            if (det1 == 0.0 && det2 == 0.0)
                m_matrix[2][1] = 1.0;
            else
                m_matrix[2][1] = det1 / det2;

            m_matrix[0][0] = topRight.x() - topLeft.x() + m_matrix[2][0] * topRight.x();
            m_matrix[0][1] = bottomLeft.x() - topLeft.x() + m_matrix[2][1] * bottomLeft.x();
            m_matrix[0][2] = topLeft.x();

            m_matrix[1][0] = topRight.y() - topLeft.y() + m_matrix[2][0] * topRight.y();
            m_matrix[1][1] = bottomLeft.y() - topLeft.y() + m_matrix[2][1] * bottomLeft.y();
            m_matrix[1][2] = topLeft.y();
        }

        m_matrix[2][2] = 1.0;
    }

}


KisPerspectiveTransformWorker::~KisPerspectiveTransformWorker()
{
}

void KisPerspectiveTransformWorker::run()
{
    KisColorSpace * cs = m_dev->colorSpace();
    Q_UINT8 pixelSize = m_dev->pixelSize();
    QRect r;
    if(m_dev->hasSelection())
        r = m_dev->selection()->selectedExactRect();
    else
        r = m_dev->exactBounds();
    if(m_dev->hasSelection())
        m_dev->selection()->clear();
    
    // TODO: in 2.0 with eigen :
    // m_matrix = m_matrix * [ sx 0 sx*tx ],[0 sy sy*ty ], [0 0 1 ]
    //Translation

    double matrix[3][3];
    double t00 = 1./r.width();
    double t02 = -t00*r.x();
    for(int j = 0; j < 3; j++)
    {
        matrix[j][0]  = t00 * m_matrix[j][0];
        matrix[j][0] += t02 * m_matrix[j][2];
    }
    
    double t11 = 1./r.height();
    double t12 = -t11*r.y();
    for(int j = 0; j < 3; j++)
    {
        matrix[j][1]  = t11 * m_matrix[j][1];
        matrix[j][1] += t12 * m_matrix[j][2];
    }

    for(int j = 0; j < 3; j++)
    {
        matrix[j][2]  = m_matrix[j][2];
    }
    
    kdDebug() << "r = " << r << endl;
    KisRectIteratorPixel dstIt = m_dev->createRectIterator(r.x(), r.y(), r.width(), r.height(), true); 
    KisPaintDeviceSP srcdev = new KisPaintDevice(*m_dev.data());

    KisRandomSubAccessorPixel srcAcc = srcdev->createRandomSubAccessor();
    while(!dstIt.isDone())
    {
        if(dstIt.isSelected())
        {
            KisPoint p;
            double sf = ( dstIt.x() * matrix[2][0] + dstIt.y() * matrix[2][1] + 1.0);
            sf = (sf == 0.) ? 1. : 1./sf;
            p.setX( ( dstIt.x() * matrix[0][0] + dstIt.y() * matrix[0][1] + matrix[0][2] ) * sf );
            p.setY( ( dstIt.x() * matrix[1][0] + dstIt.y() * matrix[1][1] + matrix[1][2] ) * sf );
//             kdDebug() << dstIt.x() << " " << dstIt.y() << " " << p << endl;
            srcAcc.moveTo( p );
            srcAcc.sampledRawData( dstIt.rawData() );
            // TODO: Should set alpha = alpha*(1-selectedness)
            cs->setAlpha( dstIt.rawData(), 255, 1);
        } else {
            cs->setAlpha( dstIt.rawData(), 0, 1);
        }
        ++dstIt;
    }
}
