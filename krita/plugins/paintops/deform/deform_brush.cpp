/*
 *  Copyright (c) 2008,2010 Lukáš Tvrdý <lukast.dev@gmail.com>
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

#include "kis_fixed_paint_device.h"

#include <KoColor.h>
#include <KoColorSpace.h>

#include <kis_types.h>
#include <kis_random_sub_accessor.h>

#include <cmath>
#include <ctime>

const qreal radToDeg = 57.29578;
const qreal degToRad = M_PI / 180.0;

#if defined(_WIN32) || defined(_WIN64)
#define srand48 srand
inline double drand48()
{
    return double(rand()) / RAND_MAX;
}
#endif

DeformBrush::DeformBrush()
{
    m_firstPaint = false;
    m_counter = 1;
}

DeformBrush::~DeformBrush()
{
/*    if (m_distanceTable != 0) {
        delete[] m_distanceTable;
    }*/
}



/// this method uses KisSubPixelAccessor
inline void DeformBrush::movePixel(qreal newX, qreal newY, quint8 *dst)
{
    if (!m_properties->useBilinear) {
        newX = qRound(newX);
        newY = qRound(newY);
    }
    m_srcAcc->moveTo(newX, newY);

    if (m_properties->useOldData) {
        m_srcAcc->sampledOldRawData(dst);
    } else {
        m_srcAcc->sampledRawData(dst);
    }
    
}


void DeformBrush::maskScale(KisFixedPaintDeviceSP dab,
                            KisPaintDeviceSP layer, 
                            qreal scale, qreal rotation, QPointF pos,qreal subPixelX, qreal subPixelY, qreal factor)
{
    int dstWidth =  m_sizeProperties->diameter;
    int dstHeight = m_sizeProperties->diameter;

    QRectF m_maskRect(0,0,m_sizeProperties->diameter, m_sizeProperties->diameter);
    
    int w = dab->bounds().width();
    int h = dab->bounds().height();
   
    // clear
    if (w!=dstWidth || h!=dstHeight){
        dab->setRect(m_maskRect.toRect());
        dab->initialize();
    }else{
        dab->clear(m_maskRect.toRect());
    }
    
    qreal centerX = dstWidth  * 0.5 - 1.0 + subPixelX;
    qreal centerY = dstHeight * 0.5 - 1.0 + subPixelY;

    quint8* dabPointer = dab->data();
    
    
    qreal newX = 0.0;
    qreal newY = 0.0;
    for (int y = 0; y <  dstHeight; y++){
        for (int x = 0; x < dstWidth; x++){
            double maskX = (x - centerX);
            double maskY = (y - centerY);

            qreal distance = sqrt(maskX*maskX + maskY*maskY) / (m_sizeProperties->diameter*0.5);
            
            if (distance > 1.0) {
                dabPointer += m_pixelSize;
                continue;
            }

            qreal scaleFactor = (1.0 - distance) * factor + distance;

            newX = maskX / scaleFactor;
            newY = maskY / scaleFactor;

            newX += pos.x();
            newY += pos.y();
            
            movePixel(newX, newY, dabPointer);    
            
            dabPointer += m_pixelSize;
        }
    }
}


void DeformBrush::maskSwirl(KisFixedPaintDeviceSP dab, KisPaintDeviceSP layer, qreal scale, qreal rotation, QPointF pos, qreal subPixelX, qreal subPixelY, qreal factor)
{
    int dstWidth =  m_sizeProperties->diameter;
    int dstHeight = m_sizeProperties->diameter;

    QRectF m_maskRect(0,0,m_sizeProperties->diameter, m_sizeProperties->diameter);
    
    int w = dab->bounds().width();
    int h = dab->bounds().height();
   
    // clear
    if (w!=dstWidth || h!=dstHeight){
        dab->setRect(m_maskRect.toRect());
        dab->initialize();
    }else{
        dab->clear(m_maskRect.toRect());
    }
    
    qreal centerX = dstWidth  * 0.5 - 1.0 + subPixelX;
    qreal centerY = dstHeight * 0.5 - 1.0 + subPixelY;

    quint8* dabPointer = dab->data();
    
    qreal newX, newY ;
    qreal rotX, rotY;
    //TODO:alpha
    qreal alpha = factor;
    for (int y = 0; y <  dstHeight; y++){
        for (int x = 0; x < dstWidth; x++){
            newX = (x - centerX);
            newY = (y - centerY);

            qreal distance = sqrt(newX*newX + newY*newY) / (m_sizeProperties->diameter*0.5);
            if (distance > 1.0) {
                dabPointer += m_pixelSize;
                continue;
            }
            distance = 1.0 - distance;
            rotX = cos(-alpha * distance) * newX - sin(-alpha * distance) * newY;
            rotY = sin(-alpha * distance) * newX + cos(-alpha * distance) * newY;

            newX = rotX;
            newY = rotY;

            newX += pos.x();
            newY += pos.y();
            
            movePixel(newX, newY, dabPointer);    
            
            dabPointer += m_pixelSize;
        }
    }

}


void DeformBrush::maskMove(KisFixedPaintDeviceSP dab, KisPaintDeviceSP layer, qreal scale, qreal rotation, QPointF pos, qreal subPixelX, qreal subPixelY, qreal dx, qreal dy)
{
    int dstWidth =  m_sizeProperties->diameter;
    int dstHeight = m_sizeProperties->diameter;

    QRectF m_maskRect(0,0,m_sizeProperties->diameter, m_sizeProperties->diameter);
    
    int w = dab->bounds().width();
    int h = dab->bounds().height();
   
    // clear
    if (w!=dstWidth || h!=dstHeight){
        dab->setRect(m_maskRect.toRect());
        dab->initialize();
    }else{
        dab->clear(m_maskRect.toRect());
    }
    
    qreal centerX = dstWidth  * 0.5 - 1.0 + subPixelX;
    qreal centerY = dstHeight * 0.5 - 1.0 + subPixelY;

    quint8* dabPointer = dab->data();
    
    qreal newX, newY ;
    
    for (int y = 0; y <  dstHeight; y++){
        for (int x = 0; x < dstWidth; x++){
            newX = (x - centerX);
            newY = (y - centerY);

            qreal distance = sqrt(newX*newX + newY*newY) / (m_sizeProperties->diameter*0.5);
            if (distance > 1.0) {
                dabPointer += m_pixelSize;
                continue;
            }
            newX -= dx * m_properties->deformAmount * (1.0 - distance);
            newY -= dy * m_properties->deformAmount * (1.0 - distance);

            newX += pos.x();
            newY += pos.y();
            
            movePixel(newX, newY, dabPointer);    
            
            dabPointer += m_pixelSize;
        }
    }
}



void DeformBrush::maskLensDistortion(KisFixedPaintDeviceSP dab, KisPaintDeviceSP layer, qreal scale, qreal rotation, QPointF pos, qreal subPixelX, qreal subPixelY, qreal k1, qreal k2)
{
    int dstWidth =  m_sizeProperties->diameter;
    int dstHeight = m_sizeProperties->diameter;

    QRectF m_maskRect(0,0,m_sizeProperties->diameter, m_sizeProperties->diameter);
    
    int w = dab->bounds().width();
    int h = dab->bounds().height();
   
    // clear
    if (w!=dstWidth || h!=dstHeight){
        dab->setRect(m_maskRect.toRect());
        dab->initialize();
    }else{
        dab->clear(m_maskRect.toRect());
    }
    
    qreal centerX = dstWidth  * 0.5 - 1.0 + subPixelX;
    qreal centerY = dstHeight * 0.5 - 1.0 + subPixelY;

    quint8* dabPointer = dab->data();
    
    qreal newX, newY ;
    m_maxdist = m_sizeProperties->diameter * 0.5;
    for (int y = 0; y <  dstHeight; y++){
        for (int x = 0; x < dstWidth; x++){
            newX = (x - centerX);
            newY = (y - centerY);

            qreal distance = sqrt(newX*newX + newY*newY) / (m_sizeProperties->diameter*0.5);
            if (distance > 1.0) {
                dabPointer += m_pixelSize;
                continue;
            }

            //normalize
            newX /= m_maxdist;
            newY /= m_maxdist;

            qreal radius_2 = newX * newX + newY * newY;
            qreal radius_4 = radius_2 * radius_2;

            if (m_properties->action == 7) {
                newX = newX * (1.0 + k1 * radius_2 + k2 * radius_4);
                newY = newY * (1.0 + k1 * radius_2 + k2 * radius_4);
            } else {
                newX = newX / (1.0 + k1 * radius_2 + k2 * radius_4);
                newY = newY / (1.0 + k1 * radius_2 + k2 * radius_4);
            }

            newX = m_maxdist * newX;
            newY = m_maxdist * newY;


            newX += pos.x();
            newY += pos.y();
            
            movePixel(newX, newY, dabPointer);    
            
            dabPointer += m_pixelSize;
        }
    }
}




void DeformBrush::maskDeformColor(KisFixedPaintDeviceSP dab, KisPaintDeviceSP layer, qreal scale, qreal rotation, QPointF pos, qreal subPixelX, qreal subPixelY, qreal amount)
{
    int dstWidth =  m_sizeProperties->diameter;
    int dstHeight = m_sizeProperties->diameter;

    QRectF m_maskRect(0,0,m_sizeProperties->diameter, m_sizeProperties->diameter);
    
    int w = dab->bounds().width();
    int h = dab->bounds().height();
   
    // clear
    if (w!=dstWidth || h!=dstHeight){
        dab->setRect(m_maskRect.toRect());
        dab->initialize();
    }else{
        dab->clear(m_maskRect.toRect());
    }
    
    qreal centerX = dstWidth  * 0.5 - 1.0 + subPixelX;
    qreal centerY = dstHeight * 0.5 - 1.0 + subPixelY;

    quint8* dabPointer = dab->data();
    
    qreal newX, newY ;
    qreal randomX, randomY;
    srand48(time(0)); 
    qreal radius = pow(m_sizeProperties->diameter*0.5,2);
    for (int y = 0; y <  dstHeight; y++){
        for (int x = 0; x < dstWidth; x++){
            newX = qRound(x - centerX);
            newY = qRound(y - centerY);

            if (newX*newX + newY*newY >  radius)  {
                dabPointer += m_pixelSize;
                continue;
            }

            randomX = amount * (drand48() * 2.0) - 1.0;
            randomY = amount * (drand48() * 2.0) - 1.0;
            
            newX += randomX;
            newY += randomY;

            newX += pos.x();
            newY += pos.y();

            movePixel(newX, newY, dabPointer);    
            dabPointer += m_pixelSize;
        }
    }
}


void DeformBrush::paint(KisFixedPaintDeviceSP dab, KisPaintDeviceSP layer, qreal scale, qreal rotation, QPointF pos, qreal subPixelX, qreal subPixelY)
{
    qreal x1 = pos.x();
    qreal y1 = pos.y();

    //m_dab = dev;
    m_pixelSize = layer->colorSpace()->pixelSize();

    KisRandomSubAccessorPixel srcAcc = layer->createRandomSubAccessor();
    m_srcAcc = &srcAcc;

    switch(m_properties->action){
        case 1: {
            if (m_properties->useCounter) {
                maskScale(dab,layer,scale, rotation, pos,subPixelX,subPixelY,1.0 + m_counter*m_counter / 100.0);
            } else {
                maskScale(dab,layer,scale, rotation, pos,subPixelX,subPixelY,1.0 + m_properties->deformAmount);
            }
            break;
        }
        case 2: {
            if (m_properties->useCounter) {
                maskScale(dab,layer,scale, rotation, pos,subPixelX,subPixelY,1.0 - m_counter*m_counter / 100.0);
            } else {
                maskScale(dab,layer,scale, rotation, pos,subPixelX,subPixelY,1.0 - m_properties->deformAmount);
            }
            break;
        }
        case 3: {
            if (m_properties->useCounter) {
                maskSwirl(dab,layer,scale, rotation, pos,subPixelX,subPixelY,(m_counter) *  degToRad);
            } else {
                maskSwirl(dab,layer,scale, rotation, pos,subPixelX,subPixelY,(360 * m_properties->deformAmount * 0.5) *  degToRad);
            }
            break;
        }
        case 4: {
            if (m_properties->useCounter) {
                maskSwirl(dab,layer,scale, rotation, pos,subPixelX,subPixelY,(m_counter) *  -degToRad);
            } else {
                maskSwirl(dab,layer,scale, rotation, pos,subPixelX,subPixelY,(360 * m_properties->deformAmount * 0.5) *  -degToRad);
            }
            break;
        }
        case 5: {
                if (m_firstPaint == false) {
                    m_prevX = x1;
                    m_prevY = y1;
                    m_firstPaint = true;
                } else {
                    maskMove(dab,layer,scale, rotation, pos,subPixelX,subPixelY,x1 - m_prevX, y1 - m_prevY);
                    m_prevX = x1;
                    m_prevY = y1;
                }
            break;
        }
        case 6: 
        case 7:
        {
            maskLensDistortion(dab,layer,scale, rotation, pos,subPixelX,subPixelY,m_properties->deformAmount, 0);
            break;
        }
        case 8:
        {
            maskDeformColor(dab,layer,scale, rotation, pos,subPixelX,subPixelY,m_properties->deformAmount);
            break;
        }
        default: break;
    }
    m_counter++;
}

void DeformBrush::debugColor(const quint8* data, KoColorSpace * cs)
{
    QColor rgbcolor;
    cs->toQColor(data, &rgbcolor);
    dbgPlugins << "RGBA: ("
    << rgbcolor.red()
    << ", " << rgbcolor.green()
    << ", " << rgbcolor.blue()
    << ", " << rgbcolor.alpha() << ")";
}

// void DeformBrush::precomputeDistances(int radius)
// {
//     int size = (radius + 1) * (radius + 1);
//     m_distanceTable = new qreal[size];
//     int pos = 0;
// 
//     for (int y = 0; y <= radius; y++)
//         for (int x = 0; x <= radius; x++, pos++) {
//             m_distanceTable[pos] = sqrt(x * x + y * y) / m_maxdist;
//         }
// }


// void DeformBrush::bilinear_interpolation(double x, double y, quint8 *dst)
// {
//     KoMixColorsOp * mixOp = m_dev->colorSpace()->mixColorsOp();
// 
//     int ix = (int)floor(x);
//     int iy = (int)floor(y);
//     
//     const quint8 *colors[4];
//     m_readAccessor->moveTo(ix, iy);
//     colors[0] = m_readAccessor->rawData(); //11
// 
//     m_readAccessor->moveTo(ix + 1, iy);
//     colors[1] = m_readAccessor->rawData(); //12
// 
//     m_readAccessor->moveTo(ix, iy + 1);
//     colors[2] = m_readAccessor->rawData(); //21
// 
//     m_readAccessor->moveTo(ix + 1, iy + 1);
//     colors[3] = m_readAccessor->rawData();  //22
// 
//     double x_frac = x - (double)ix;
//     double y_frac = y - (double)iy;
// 
//     qint16 colorWeights[4];
//     int MAX_16BIT = 255;
// 
//     colorWeights[0] = static_cast<quint16>((1.0 - y_frac) * (1.0 - x_frac) * MAX_16BIT);
//     colorWeights[1] = static_cast<quint16>((1.0 - y_frac) *  x_frac * MAX_16BIT);
//     colorWeights[2] = static_cast<quint16>(y_frac * (1.0 - x_frac) * MAX_16BIT);
//     colorWeights[3] = static_cast<quint16>(y_frac * x_frac * MAX_16BIT);
// 
//     mixOp->mixColors(colors, colorWeights, 4, dst);
// }


// void DeformBrush::bilinear_interpolation_old(double x, double y , quint8 *dst)
// {
//     KoMixColorsOp * mixOp = m_dev->colorSpace()->mixColorsOp();
// 
//     int ix = (int)floor(x);
//     int iy = (int)floor(y);
//     
//     const quint8 *colors[4];
//     m_readAccessor->moveTo(ix, iy);
//     colors[0] = m_readAccessor->oldRawData(); //11
// 
//     m_readAccessor->moveTo(ix + 1, iy);
//     colors[1] = m_readAccessor->oldRawData(); //12
// 
//     m_readAccessor->moveTo(ix, iy + 1);
//     colors[2] = m_readAccessor->oldRawData(); //21
// 
//     m_readAccessor->moveTo(ix + 1, iy + 1);
//     colors[3] = m_readAccessor->oldRawData();  //22
// 
//     double x_frac = x - (double)ix;
//     double y_frac = y - (double)iy;
// 
//     qint16 colorWeights[4];
//     int MAX_16BIT = 255;
// 
//     colorWeights[0] = static_cast<quint16>((1.0 - y_frac) * (1.0 - x_frac) * MAX_16BIT);
//     colorWeights[1] = static_cast<quint16>((1.0 - y_frac) *  x_frac * MAX_16BIT);
//     colorWeights[2] = static_cast<quint16>(y_frac * (1.0 - x_frac) * MAX_16BIT);
//     colorWeights[3] = static_cast<quint16>(y_frac * x_frac * MAX_16BIT);
//     mixOp->mixColors(colors, colorWeights, 4, dst);
// }
