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
#include "kis_paint_device.h"


KisPaintLayer::KisPaintLayer(KisImage *img, const QString& name, Q_UINT8 opacity, KisPaintDeviceSP dev)
    : super(img, name, opacity)
{
    Q_ASSERT(img);
    Q_ASSERT(dev);
    m_paintdev = dev;
}


KisPaintLayer::KisPaintLayer(KisImage *img, const QString& name, Q_UINT8 opacity)
    : super(img, name, opacity)
{
    Q_ASSERT(img);
    m_paintdev = new KisPaintDevice(img, img -> colorSpace());
}

KisPaintLayer::KisPaintLayer(KisImage *img, const QString& name, Q_UINT8 opacity, KisColorSpace * colorSpace)
    : super(img, name, opacity)
{
    Q_ASSERT(img);
    Q_ASSERT(colorSpace);
    m_paintdev = new KisPaintDevice(img, colorSpace);
}

KisPaintLayer::KisPaintLayer(const KisPaintLayer& rhs) : KisLayer(rhs)
{
    m_paintdev = new KisPaintDevice( *rhs.m_paintdev.data() );
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

void KisPaintLayer::setImage(KisImage *image)
{
    super::setImage(image);
    m_paintdev->setImage(image);
}

QImage KisPaintLayer::createThumbnail(Q_INT32 w, Q_INT32 h)
{
    return m_paintdev->createThumbnail(w, h);
}

#include "kis_paint_layer.moc"
