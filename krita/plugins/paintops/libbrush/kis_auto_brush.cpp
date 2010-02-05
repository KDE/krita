/*
 *  Copyright (c) 2004,2007-2009 Cyrille Berger <cberger@cberger.net>
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
};

KisAutoBrush::KisAutoBrush(KisMaskGenerator* as, double angle)
        : KisBrush()
        , d(new Private)
{
    d->shape = as;
    d->angle = angle;
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
            memset(dst->data(), OPACITY_TRANSPARENT, dstWidth * dstHeight * dst->pixelSize());
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
        cs->setAlpha(dst->data(), OPACITY_TRANSPARENT, dst->bounds().width() * dst->bounds().height());
    }

    int rowWidth = dst->bounds().width();

    double invScaleX = 1.0 / scaleX;
    double invScaleY = 1.0 / scaleY;

    double centerX = dstWidth  * 0.5 - 1.0 + subPixelX;
    double centerY = dstHeight * 0.5 - 1.0 + subPixelY;

    // check geometrical and color symmetricity
    if (isBrushSymmetric(angle) && (dynamic_cast<PlainColoringInformation*>(coloringInformation))) 
    {
        int maskCenterX = qRound(dstWidth * 0.5);
        int maskCenterY = qRound(dstHeight * 0.5);
        
        // dabPointer is topLeft 
        int dstRow = dstWidth * pixelSize;
        int quarterRow = (maskCenterX) * pixelSize;

        quint8 * topRightPointer = dst->data();

        topRightPointer += dstRow - pixelSize; 

        // compute 1/4 of the mask 
        for (int y = 0; y < maskCenterY;y++){
            for (int x = 0; x < maskCenterX;x++){
                // we don't rotate here compared to version below
                double maskX = (x - centerX) * invScaleX;
                double maskY = (y - centerY) * invScaleY;

                if (coloringInformation) {
                    if (color) {
                        memcpy(dabPointer, color, pixelSize);
                    } else {
                        memcpy(dabPointer, coloringInformation->color(), pixelSize);
                        coloringInformation->nextColumn();
                    }
                }
                cs->setAlpha(dabPointer, OPACITY_OPAQUE - d->shape->valueAt(maskX, maskY) , 1);
                memcpy(topRightPointer, dabPointer, pixelSize);
            
                // topLeft goes to next right pixel
                dabPointer += pixelSize;
                // one pixel left
                topRightPointer -= pixelSize;
            }
            if (!color && coloringInformation) {
                coloringInformation->nextRow();
            }

            // put the pointer back at the start of the row and next row
            dabPointer = (dabPointer - quarterRow ) + dstRow;
            // put the pointer back at the end of the row and next row
            topRightPointer = (topRightPointer + quarterRow ) + dstRow;
            //TODO: this never happens probably? 
            /*            if (dstWidth < rowWidth) {
                dabPointer += (pixelSize * (rowWidth - dstWidth));
            }*/
        }
        
        // copy the upper half of the mask 
        // if the height is even copy every row, if not, skip the one in the middle
        if ((dstHeight % 2) == 0)
        {        
            topRightPointer = topRightPointer - dstRow + pixelSize - dstRow;
            // copy the upper part of the mask
            for (int i = 0; i < maskCenterY ;i++){
                memcpy(dabPointer, topRightPointer , dstRow);
                topRightPointer -= dstRow;
                dabPointer += dstRow;
            }
        }
        else
        {
            topRightPointer = topRightPointer - dstRow + pixelSize - dstRow - dstRow;
            for (int i = 1; i < maskCenterY ;i++){
                memcpy(dabPointer, topRightPointer , dstRow);
                topRightPointer -= dstRow;
                dabPointer += dstRow;
            }
        }
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
                    cs->setAlpha(dabPointer, OPACITY_OPAQUE - d->shape->valueAt(maskX, maskY), 1);
                    dabPointer += pixelSize;
                }
                if (!color && coloringInformation) {
                    coloringInformation->nextRow();
                }
                //TODO: this never happens probably? 
                if (dstWidth < rowWidth) {
                    dabPointer += (pixelSize * (rowWidth - dstWidth));
                }
            }
        
    }//else rotation involved
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


void KisAutoBrush::toXML(QDomDocument& doc, QDomElement& e) const
{
    d->shape->toXML(doc, e);
    e.setAttribute("brush_type", "kis_auto_brush");
    e.setAttribute("brush_spacing", spacing());
    e.setAttribute("brush_angle", d->angle);
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
            qint8 v = d->shape->valueAt(i - centerX, j - centerY);
            pixel[i] = qRgb(v, v, v);
        }
    }
    return image.transformed(QMatrix().rotate(-d->angle * 180 / M_PI));
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
