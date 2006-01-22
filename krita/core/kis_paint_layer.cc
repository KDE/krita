/*
 *  Copyright (c) 2005 Casper Boemann <cbr@boemann.dk>
 *
 *  this program is free software; you can redistribute it and/or modify
 *  it under the terms of the gnu general public license as published by
 *  the free software foundation; either version 2 of the license, or
 *  (at your option) any later version.
 *
 *  this program is distributed in the hope that it will be useful,
 *  but without any warranty; without even the implied warranty of
 *  merchantability or fitness for a particular purpose.  see the
 *  gnu general public license for more details.
 *
 *  you should have received a copy of the gnu general public license
 *  along with this program; if not, write to the free software
 *  foundation, inc., 675 mass ave, cambridge, ma 02139, usa.
 */

#include <kdebug.h>
#include <qimage.h>

#include "kis_debug_areas.h"
#include "kis_image.h"
#include "kis_paint_layer.h"
#include "kis_selection.h"
#include "kis_painter.h"
#include "kis_undo_adapter.h"
#include "kis_iterators_pixel.h"
#include "kis_paint_device_impl.h"


KisPaintLayer::KisPaintLayer(KisImage *img, const QString& name, Q_UINT8 opacity, KisPaintDeviceImplSP dev)
    : super(img, name, opacity)
{
    m_paintdev = dev;
}


KisPaintLayer::KisPaintLayer(KisImage *img, const QString& name, Q_UINT8 opacity)
    : super(img, name, opacity)
{
    m_paintdev = new KisPaintDeviceImpl(img, img -> colorSpace());
}

KisPaintLayer::KisPaintLayer(KisImage *img, const QString& name, Q_UINT8 opacity, KisColorSpace * colorSpace)
    : super(img, name, opacity)
{
    m_paintdev = new KisPaintDeviceImpl(img, colorSpace);
}

KisPaintLayer::KisPaintLayer(const KisPaintLayer& rhs) : KisLayer(rhs)
{
    m_paintdev = new KisPaintDeviceImpl( *rhs.m_paintdev.data() );
}

KisLayerSP KisPaintLayer::clone() const
{
    return new KisPaintLayer(*this);
}

KisPaintLayer::~KisPaintLayer()
{
}

void KisPaintLayer::paintSelection(QImage &img, Q_INT32 x, Q_INT32 y, Q_INT32 w, Q_INT32 h)
{
    if (m_paintdev -> hasSelection())
            m_paintdev->selection()->paintSelection(img, x, y, w, h);
}

void KisPaintLayer::paintMaskInactiveLayers(QImage &img, Q_INT32 x, Q_INT32 y, Q_INT32 w, Q_INT32 h)
{
    uchar *j = img.bits();

    KisColorSpace *cs = m_paintdev->colorSpace();

    for (Q_INT32 y2 = y; y2 < h + y; ++y2) {
        KisHLineIteratorPixel it = m_paintdev->createHLineIterator(x, y2, w, false);
        while ( ! it.isDone()) {
            Q_UINT8 s = cs->getAlpha(it.rawData());
            if(s==0)
            {
                Q_UINT8 g = (*(j + 0)  + *(j + 1 ) + *(j + 2 )) / 9;

                *(j+0) = 128+g ;
                *(j+1) = 165+g;
                *(j+2) = 128+g;
            }
            j+=4;
            ++it;
        }
    }
}

QImage KisPaintLayer::createThumbnail(Q_INT32 w, Q_INT32 h)
{
    if (w > image()->width())
    {
        w = image()->width();
        h = Q_INT32(double(image()->width()) / w * h);
    }
    if (h > image()->height())
    {
        h = image()->height();
        w = Q_INT32(double(image()->height()) / h * w);
    }

    if (image()->width() > image()->height())
        h = Q_INT32(double(image()->height()) / image()->width() * w);
    else if (image()->height() > image()->width())
        w = Q_INT32(double(image()->width()) / image()->height() * h);

    QColor c;
    Q_UINT8 opacity;
    QImage img(w,h,32);

    for (Q_INT32 y=0; y < h; ++y) {
        Q_INT32 iY = (y * image()->height() ) / h;
        for (Q_INT32 x=0; x < w; ++x) {
            Q_INT32 iX = (x * image()->width() ) / w;
            paintDevice()->pixel(iX, iY, &c, &opacity);
            const QRgb rgb = c.rgb();
            img.setPixel(x, y, qRgba(qRed(rgb), qGreen(rgb), qBlue(rgb), opacity));
        }
    }

    return img;
}

void KisPaintLayer::setImage(KisImage *image)
{
    super::setImage(image);
    m_paintdev->setImage(image);
}

#include "kis_paint_layer.moc"
