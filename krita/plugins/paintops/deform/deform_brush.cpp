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
#include "kis_painter.h"

#include "kis_fixed_paint_device.h"

#include <KoColor.h>
#include <KoColorSpace.h>

#include <QRect>

#include <kis_types.h>
#include <kis_random_sub_accessor.h>

#include <cmath>
#include <ctime>

const qreal radToDeg = 57.29578;
const qreal degToRad = M_PI / 180.0;


DeformBrush::DeformBrush()
{
    m_firstPaint = false;
    m_counter = 1;
    m_deformAction = 0;
}

DeformBrush::~DeformBrush()
{
    delete m_deformAction;
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


void DeformBrush::fastDeformColor(KisPaintDeviceSP dab,KisPaintDeviceSP layer, QPointF pos, qreal amount)
{
    if (!setupAction(pos)){
        return;
    }
    
    KisRandomSubAccessorPixel srcAcc = layer->createRandomSubAccessor();
    m_srcAcc = &srcAcc;
    m_pixelSize = layer->pixelSize();
    
    int curXi = static_cast<int>(pos.x() + 0.5);
    int curYi = static_cast<int>(pos.y() + 0.5);
    
    qreal x, y;
    qreal distance;
    qreal randomX, randomY;

    int width = 0.5 * m_sizeProperties->diameter;
    int height =  0.5 * m_sizeProperties->diameter * m_sizeProperties->aspect;
    int left = curXi - width;
    int top = curYi - height;
    int w = width * 2 + 1;
    int h = height * 2 + 1;

    qreal maskX;
    qreal maskY;
    // major axis
    m_majorAxis = 2.0/w;
    // minor axis
    m_minorAxis = 2.0/h;

    KisRectIterator m_srcIt = dab->createRectIterator(left, top, w , h);
    for (; !m_srcIt.isDone(); ++m_srcIt) {
        x = m_srcIt.x();
        y = m_srcIt.y();
        maskX = x - curXi;
        maskY = y - curYi;
        
        distance = norme(maskX * m_majorAxis, maskY * m_minorAxis);
        if (distance > 1.0){
                continue;
        }

        m_deformAction->transform( &maskX, &maskY, distance);

        maskX += x;
        maskY += y;

        movePixel(maskX, maskY, m_srcIt.rawData());
    }
}



void DeformBrush::initDeformAction()
{
    switch(m_properties->action){
        case 1:
        case 2: 
        {
            m_deformAction = new DeformScale();
            break;
        }
        case 3:
        case 4:
        {
            m_deformAction = new DeformRotation();
            break;
        }
        
        case 5:
        {
            m_deformAction = new DeformMove();
            static_cast<DeformMove*>(m_deformAction)->setFactor(m_properties->deformAmount);
            break;
        }
        case 6:
        case 7:
        {
            m_deformAction = new DeformLens(); 
            static_cast<DeformLens*>(m_deformAction)->setLensFactor(m_properties->deformAmount,0.0);
            static_cast<DeformLens*>(m_deformAction)->setMode(m_properties->action == 7);
            break;
        }
        case 8:
        {
            m_deformAction = new DeformColor();
            static_cast<DeformColor*>(m_deformAction)->setFactor(m_properties->deformAmount);
            break;
        }
        default:{
            m_deformAction = new DeformBase();
            break;
        }
    }
}

bool DeformBrush::setupAction(QPointF pos)
{
    
    switch(m_properties->action){
        case 1:
        case 2: 
        {
            // grow or shrink, the sign decide
            qreal sign = (m_properties->action == 1) ? 1.0 : -1.0;
            qDebug() << sign;
            qreal factor;
            if (m_properties->useCounter){
                factor = (1.0 + sign*(m_counter*m_counter / 100.0));
            } else{
                factor =  (1.0 + sign*(m_properties->deformAmount));
            }
            dynamic_cast<DeformScale*>(m_deformAction)->setFactor(factor);
            break;
        }
        case 3:
        case 4:
        {
             // CW or CCW, the sign decide
            qreal sign = (m_properties->action == 3) ? 1.0 : -1.0;
            qreal factor;
            if (m_properties->useCounter){
                factor = m_counter * sign * degToRad;
            } else{
                factor =  (360 * m_properties->deformAmount * 0.5) * sign * degToRad;
            }
            dynamic_cast<DeformRotation*>(m_deformAction)->setAlpha(factor);
            break;
        }
        case 5:
        {
            if (m_firstPaint == false) {
                m_prevX = pos.x();
                m_prevY = pos.y();
                static_cast<DeformMove*>(m_deformAction)->setDistance(0.0,0.0);
                m_firstPaint = true;
                // can't paint for the first time
                return false;
            } else {
                static_cast<DeformMove*>(m_deformAction)->setDistance(pos.x() - m_prevX,pos.y() - m_prevY);
                m_prevX = pos.x();
                m_prevY = pos.y();
            }
            break;
        }
        case 6:
        case 7:
        {
            static_cast<DeformLens*>(m_deformAction)->setMaxDistance(m_sizeProperties->diameter * 0.5, m_sizeProperties->diameter * 0.5);
            break;
        }
        case 8:
        {
            // no run-time setup 
            break;
        }
        default:{
            break;
        }
    }
    return true;
}

void DeformBrush::paintMask(KisFixedPaintDeviceSP dab, KisPaintDeviceSP layer, qreal scale, qreal rotation, QPointF pos, qreal subPixelX, qreal subPixelY)
{
    KisRandomSubAccessorPixel srcAcc = layer->createRandomSubAccessor();
    m_srcAcc = &srcAcc;
    m_pixelSize = layer->colorSpace()->pixelSize();
    
    qreal fWidth = maskWidth(scale);
    qreal fHeight = maskHeight(scale);
    
    int dstWidth =  qRound( m_maskRect.width() );
    int dstHeight = qRound( m_maskRect.height());
  
    // clear
    if (dab->bounds().width() != dstWidth || dab->bounds().height() != dstHeight){
        dab->setRect(m_maskRect.toRect());
        dab->initialize();
    }else{
        dab->clear(m_maskRect.toRect());
    }
    
    m_centerX = dstWidth  * 0.5  + subPixelX;
    m_centerY = dstHeight * 0.5  + subPixelY;

    quint8* dabPointer = dab->data();
    
    // major axis
    m_majorAxis = 2.0/fWidth;
    // minor axis
    m_minorAxis = 2.0/fHeight;
    // inverse square
    m_inverseScale = 1.0 / scale;
    // amount of precomputed data
    m_maskRadius = 0.5 * fWidth;

    qreal maskX;
    qreal maskY;
    qreal distance;

    // if can't paint, stop
    if (!setupAction(pos)) return;
    
    for (int y = 0; y <  dstHeight; y++){
        for (int x = 0; x < dstWidth; x++){
            maskX = x - m_centerX;
            maskY = y - m_centerY;
            distance = norme(maskX * m_majorAxis, maskY * m_minorAxis);
            if (distance > 1.0){
                // leave there OPACITY TRANSPARENT pixel (default pixel)
                dabPointer += m_pixelSize;
                continue;
            }
            
            m_deformAction->transform( &maskX, &maskY, distance);
            
            maskX += pos.x();
            maskY += pos.y();
            
            movePixel(maskX, maskY, dabPointer);    
            dabPointer += m_pixelSize;
        }
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


QPointF DeformBrush::hotSpot(qreal scale, qreal rotation)
{
    qreal fWidth = maskWidth(scale);
    qreal fHeight = maskHeight(scale);

    QTransform m;
    m.reset();
    m.rotateRadians(rotation);

    m_maskRect = QRect(0,0,fWidth,fHeight);
    m_maskRect.translate(-m_maskRect.center());
    m_maskRect = m.mapRect(m_maskRect);
    m_maskRect.translate(-m_maskRect.topLeft());
    return m_maskRect.center();
}


