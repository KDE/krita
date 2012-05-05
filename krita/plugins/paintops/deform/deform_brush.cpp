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
#include <kis_iterator_ng.h>

#include <cmath>
#include <ctime>
#include <KoColorSpaceRegistry.h>

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
    // here there was a switch to select between new and old sampled data, but the
    // sub accessor always used a const accessor, which means that there was no
    // difference.
    m_srcAcc->sampledOldRawData(dst);

}

void DeformBrush::oldDeform(KisPaintDeviceSP dab,KisPaintDeviceSP layer,QPointF pos)
{
    m_srcAcc = layer->createRandomSubAccessor();
    m_pixelSize = layer->pixelSize();

    if (!setupAction(DeformModes(m_properties->action-1),pos)){ return; }

    int curXi = static_cast<int>(pos.x() + 0.5);
    int curYi = static_cast<int>(pos.y() + 0.5);

    qreal maskX, maskY;
    qreal distance;

    int radius = m_sizeProperties->diameter * 0.5;
    int left = curXi - radius;
    int top = curYi - radius;
    int w = radius * 2 + 1;
    int h = w;
    qreal m_majorAxis = 2.0/radius;
    qreal m_minorAxis = 2.0/radius;

    KisRectIteratorSP m_srcIt = dab->createRectIteratorNG(left, top, w , h);

    do {
        maskX = m_srcIt->x() - curXi;
        maskY = m_srcIt->y() - curYi;

        distance = norme(maskX * m_majorAxis, maskY * m_minorAxis);
        if (distance > 1.0){ continue; }

        m_deformAction->transform( &maskX, &maskY, distance);

        maskX += curXi;
        maskY += curYi;

        movePixel(maskX, maskY, m_srcIt->rawData());
    } while (m_srcIt->nextPixel());

    m_counter++;
}


void DeformBrush::initDeformAction()
{
    DeformModes mode = DeformModes(m_properties->action-1);
    
    switch(mode){
        case GROW:
        case SHRINK:
        {
            m_deformAction = new DeformScale();
            break;
        }
        case SWIRL_CW:
        case SWIRL_CCW:
        {
            m_deformAction = new DeformRotation();
            break;
        }

        case MOVE:
        {
            m_deformAction = new DeformMove();
            static_cast<DeformMove*>(m_deformAction)->setFactor(m_properties->deformAmount);
            break;
        }
        case LENS_IN:
        case LENS_OUT:
        {
            m_deformAction = new DeformLens();
            static_cast<DeformLens*>(m_deformAction)->setLensFactor(m_properties->deformAmount,0.0);
            static_cast<DeformLens*>(m_deformAction)->setMode(mode == LENS_OUT);
            break;
        }
        case DEFORM_COLOR:
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

bool DeformBrush::setupAction(DeformModes mode,const QPointF& pos)
{

    switch(mode){
        case GROW:
        case SHRINK:
        {
            // grow or shrink, the sign decide
            qreal sign = (mode == GROW) ? 1.0 : -1.0;
            qreal factor;
            if (m_properties->useCounter){
                factor = (1.0 + sign*(m_counter*m_counter / 100.0));
            } else{
                factor =  (1.0 + sign*(m_properties->deformAmount));
            }
            dynamic_cast<DeformScale*>(m_deformAction)->setFactor(factor);
            break;
        }
        case SWIRL_CW:
        case SWIRL_CCW:
        {
             // CW or CCW, the sign decide
            qreal sign = (mode == SWIRL_CW) ? 1.0 : -1.0;
            qreal factor;
            if (m_properties->useCounter){
                factor = m_counter * sign * degToRad;
            } else{
                factor =  (360 * m_properties->deformAmount * 0.5) * sign * degToRad;
            }
            dynamic_cast<DeformRotation*>(m_deformAction)->setAlpha(factor);
            break;
        }
        case MOVE:
        {
            if (m_firstPaint == false) {
                m_prevX = pos.x();
                m_prevY = pos.y();
                static_cast<DeformMove*>(m_deformAction)->setDistance(0.0,0.0);
                m_firstPaint = true;
                return false;
            } else {
                static_cast<DeformMove*>(m_deformAction)->setDistance(pos.x() - m_prevX,pos.y() - m_prevY);
                m_prevX = pos.x();
                m_prevY = pos.y();
            }
            break;
        }
        case LENS_IN:
        case LENS_OUT:
        {
            static_cast<DeformLens*>(m_deformAction)->setMaxDistance(m_sizeProperties->diameter * 0.5, m_sizeProperties->diameter * 0.5);
            break;
        }
        case DEFORM_COLOR:
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

KisFixedPaintDeviceSP DeformBrush::paintMask(KisFixedPaintDeviceSP dab, 
                                             KisPaintDeviceSP layer, 
                                             qreal scale, 
                                             qreal rotation, 
                                             QPointF pos, qreal subPixelX, qreal subPixelY, int dabX, int dabY)
{
    KisFixedPaintDeviceSP mask = new KisFixedPaintDevice(KoColorSpaceRegistry::instance()->alpha8());
    m_srcAcc = layer->createRandomSubAccessor();

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
    if (!setupAction(DeformModes(m_properties->action-1),pos)) {
        return 0;
    }

    qreal cosa = cos(-rotation);
    qreal sina = sin(-rotation);

    qreal bcosa = cos(rotation);
    qreal bsina = sin(rotation);

    
    mask->setRect(dab->bounds());
    mask->initialize();
    quint8* maskPointer = mask->data();
    qint8 maskPixelSize = mask->pixelSize();
    KoColor pixel(dab->colorSpace());
    
    for (int y = 0; y <  dstHeight; y++){
        for (int x = 0; x < dstWidth; x++){
            maskX = x - m_centerX;
            maskY = y - m_centerY;
            qreal rmaskX = cosa * maskX - sina * maskY;
            qreal rmaskY = sina * maskX + cosa * maskY;


            distance = norme(rmaskX * m_majorAxis, rmaskY * m_minorAxis);
            if (distance > 1.0){
                // leave there OPACITY TRANSPARENT pixel (default pixel)
                m_srcAcc->moveTo(x + dabX, y + dabY);
                m_srcAcc->sampledOldRawData(dabPointer);
                dabPointer += m_pixelSize;

                *maskPointer = OPACITY_TRANSPARENT_U8;
                maskPointer += maskPixelSize;
                continue;
            }

            if (m_sizeProperties->density != 1.0){
                if (m_sizeProperties->density < drand48()){
                    dabPointer += m_pixelSize;
                    *maskPointer = OPACITY_TRANSPARENT_U8;
                    maskPointer += maskPixelSize;
                    continue;
                }
            }

            m_deformAction->transform( &rmaskX, &rmaskY, distance);

            maskX = bcosa * rmaskX - bsina * rmaskY;
            maskY = bsina * rmaskX + bcosa * rmaskY;

            maskX += pos.x();
            maskY += pos.y();

            movePixel(maskX, maskY, dabPointer);
            dabPointer += m_pixelSize;
            
            *maskPointer = OPACITY_OPAQUE_U8;
            maskPointer += maskPixelSize;
            
        }
    }
    m_counter++;

    return mask;

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


/***
* methods with fast prefix (like this one called fastScale) uses KisRectIterator,
* they are faster just a little bit (according my tests 120 miliseconds faster with big radius, slower with small radius)
**/
