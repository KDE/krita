/*
 *  Copyright (c) 2004,2007-2008 Cyrille Berger <cberger@cberger.net>
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
};

KisAutoBrush::KisAutoBrush(KisMaskGenerator* as)
    : KisBrush()
    , d( new Private )
{
    d->shape = as;
    QImage img = createBrushPreview();
    setImage(img);
    setBrushType(MASK);
}

KisAutoBrush::~KisAutoBrush()
{
    delete d->shape;
    delete d;
}

void KisAutoBrush::generateMask(KisPaintDeviceSP dst,
                                KisBrush::ColoringInformation* src,
                                double scaleX, double scaleY, double angle,
                                const KisPaintInformation& info,
                                double subPixelX , double subPixelY) const
{
    Q_UNUSED(info);

    // Generate the paint device from the mask
    const KoColorSpace* cs = dst->colorSpace();
    quint32 pixelSize = cs->pixelSize();

    int dstWidth = maskWidth(scaleX, angle);
    int dstHeight = maskHeight(scaleY, angle);

    double invScaleX = 1.0 / scaleX;
    double invScaleY = 1.0 / scaleY;

    double centerX = dstWidth  * 0.5 - 1.0 + subPixelX;
    double centerY = dstHeight * 0.5 - 1.0 + subPixelY;
    double cosa = cos(angle);
    double sina = sin(angle);

    quint8* dabData = new quint8[pixelSize * dstWidth * dstHeight];
    quint8* dabPointer = dabData;
    memset(dabData, OPACITY_TRANSPARENT, pixelSize * dstWidth * dstHeight);

    quint8* color = 0;
    if (src) {
        if (dynamic_cast<PlainColoringInformation*>(src)) {
            color = const_cast<quint8*>(src->color());
        }
    }
    
    for (int y = 0; y < dstHeight; y++) {
        for (int x = 0; x < dstWidth; x++) {
            
            double x_ = (x - centerX) * invScaleX;
            double y_ = (y - centerY) * invScaleY;
            double maskX = cosa * x_ - sina * y_;
            double maskY = sina * x_ + cosa * y_;

            if (src) {
                if (color) {
                    memcpy(dabPointer, color, pixelSize);
                }
                else {
                    memcpy(dabPointer, src->color(), pixelSize);
                    src->nextColumn();
                }
            }
            cs->setAlpha(dabPointer, OPACITY_OPAQUE - d->shape->interpolatedValueAt(maskX, maskY), 1);
            dabPointer += pixelSize;
        }
        if (!color && src) {
            src->nextRow();
        }
    }

    dst->writeBytes(dabData, 0, 0, dstWidth, dstHeight);
    delete[] dabData;

}

void KisAutoBrush::toXML(QDomDocument& doc, QDomElement& e) const
{
    d->shape->toXML(doc, e);
    e.setAttribute( "brush_type", "kis_auto_brush" );
    e.setAttribute( "brush_spacing", spacing() );
}

QImage KisAutoBrush::createBrushPreview()
{
    int width = qMax((int)(d->shape->width() + 0.5), 1);
    int height = qMax((int)(d->shape->height() + 0.5), 1);
    QImage img(width, height, QImage::Format_ARGB32);

    double centerX = img.width() * 0.5;
    double centerY = img.height() * 0.5;
    for (int j = 0; j < d->shape->height(); j++) {
        for (int i = 0; i < d->shape->width(); i++) {
            qint8 v = d->shape->valueAt(i - centerX, j - centerY);
            img.setPixel(i, j, qRgb(v, v, v));
        }
    }
    return img;
}

