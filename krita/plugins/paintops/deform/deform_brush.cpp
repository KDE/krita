/*
 *  Copyright (c) 2008 Lukas Tvrdy <lukast.dev@gmail.com>
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

#include "deform_brush.h"

#include <KoColor.h>
#include <KoColorSpace.h>
#include <KoColorTransformation.h>

#include <QVariant>
#include <QHash>
#include <QList>

#include "kis_random_accessor.h"

#include <cmath>
#include <ctime>

const qreal radToDeg = 57.29578;

#ifdef _WIN32
#define srand48 srand
#define drand48 rand
#endif

DeformBrush::DeformBrush()
{
}

DeformBrush::~DeformBrush()
{
}

void DeformBrush::swirl(qreal cursorX,qreal cursorY, qreal alpha, qreal scale ) {

    int curXi = static_cast<int>(cursorX+0.5);
    int curYi = static_cast<int>(cursorY+0.5);

    int centerX = m_image->width()  / 2;
    int centerY = m_image->height() / 2;

    //double norm =  sqrt(centerX*centerX + centerY*centerY);
    //double t = 0.0;    

    qreal newX = 0, newY = 0;
    qreal rotX = 0, rotY = 0;

    for (int x = curXi - m_radius; x < curXi + m_radius;x++){
        for (int y = curYi - m_radius; y < curYi + m_radius;y++){
            newX = x - centerX;
            newY = y - centerY;
            //t = sqrt(newX*newX + newY*newY);
            //t /= norma;
            //rotX = cos(-alpha * t) * newX - sin(-alpha * t) * newY;
            //rotY = sin(-alpha * t) * newX + cos(-alpha * t) * newY;

            rotX = cos(-alpha) * newX - sin(-alpha ) * newY;
            rotY = sin(-alpha) * newX + cos(-alpha ) * newY;

            // scale 
            rotX = newX/scale;
            rotY = newY/scale;

            newX = rotX + centerX;
            newY = rotY + centerY;

            if (point_interpolation( &newX, &newY, m_image ) )
            {
                // read pixelValue
                m_readAccessor->moveTo(newX,newY);
                m_writeAccessor->moveTo(x,y);
                memcpy(m_writeAccessor->rawData() , m_readAccessor->oldRawData() , m_pixelSize );
            }
        }
    }

}

void DeformBrush::paint(KisPaintDeviceSP dev,KisPaintDeviceSP layer, const KisPaintInformation &info)
{
    qreal x1 = info.pos().x();
    qreal y1 = info.pos().y();

    m_pixelSize = dev->colorSpace()->pixelSize();
    
    KisRandomAccessor accessor = dev->createRandomAccessor((int)x1, (int)y1);
    m_writeAccessor = &accessor;

    KisRandomAccessor accessor2 = layer->createRandomAccessor((int)x1, (int)y1);
    m_readAccessor = &accessor2;

    m_dev = dev;

    swirl(x1,y1,45*radToDeg,1);

    m_dev = 0;
    //m_accessor = 0;
}


void DeformBrush::paintLine(KisPaintDeviceSP dev,KisPaintDeviceSP layer, const KisPaintInformation &pi1, const KisPaintInformation &pi2)
{
    //dbgKritaPlugins << "Whaaat!";
#if 0    
    qreal dx = pi2.pos().x() - pi1.pos().x();
    qreal dy = pi2.pos().y() - pi1.pos().y();

    qreal x1 = pi1.pos().x();
    qreal y1 = pi1.pos().y();

    qreal x2 = pi2.pos().x();
    qreal y2 = pi2.pos().y();

    qreal angle = atan2(dy, dx);

    /*
    qreal slope = 0.0;
    if (dx != 0){
        slope = dy / dx;
    } 
    dbgPlugins << "slope: " << slope;
    */
    qreal distance = sqrt(dx * dx + dy * dy);
    qreal pressure = pi2.pressure();

    KisRandomAccessor accessor = dev->createRandomAccessor((int)x1, (int)y1);
    m_pixelSize = dev->colorSpace()->pixelSize();
    m_accessor = &accessor;
    m_dev = dev;

    m_dev = 0;
    m_accessor = 0;
#endif 
}


bool DeformBrush::point_interpolation( qreal* x, qreal* y, KisImageSP image ) {
    if ( *x >= 0 && *x < image->width()-1 && *y >= 0 && *y < image->height()-1){
        *x = *x + 0.5; // prepare for typing to int
        *y = *y + 0.5;
       return true;
    }
    return false;
}

#if 0
CvScalar bilinear_interpolation( KisPaintDevice dev, double x, double y ) {
    CvScalar pixel = cvScalar( 0.0 );

    int x1 = (int)floor(u);
    int y1 = (int)floor(v);


    if (  x1 >= 0 && 
          x1 <= img->width-2 && 
          y1 >= 0 && 
          y1 <= img->height-2)
    {

    CvScalar p11 = cvGet2D (img, y1, x1);
    CvScalar p12 = cvGet2D (img, y1, x1 +1);
    CvScalar p21 = cvGet2D (img, y1+1,x1);
    CvScalar p22 = cvGet2D (img, y1+1, x1+1);

    double x = u - (double)x1; 
    double y = v - (double)y1;
      
    for (int c = 0; c < img->nChannels; c++)
    {
      pixel.val[c] = (
        p11.val[c] * (1.0 - y) * (1.0 - x) +
        p12.val[c] * (1.0 - y) *  x +
        p21.val[c] * y * (1.0 - x) +
        p22.val[c] * y * x
      );
    }

    }
      

      return pixel;
}
#endif