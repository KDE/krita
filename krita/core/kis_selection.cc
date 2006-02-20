/*
 *  Copyright (c) 2004 Boudewijn Rempt <boud@valdyas.org>
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

#include <qimage.h>

#include <kdebug.h>
#include <klocale.h>
#include <qcolor.h>

#include "kis_layer.h"
#include "kis_debug_areas.h"
#include "kis_types.h"
#include "kis_colorspace_factory_registry.h"
#include "kis_fill_painter.h"
#include "kis_iterators_pixel.h"
#include "kis_integer_maths.h"
#include "kis_image.h"
#include "kis_datamanager.h"
#include "kis_fill_painter.h"
#include "kis_selection.h"

KisSelection::KisSelection(KisPaintDeviceSP dev)
    : super(dev->parentLayer(), KisMetaRegistry::instance()->csRegistry()->getAlpha8(), (QString("selection for ") + dev->name()).latin1())
    , m_parentPaintDevice(dev)
{
    Q_ASSERT(dev);
}

KisSelection::KisSelection()
    : super(KisMetaRegistry::instance()->csRegistry()->getAlpha8(), "anonymous selection")
    , m_parentPaintDevice(0)
{
}

KisSelection::KisSelection(const KisSelection& rhs)
    : super(rhs)
{
    m_parentPaintDevice = rhs.m_parentPaintDevice;
}

KisSelection::~KisSelection()
{
    //kdDebug() << "Deleting selection " << name() << "\n" << kdBacktrace() << "\n";
}

Q_UINT8 KisSelection::selected(Q_INT32 x, Q_INT32 y)
{
    KisHLineIteratorPixel iter = createHLineIterator(x, y, 1, false);

    Q_UINT8 *pix = iter.rawData();

    return *pix;
}

void KisSelection::setSelected(Q_INT32 x, Q_INT32 y, Q_UINT8 s)
{
    KisHLineIteratorPixel iter = createHLineIterator(x, y, 1, true);

    Q_UINT8 *pix = iter.rawData();

    *pix = s;
}

QImage KisSelection::maskImage()
{
    // If part of a KisAdjustmentLayer, there may be no parent device.
    QImage img;
    Q_INT32 x, y, w, h, y2, x2;
    if (m_parentPaintDevice) {
    
        m_parentPaintDevice -> exactBounds(x, y, w, h);
        img = QImage(w, h, 32);
    }
    else {
        x = 0;
        y = 0;
        w = image()->width();
        h = image()->width();
        img = QImage(w, h, 32);
    }
    
    for (y2 = y; y2 < h - y; ++y2) {
            KisHLineIteratorPixel it = createHLineIterator(x, y2, w, false);
            x2 = 0;
            while (!it.isDone()) {
                    Q_UINT8 s = MAX_SELECTED - *(it.rawData());
                    Q_INT32 c = qRgb(s, s, s);
                    img.setPixel(x2, y2, c);
                    ++x2;
                    ++it;
            }
    }
    return img;
}
void KisSelection::select(QRect r)
{
    KisFillPainter painter(this);
    KisColorSpace * cs = KisMetaRegistry::instance()->csRegistry()->getRGB8();
    painter.fillRect(r, KisColor(Qt::white, cs), MAX_SELECTED);
    Q_INT32 x, y, w, h;
    extent(x, y, w, h);
    kdDebug (DBG_AREA_CORE) << "Selected rect: x:" << x << ", y: " << y << ", w: " << w << ", h: " << h << "\n";
}

void KisSelection::clear(QRect r)
{
    KisFillPainter painter(this);
    KisColorSpace * cs = KisMetaRegistry::instance()->csRegistry()->getRGB8();
    painter.fillRect(r, KisColor(Qt::white, cs), MIN_SELECTED);
    //emitSelectionChanged(r);
}

void KisSelection::clear()
{
    Q_UINT8 defPixel = MIN_SELECTED;
    m_datamanager -> setDefaultPixel(&defPixel);
    m_datamanager -> clear();
    //emitSelectionChanged();
}

void KisSelection::invert()
{
    Q_INT32 x,y,w,h;

    extent(x, y, w, h);
    KisRectIterator it = createRectIterator(x, y, w, h, true);
    while ( ! it.isDone() )
    {
        // CBR this is wrong only first byte is inverted
        // BSAR: But we have always only one byte in this color model :-).
        *(it.rawData()) = MAX_SELECTED - *(it.rawData());
        ++it;
    }
    Q_UINT8 defPixel = MAX_SELECTED - *(m_datamanager -> defaultPixel());
    m_datamanager -> setDefaultPixel(&defPixel);
    //emitSelectionChanged();
}

bool KisSelection::isTotallyUnselected(QRect r)
{
    if(*(m_datamanager -> defaultPixel()) != MIN_SELECTED)
        return false;

    return ! r.intersects(extent());
}

QRect KisSelection::selectedRect()
{
    if(*(m_datamanager -> defaultPixel()) == MIN_SELECTED || !m_parentPaintDevice)
        return extent();
    else 
        return extent().unite(m_parentPaintDevice->extent());
}

QRect KisSelection::selectedExactRect()
{
    if(*(m_datamanager -> defaultPixel()) == MIN_SELECTED || !m_parentPaintDevice)
        return exactBounds();
    else
        return exactBounds().unite(m_parentPaintDevice->exactBounds());
}

void KisSelection::paintSelection(QImage img, Q_INT32 x, Q_INT32 y, Q_INT32 w, Q_INT32 h)
{
    Q_INT32 x2;
    uchar *j = img.bits();

    for (Q_INT32 y2 = y; y2 < h + y; ++y2) {
        KisHLineIteratorPixel it = createHLineIterator(x-1, y2, w+2, false);
        Q_UINT8 preS = *(it.rawData());
        ++it;
        x2 = 0;
        KisHLineIteratorPixel prevLineIt = createHLineIterator(x, y2-1, w, false);
        KisHLineIteratorPixel nextLineIt = createHLineIterator(x, y2+1, w, false);
        while (!it.isDone() && x2<w) {
            Q_UINT8 s = *(it.rawData());
            ++it; //so that it will become the posvalue
            if(s!=MAX_SELECTED)
            {
                // this is where we come if the pixels should be blue or bluish

                Q_UINT8 g = (*(j + 0)  + *(j + 1 ) + *(j + 2 )) / 9;
                Q_UINT8 alpha = *(j + 3);

                // Colour influence is proportional to alpha.
                g = UINT8_MULT(g, alpha);

                if(s==MIN_SELECTED)
                {
                    //this is where we come if the pixels should be blue (or red outline)

                    // now for a simple outline based on 4-connectivity
                    if(preS != MIN_SELECTED
                        || *(it.rawData()) != MIN_SELECTED
                        || *(prevLineIt.rawData()) != MIN_SELECTED
                        || *(nextLineIt.rawData()) != MIN_SELECTED)
                    {
                        *(j+0) = 0;
                        *(j+1) = 0;
                        *(j+2) = 255;
                    }
                    else
                    {
                        *(j+0) = 165+g;
                        *(j+1) = 128+g;
                        *(j+2) = 128+g;
                    }

                    // Stop unselected transparent areas from appearing the same
                    // as selected transparent areas.
                    *(j+3) = QMAX(alpha, 192);
                }
                else
                {
                    *(j+0) = UINT8_BLEND(*(j+0), g+165, s);
                    *(j+1) = UINT8_BLEND(*(j+1), g+128, s);
                    *(j+2) = UINT8_BLEND(*(j+2), g+128, s);
                }
            }
            j+=4;
            ++x2;
            preS=s;
            ++prevLineIt;
            ++nextLineIt;
        }
    }
}
