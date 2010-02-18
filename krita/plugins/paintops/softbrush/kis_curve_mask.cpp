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



void KisCurveMask::mask(KisFixedPaintDeviceSP dab, const KoColor color, qreal /*scale*/, qreal /*rotation*/, qreal /*subPixelX*/, qreal /*subPixelY*/)
{
    KoColor dabColor(color);
    
    int maskWidth = m_properties->diameter;
    int maskHeight = m_properties->diameter;

    QRect maskRect = QRect(0,0,maskWidth,maskHeight);
    int w = dab->bounds().width();
    int h = dab->bounds().height();
   
    quint32 pixelSize = dab->colorSpace()->pixelSize();
    
    // clear
    if (w!=maskWidth || h!=maskHeight){
        dab->setRect(maskRect);
        dab->initialize();
    }else{
        dab->clear(maskRect);
    }
    
    qreal centerX = maskWidth  * 0.5;// - 1.0 + subPixelX;
    qreal centerY = maskHeight * 0.5;// - 1.0 + subPixelY;

    quint8* dabPointer = dab->data();
    
//     double invScaleX = 1.0 / scale;
//     double invScaleY = 1.0 / scale;

    // maximal distnace in the circle is radius^2
    qreal border = (maskWidth * 0.5) * (maskWidth * 0.5);
    
    KoColor red(dab->colorSpace());
    red.fromQColor(Qt::red);
    
    for (int y = 0; y <  maskHeight; y++){
        for (int x = 0; x < maskWidth; x++){
            double maskX = (x - centerX);// * invScaleX;
            double maskY = (y - centerY);// * invScaleY;
            
            qreal dist = maskX*maskX + maskY*maskY;
            if (dist <= border){
                
                qreal distance = sqrt(dist);
                quint16 alphaValue = distance;
                qreal alphaValueF = distance - alphaValue;

                if (m_properties->curveData.size() <= (alphaValue+1)){
                    qDebug() << "[ " << maskX << ", " << maskY << " ] distance: " << distance << "size: " << m_properties->curveData.size();;
                    continue;
                }
           
                qreal alpha = (
                (1.0 - alphaValueF) * m_properties->curveData.at(alphaValue) + 
                    alphaValueF * m_properties->curveData.at(alphaValue+1)); 
            
                dabColor.setOpacity(alpha);
                memcpy(dabPointer,dabColor.data(), pixelSize);
                
            }
            dabPointer += pixelSize;
        }
    }
}


