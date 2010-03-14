/*
 *  Copyright (c) 2010 Lukáš Tvrdý <lukast.dev@gmail.com>
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
#include <cmath>

#include <KoColor.h>

#include <kis_types.h>
#include <kis_fixed_paint_device.h>

#include "kis_curve_mask.h"

KisCurveMask::KisCurveMask()
{

}


QPointF KisCurveMask::hotSpot(qreal scale, qreal rotation)
{
    m_fWidth = maskWidth(scale);
    m_fHeight = maskHeight(scale);
    
    QTransform m;
    m.reset();
    m.rotateRadians(rotation);
    
    m_maskRect = QRect(0,0,m_fWidth,m_fHeight);
    m_maskRect.translate(-m_maskRect.center());
    m_maskRect = m.mapRect(m_maskRect);
    m_maskRect.translate(-m_maskRect.topLeft());
    return m_maskRect.center();
}


void KisCurveMask::mask(KisFixedPaintDeviceSP dab, const KoColor &color, qreal scale, qreal rotation, qreal subPixelX, qreal subPixelY)
{
    qreal cosa = cos(rotation);
    qreal sina = sin(rotation);
    
    int dstWidth =  qRound( m_maskRect.width() );
    int dstHeight = qRound( m_maskRect.height());
    
    int w = dab->bounds().width();
    int h = dab->bounds().height();

    // clear
    if (w!=dstWidth || h!=dstHeight){
        dab->setRect(m_maskRect.toRect());
        dab->initialize();
    }
     
    const KoColorSpace * cs = dab->colorSpace();    
    dab->fill(0,0,dstWidth, dstHeight,color.data());
    cs->setOpacity(dab->data(), OPACITY_TRANSPARENT_U8, dstWidth * dstHeight);
    
    quint32 pixelSize = cs->pixelSize();
    
    qreal centerX = dstWidth  * 0.5 - 1.0 + subPixelX;
    qreal centerY = dstHeight * 0.5 - 1.0 + subPixelY;

    quint8* dabPointer = dab->data();

    // major axis
    m_majorAxis = 2.0/m_fWidth;
    // minor axis
    m_minorAxis = 2.0/m_fHeight;
    // inverse square of amount of data in curve
    m_inverseScale = (0.5 * m_fWidth) / scale;
    
    //srand48(12345678);
    for (int y = 0; y <  dstHeight; y++){
        for (int x = 0; x < dstWidth; x++){
            double maskX = (x - centerX);
            double maskY = (y - centerY);

            double rmaskX = cosa * maskX - sina * maskY;
            double rmaskY = sina * maskX + cosa * maskY;

            qreal alpha = valueAt(rmaskX, rmaskY);
            if (alpha != OPACITY_TRANSPARENT_F)
            {
                if (m_sizeProperties->density != 1.0){
                    if (m_sizeProperties->density >= drand48()){
                        cs->setOpacity(dabPointer,alpha,1);
                    }
                }else{
                    cs->setOpacity(dabPointer,alpha,1);
                }
                
            }
            dabPointer += pixelSize;
        }
    }
}

qreal KisCurveMask::valueAt(qreal x, qreal y)
{
    qreal dist = norme(x * m_majorAxis, y * m_minorAxis);
    if (dist <= 1.0){
        qreal distance = dist * m_inverseScale;
    
        quint16 alphaValue = distance;
        qreal alphaValueF = distance - alphaValue;
           
        qreal alpha = (
            (1.0 - alphaValueF) * m_properties->curveData.at(alphaValue) + 
                    alphaValueF * m_properties->curveData.at(alphaValue+1));
        return alpha;
    }            
    return OPACITY_TRANSPARENT_F;
}

