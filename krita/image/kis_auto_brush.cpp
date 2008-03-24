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

QImage KisAutoBrush::createBrushPreview()
{
    QImage img((int)(d->shape->width() + 0.5), (int)( d->shape->height() + 0.5), QImage::Format_ARGB32);
    double centerX = img.width() * 0.5;
    double centerY = img.height() * 0.5;
    for(int j = 0; j < d->shape->height(); j++)
    {
        for(int i = 0; i < d->shape->width(); i++)
        {
            qint8 v = d->shape->valueAt( i - centerX, j - centerY);
            img.setPixel( i, j, qRgb(v,v,v));
        }
    }
    return img;
}

KisAutoBrush::KisAutoBrush(KisMaskGenerator* as) : KisBrush(""), d(new Private)
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

void KisAutoBrush::generateMask(KisPaintDeviceSP dst, KisBrush::ColoringInformation* src, double scaleX, double scaleY, double angle, const KisPaintInformation& info, double subPixelX , double subPixelY ) const
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
    // Apply the alpha mask
    
    KisHLineIteratorPixel hiter = dst->createHLineIterator(0, 0, dstWidth);
    for (int y = 0; y < dstHeight; y++)
    {
        while(! hiter.isDone())
        {
            double x_ = ( hiter.x() - centerX) * invScaleX;
            double y_ = ( hiter.y() - centerY) * invScaleY ;
            double x = cosa * x_ - sina * y_;
            double y = sina * x_ + cosa * y_;
            if(src)
            {
                memcpy( hiter.rawData(), src->color(), pixelSize);
                src->nextColumn();
            }
            cs->setAlpha( hiter.rawData(),
                          OPACITY_OPAQUE - d->shape->interpolatedValueAt(x, y), 1 );
            ++hiter;
        }
        if(src) src->nextRow();
        hiter.nextRow();
    }


}

void KisAutoBrush::toXML(QDomDocument& doc, QDomElement& e) const
{
    KisBrush::toXML(doc,e);
    e.setAttribute( "type", "autobrush" );
    d->shape->toXML(doc, e);
}
