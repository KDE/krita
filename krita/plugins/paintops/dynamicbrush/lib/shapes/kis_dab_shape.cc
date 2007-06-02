/*
 *  Copyright (c) 2006-2007 Cyrille Berger <cberger@cberger.net>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "kis_dab_shape.h"

#include <kis_autobrush_resource.h>
#include <kis_iterators_pixel.h>
#include <kis_paint_device.h>
#include <kis_qimage_mask.h>

#include "kis_dynamic_coloring.h"

quint8 KisAlphaMaskShape::alphaAt(int x, int y)
{
    return alphaMask->alphaAt(x,y);
}

quint8 KisAutoMaskShape::alphaAt(int x, int y)
{
    return 255 - m_shape->valueAt(x,y);
}

KisAutoMaskShape::~KisAutoMaskShape() { if(m_shape) delete m_shape; }

KisDabShape::~KisDabShape() { }
KisDabShape::KisDabShape() { }


void KisAlphaMaskShape::resize(double xs, double ys)
{
    Q_UNUSED(xs);
    Q_UNUSED(ys);
    // TODO: implement it
}

KisDynamicShape* KisAutoMaskShape::clone() const
{
    return new KisAutoMaskShape(*this);
}

void KisAutoMaskShape::resize(double xs, double ys)
{
    autoDab.width *= (int)(2 * xs);
    autoDab.hfade *= (int)(2 * xs);
    autoDab.height *= (int)(2 * ys);
    autoDab.vfade *= (int)(2 * ys);
}

void KisAutoMaskShape::createStamp(KisPaintDeviceSP stamp, KisDynamicColoring* coloringsrc,const QPointF &/*pos*/, const KisPaintInformation& /*info*/)
{
    // Transform into the paintdevice to apply
    switch(autoDab.shape)
    {
        case KisAutoDab::ShapeCircle:
            m_shape = new KisAutobrushCircleShape(autoDab.width, autoDab.height, autoDab.hfade, autoDab.vfade);
            break;
        case KisAutoDab::ShapeRectangle:
            m_shape = new KisAutobrushRectShape(autoDab.width, autoDab.height, autoDab.hfade, autoDab.vfade);
            break;
    }

    // Apply the coloring
    KoColorSpace * colorSpace = stamp->colorSpace();

    // Convert the kiscolor to the right colorspace.
    KoColor kc;
    qint32 pixelSize = colorSpace->pixelSize();

    KisHLineIteratorPixel hiter = stamp->createHLineIterator( -autoDab.width/2, -autoDab.height/2, autoDab.width); // hum cheating (can't remember what?)
    for (int y = 0; y < autoDab.height; y++) // hum cheating (once again) /me slaps himself (again, what ?)
    {
        int x=0;
        while(! hiter.isDone())
        {
            coloringsrc->colorAt(x,y, &kc);
            colorSpace->setAlpha(kc.data(), alphaAt(x, y), 1);
            memcpy(hiter.rawData(), kc.data(), pixelSize);
            x++;
            ++hiter;
        }
        hiter.nextRow();
    }
}
