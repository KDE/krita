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
    if (m_distanceTable != NULL){
        delete[] m_distanceTable;
    }

}

void DeformBrush::scale(qreal cursorX,qreal cursorY, qreal factor){
    int curXi = static_cast<int>(cursorX+0.5);
    int curYi = static_cast<int>(cursorY+0.5);
    //KoColor kcolor( m_dev->colorSpace() );

    int centerX = m_image->width()  / 2;
    int centerY = m_image->height() / 2;
    qreal newX, newY;

    qreal distance;
    qreal scaleFactor;

    for (int x = curXi - m_radius; x < curXi + m_radius;x++){
        for (int y = curYi - m_radius; y < curYi + m_radius;y++){
            newX = x - curXi;
            newY = y - curYi;

            // normalized distance
            distance = distanceFromCenter(abs(newX), abs(newY)); 

            // we want circle
            if (distance > 1.0) continue;
            scaleFactor = (1.0 - distance)*factor + distance;

            newX /= scaleFactor;
            newY /= scaleFactor;

            newX += curXi;
            newY += curYi;

            if (m_useBilinear){
                // fill the result to m_tempColor coz of optimalization [creating KoColor used to be slow..]
                bilinear_interpolation(newX, newY );
                m_writeAccessor->moveTo(x, y); 
                memcpy(m_writeAccessor->rawData(), m_tempColor->data() , m_pixelSize );
            }else
            if (point_interpolation(&newX,&newY,m_image)){
                // copy pixel
                m_readAccessor->moveTo(newX, newY);
                m_writeAccessor->moveTo(x, y); 

                //memcpy(kcolor.data(), m_readAccessor->rawData(), m_pixelSize);
                //memcpy(m_writeAccessor->rawData(), kcolor.data(), m_pixelSize );
                memcpy(m_writeAccessor->rawData(), m_readAccessor->rawData(), m_pixelSize );
            }

        }
    }
}

void DeformBrush::swirl(qreal cursorX,qreal cursorY, qreal alpha){
    int curXi = static_cast<int>(cursorX+0.5);
    int curYi = static_cast<int>(cursorY+0.5);

    //KoColor kcolor( m_dev->colorSpace() );

    int centerX = m_image->width()  / 2;
    int centerY = m_image->height() / 2;
    qreal newX, newY;

    qreal rotX, rotY;
    qreal distance;

    for (int x = curXi - m_radius; x <= curXi + m_radius;x++){
        for (int y = curYi - m_radius; y <= curYi + m_radius;y++){

            newX = x - curXi;
            newY = y - curYi;

            distance = distanceFromCenter(abs(newX), abs(newY));
            if (distance > 1.0) continue;

            distance = 1.0 - distance;
            rotX = cos(-alpha * distance) * newX - sin(-alpha * distance) * newY;
            rotY = sin(-alpha * distance) * newX + cos(-alpha * distance) * newY;

            newX = rotX;
            newY = rotY;

            newX += curXi;
            newY += curYi;

            if (m_useBilinear){
                // fill the result to m_tempColor coz of optimalization [creating KoColor used to be slow..]
                bilinear_interpolation(newX, newY );
                m_writeAccessor->moveTo(x, y); 
                memcpy(m_writeAccessor->rawData(), m_tempColor->data() , m_pixelSize );
            }else
            if (point_interpolation(&newX,&newY,m_image)){
                // copy pixel
                m_readAccessor->moveTo(newX, newY);
                m_writeAccessor->moveTo(x, y); 
                memcpy(m_writeAccessor->rawData(), m_readAccessor->rawData(), m_pixelSize );
            }
        }
    }
}


void DeformBrush::paint(KisPaintDeviceSP dev,KisPaintDeviceSP layer, const KisPaintInformation &info)
{
    qreal x1 = info.pos().x();
    qreal y1 = info.pos().y();

    m_dev = layer;

    if (m_useBilinear)
    {
        // only used when bilinear interepolation checked
        m_tempColor = new KoColor(m_dev->colorSpace());
    }

    m_pixelSize = dev->colorSpace()->pixelSize();

    KisRandomAccessor accessor = dev->createRandomAccessor((int)x1, (int)y1);
    m_writeAccessor = &accessor;

    KisRandomAccessor accessor2 = layer->createRandomAccessor((int)x1, (int)y1);
    m_readAccessor = &accessor2;

    if (m_action == 1){
        // grow
        //scale(x1,y1,1.0 + m_counter*m_counter/100.0);
        scale(x1,y1,1.0 + m_amount);
    } else 
    if (m_action == 2){
        // shrink
        //scale(x1,y1,1.0 - m_counter*m_counter/100.0);
        scale(x1,y1,1.0 - m_amount);
    } else 
    if (m_action == 3){
        // CW
        swirl(x1,y1, (1.0/360*m_counter) *  radToDeg);
    } else 
    if (m_action == 4){
        // CCW
        swirl(x1,y1, (1.0/360*m_counter) * -radToDeg);
    }
    m_counter++;
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

void DeformBrush::debugColor(const quint8* data){
    QColor rgbcolor;
    m_dev->colorSpace()->toQColor(data, &rgbcolor);
    dbgPlugins << "RGBA: ("
    << rgbcolor.red() 
    << ", "<< rgbcolor.green()
    << ", "<< rgbcolor.blue()
    << ", "<< rgbcolor.alpha() << ")";
}

void DeformBrush::precomputeDistances(int radius){
    int size = (radius+1)*(radius+1);
    m_distanceTable = new qreal[size];
    int pos = 0;

    for (int y = 0;y <= radius; y++)
        for (int x = 0; x <= radius; x++,pos++)
        {
            m_distanceTable[pos] = sqrt(x*x+y*y)/m_maxdist;
        }
}


/// result of bilinear interpolation is in m_tempColor
void DeformBrush::bilinear_interpolation(double x, double y ) {
    KoMixColorsOp * mixOp = m_dev->colorSpace()->mixColorsOp();

    int ix = (int)floor(x);
    int iy = (int)floor(y);

    if (  ix >= 0 && 
          ix <= m_image->width()-2 && 
          iy >= 0 && 
          iy <= m_image->height()-2)
    {
        const quint8 *colors[4];
        m_readAccessor->moveTo(ix, iy);
        colors[0] = m_readAccessor->rawData(); //11
    
        m_readAccessor->moveTo(ix+1, iy);
        colors[1] = m_readAccessor->rawData(); //12
    
        m_readAccessor->moveTo(ix, iy+1);
        colors[2] = m_readAccessor->rawData(); //21
    
        m_readAccessor->moveTo(ix+1, iy+1);
        colors[3] = m_readAccessor->rawData();  //22  
    
        double x_frac = x - (double)ix; 
        double y_frac = y - (double)iy;

        qint16 colorWeights[4];
        int MAX_16BIT = 255;

        colorWeights[0] = static_cast<quint16>( (1.0 - y_frac) * (1.0 - x_frac) * MAX_16BIT); 
        colorWeights[1] = static_cast<quint16>( (1.0 - y_frac) *  x_frac * MAX_16BIT); 
        colorWeights[2] = static_cast<quint16>(y_frac * (1.0 - x_frac) * MAX_16BIT);
        colorWeights[3] = static_cast<quint16>(y_frac * x_frac* MAX_16BIT);
        mixOp->mixColors(colors, colorWeights, 4, m_tempColor->data() );
        
#if 0
        // debug code
        debugColor(colors[0]);
        debugColor(colors[1]);
        debugColor(colors[2]);
        debugColor(colors[3]);
        debugColor(m_tempColor->data() );

        dbgPlugins << "w0,w1,w2,w3: ("
        << colorWeights[0] 
        << ", "<< colorWeights[1]
        << ", "<< colorWeights[2]
        << ", "<< colorWeights[3] << ")";
#endif

    }
}
