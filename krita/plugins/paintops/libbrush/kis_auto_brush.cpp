/*
 *  Copyright (c) 2004,2007-2009 Cyrille Berger <cberger@cberger.net>
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

#include "kis_auto_brush.h"


#include <kis_debug.h>
#include <math.h>

#include <QDomElement>

#include <KoColor.h>
#include <KoColorSpace.h>

#include "kis_datamanager.h"
#include "kis_paint_device.h"

#include "kis_mask_generator.h"

struct KisAutoBrush::Private {
    KisMaskGenerator* shape;
    qreal angle;
    qreal randomness;
    mutable QVector<quint8> precomputedQuarter;
};

KisAutoBrush::KisAutoBrush(KisMaskGenerator* as, qreal angle, qreal randomness)
        : KisBrush()
        , d(new Private)
{
    d->shape = as;
    d->angle = angle;
    d->randomness = randomness;
    QImage image = createBrushPreview();
    setImage(image);
    setBrushType(MASK);
    setWidth(d->shape->width());
    setHeight(d->shape->height());
}

KisAutoBrush::~KisAutoBrush()
{
    delete d->shape;
    delete d;
}

void KisAutoBrush::generateMaskAndApplyMaskOrCreateDab(KisFixedPaintDeviceSP dst,
        KisBrush::ColoringInformation* coloringInformation,
        double scaleX, double scaleY, double angle,
        const KisPaintInformation& info,
        double subPixelX , double subPixelY) const
{
    Q_UNUSED(info);

    // Generate the paint device from the mask
    const KoColorSpace* cs = dst->colorSpace();
    quint32 pixelSize = cs->pixelSize();

    angle += d->angle;
    
    // Make sure the angle stay in [0;2*M_PI]
    if(angle < 0) angle += 2 * M_PI;
    if(angle > 2 * M_PI) angle -= 2 * M_PI;
    
    int dstWidth = maskWidth(scaleX, angle);
    int dstHeight = maskHeight(scaleY, angle);

    // if there's coloring information, we merely change the alpha: in that case,
    // the dab should be big enough!
    if (coloringInformation) {

        // old bounds
        QRect bounds = dst->bounds();

        // new bounds. we don't care if there is some extra memory occcupied.
        dst->setRect(QRect(0, 0, dstWidth, dstHeight));

        if (dstWidth * dstHeight <= bounds.width() * bounds.height()) {
            // just clear the data in dst,
            memset(dst->data(), OPACITY_TRANSPARENT_U8, dstWidth * dstHeight * dst->pixelSize());
        } else {
            // enlarge the data
            dst->initialize();
        }
    } else {
        if (dst->data() == 0 || dst->bounds().isEmpty()) {
            qWarning() << "Creating a default black dab: no coloring info and no initialized paint device to mask";
            dst->clear(QRect(0, 0, dstWidth, dstHeight));
        }
        Q_ASSERT(dst->bounds().size().width() >= dstWidth && dst->bounds().size().height() >= dstHeight);
    }

    quint8* dabPointer = dst->data();

    quint8* color = 0;
    if (coloringInformation) {
        if (dynamic_cast<PlainColoringInformation*>(coloringInformation)) {
            color = const_cast<quint8*>(coloringInformation->color());
        }
    } else {
        // Mask everything out
        cs->setOpacity(dst->data(), OPACITY_TRANSPARENT_U8, dst->bounds().width() * dst->bounds().height());
    }

    int rowWidth = dst->bounds().width();

    double invScaleX = 1.0 / scaleX;
    double invScaleY = 1.0 / scaleY;

    double centerX = dstWidth  * 0.5 - 1.0 + subPixelX;
    double centerY = dstHeight * 0.5 - 1.0 + subPixelY;

    // the results differ, sometimes this code is faster, sometimes it is not
    // more investigation is probably needed
    // valueAt is costly similary to interpolation is some cases
    if (false && isBrushSymmetric(angle) && (dynamic_cast<PlainColoringInformation*>(coloringInformation))){
        // round eg. 14.3 to 15 so that we can interpolate
        // we have to add one pixel because of subpixel precision (see the centerX, centerY computation) 
        // and add one pixel because of interpolation
        int halfWidth = qRound((dstWidth - centerX) ) + 2;
        int halfHeight = qRound((dstHeight - centerY) ) + 2;
        
        int size = halfWidth * halfHeight;
        if (d->precomputedQuarter.size() != size)
        {
            d->precomputedQuarter.resize(size);
        }
        
        // precompute the table for interpolation 
        int pos = 0;
        for (int y = 0; y < halfHeight; y++){
            for (int x = 0; x < halfWidth; x++, pos++){
                double maskX = x * invScaleX;
                double maskY = y * invScaleY;
                d->precomputedQuarter[pos] = d->shape->valueAt(maskX, maskY);
            }
        }
        
        for (int y = 0; y < dstHeight; y++) {
            for (int x = 0; x < dstWidth; x++) {

                double maskX = (x - centerX);
                double maskY = (y - centerY);

                if (coloringInformation) {
                    if (color) {
                        memcpy(dabPointer, color, pixelSize);
                    } else {
                        memcpy(dabPointer, coloringInformation->color(), pixelSize);
                        coloringInformation->nextColumn();
                    }
                }
                double random = (1.0 - d->randomness) + d->randomness * float(rand()) / RAND_MAX;
                cs->setOpacity(dabPointer,quint8( ( OPACITY_OPAQUE_U8 - interpolatedValueAt(maskX, maskY,d->precomputedQuarter,halfWidth) ) * random), 1);
                dabPointer += pixelSize;
            }//endfor x
            //printf("\n");

            if (!color && coloringInformation) {
                coloringInformation->nextRow();
            }
            //TODO: this never happens probably? 
            if (dstWidth < rowWidth) {
                dabPointer += (pixelSize * (rowWidth - dstWidth));
            }
            
        }//endfor y
        
    }
    else
    {
        double cosa = cos(angle);
        double sina = sin(angle);
        
        for (int y = 0; y < dstHeight; y++) {
            for (int x = 0; x < dstWidth; x++) {

                double x_ = (x - centerX) * invScaleX;
                double y_ = (y - centerY) * invScaleY;
                double maskX = cosa * x_ - sina * y_;
                double maskY = sina * x_ + cosa * y_;

                if (coloringInformation) {
                    if (color) {
                        memcpy(dabPointer, color, pixelSize);
                    } else {
                        memcpy(dabPointer, coloringInformation->color(), pixelSize);
                        coloringInformation->nextColumn();
                    }
                }
                
                double random = (1.0 - d->randomness) + d->randomness * float(rand()) / RAND_MAX;
                cs->setOpacity(dabPointer, quint8( (OPACITY_OPAQUE_U8 - d->shape->valueAt(maskX, maskY)) * random), 1);
                dabPointer += pixelSize;
            }//endfor x
            if (!color && coloringInformation) {
                coloringInformation->nextRow();
            }
            //TODO: this never happens probably? 
            if (dstWidth < rowWidth) {
                dabPointer += (pixelSize * (rowWidth - dstWidth));
            }
        }//endfor y
    }//else 
}


void KisAutoBrush::toXML(QDomDocument& doc, QDomElement& e) const
{
    QDomElement shapeElt = doc.createElement("MaskGenerator");
    d->shape->toXML(doc, shapeElt);
    e.appendChild(shapeElt);
    e.setAttribute("type", "auto_brush");
    e.setAttribute("spacing", spacing());
    e.setAttribute("angle", d->angle);
    e.setAttribute("randomness", d->randomness);
}

QImage KisAutoBrush::createBrushPreview()
{
    int width = qMax((int)(d->shape->width() + 0.5), 1);
    int height = qMax((int)(d->shape->height() + 0.5), 1);
    QImage image(width, height, QImage::Format_ARGB32);

    double centerX = image.width() * 0.5;
    double centerY = image.height() * 0.5;
    for (int j = 0; j < height; ++j) {
        QRgb *pixel = reinterpret_cast<QRgb *>(image.scanLine(j));
        for (int i = 0; i < width; ++i) {
            double random = (1.0 - d->randomness) + d->randomness * float(rand()) / RAND_MAX;
            qint8 v = 255 - (255 - d->shape->valueAt(i - centerX, j - centerY)) * random;
            pixel[i] = qRgb(v, v, v);
        }
    }
    return image.transformed(QTransform().rotate(-d->angle * 180 / M_PI));
}

QPointF KisAutoBrush::hotSpot(double scaleX, double scaleY, double rotation) const
{
    return KisBrush::hotSpot(scaleX, scaleY, rotation + d->angle);
}

const KisMaskGenerator* KisAutoBrush::maskGenerator() const
{
    return d->shape;
}

qreal KisAutoBrush::angle() const
{
    return d->angle;
}


bool KisAutoBrush::isBrushSymmetric(double angle) const
{       
    // small brushes compute directly   
    if (d->shape->height() < 3 ) return false;          
    // even spikes are symmetric        
    if ((d->shape->spikes() % 2) != 0) return false;    
    // main condition, if not rotated or use optimization for rotated circles - rotated circle is circle again          
    if ( angle == 0.0 || ( ( d->shape->type() == KisMaskGenerator::CIRCLE ) && ( d->shape->width() == d->shape->height() ) ) ) return true;     
    // in other case return false       
    return false;
}


quint8 KisAutoBrush::interpolatedValueAt(double x, double y,const QVector<quint8> &precomputedQuarter,int width) const 
{
    x = qAbs(x);
    y = qAbs(y);
    
    double x_i = floor(x);         
    double x_f = x - x_i;       
    double x_f_r = 1.0 - x_f;   

    double y_i = floor(y);      
    double y_f = fabs(y - y_i);         
    double y_f_r = 1.0 - y_f;   
    
    return (x_f_r * y_f_r * valueAt(x_i , y_i, precomputedQuarter, width) +        
            x_f   * y_f_r * valueAt(x_i + 1, y_i, precomputedQuarter, width) +     
            x_f_r * y_f   * valueAt(x_i,  y_i + 1, precomputedQuarter, width) +    
            x_f   * y_f   * valueAt(x_i + 1,  y_i + 1, precomputedQuarter, width));
}


