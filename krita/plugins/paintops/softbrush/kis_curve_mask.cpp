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



void KisCurveMask::mask(KisFixedPaintDeviceSP dab, const KoColor color, qreal scale, qreal rotation, qreal subPixelX, qreal subPixelY)
{
    Q_UNUSED(rotation);
    KoColor dabColor(color);
    
    int dstWidth = maskWidth(scale);
    int dstHeight = maskHeight(scale);

    QRect maskRect = QRect(0,0,dstWidth,dstHeight);
    int w = dab->bounds().width();
    int h = dab->bounds().height();
   
    quint32 pixelSize = dab->colorSpace()->pixelSize();
    
    // clear
    if (w!=dstWidth || h!=dstHeight){
        dab->setRect(maskRect);
        dab->initialize();
    }else{
        dab->clear(maskRect);
    }
    
    qreal centerX = dstWidth  * 0.5 - 1.0 + subPixelX;
    qreal centerY = dstHeight * 0.5 - 1.0 + subPixelY;

    quint8* dabPointer = dab->data();
    
    // major axis
    m_majorAxis = 2.0/dstWidth;
    // minor axis
    m_minorAxis = 2.0/dstHeight;
    // inverse square
    m_inverseScale = 1.0 / scale;
    // amount of precomputed data
    m_maskRadius = 0.5 * dstWidth;
    
    for (int y = 0; y <  dstHeight; y++){
        for (int x = 0; x < dstWidth; x++){
            double maskX = (x - centerX);
            double maskY = (y - centerY);

            qreal alpha = valueAt(maskX, maskY);
            if (alpha != OPACITY_OPAQUE_F)
            {
                dabColor.setOpacity(alpha);
                memcpy(dabPointer,dabColor.data(), pixelSize);
            }
            dabPointer += pixelSize;
        }
    }
}

qreal KisCurveMask::valueAt(qreal x, qreal y)
{
    qreal dist = norme(x * m_majorAxis, y * m_minorAxis);
    if (dist <= 1.0){
        qreal distance = dist * m_maskRadius;
        distance *= m_inverseScale; // apply scale
    
        quint16 alphaValue = distance;
        qreal alphaValueF = distance - alphaValue;

        if (m_properties->curveData.size() <= (alphaValue+1)){
            kDebug() << "[ " << x << ", " << y << " ] distance: " << distance << "size: " << m_properties->curveData.size();
            return OPACITY_TRANSPARENT_F;
        }
           
        qreal alpha = (
            (1.0 - alphaValueF) * m_properties->curveData.at(alphaValue) + 
                    alphaValueF * m_properties->curveData.at(alphaValue+1));
        return alpha;
    }            
    return OPACITY_TRANSPARENT_F;
}

