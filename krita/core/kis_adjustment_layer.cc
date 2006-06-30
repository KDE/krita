/*
 *  Copyright (c) 2002 Patrick Julien <freak@codepimps.org>
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
#include "kis_group_layer.h"
#include "kis_image.h"
#include "kis_layer.h"
#include "kis_adjustment_layer.h"
#include "kis_painter.h"
#include "kis_undo_adapter.h"
#include "kis_selection.h"
#include "kis_fill_painter.h"

KisAdjustmentLayer::KisAdjustmentLayer(KisImageSP img, const QString &name, KisFilterConfiguration * kfc, KisSelectionSP selection)
    : KisLayer (img, name, OPACITY_OPAQUE)
{
    m_filterConfig = kfc;
    setSelection( selection );
    m_cachedPaintDev = new KisPaintDevice( img->colorSpace(), name.latin1());
    m_showSelection = true;
    Q_ASSERT(m_cachedPaintDev);
}

KisAdjustmentLayer::KisAdjustmentLayer(const KisAdjustmentLayer& rhs)
    : KisLayer(rhs)
{
    m_filterConfig = new KisFilterConfiguration(*rhs.m_filterConfig);
    if (rhs.m_selection) {
        m_selection = new KisSelection( *rhs.m_selection.data() );
        m_selection->setParentLayer(this);
    }
    m_cachedPaintDev = new KisPaintDevice( *rhs.m_cachedPaintDev.data() );
    m_showSelection = false;
}


KisAdjustmentLayer::~KisAdjustmentLayer()
{
    delete m_filterConfig;
}


KisLayerSP KisAdjustmentLayer::clone() const
{
    return new KisAdjustmentLayer(*this);
}


void KisAdjustmentLayer::resetCache()
{
    m_cachedPaintDev = new KisPaintDevice(image()->colorSpace(), name().latin1());
}

KisFilterConfiguration * KisAdjustmentLayer::filter()
{
    Q_ASSERT(m_filterConfig);
    return m_filterConfig;
}


void KisAdjustmentLayer::setFilter(KisFilterConfiguration * filterConfig)
{
    Q_ASSERT(filterConfig);
    m_filterConfig = filterConfig;
}


KisSelectionSP KisAdjustmentLayer::selection()
{
    return m_selection;
}

void KisAdjustmentLayer::setSelection(KisSelectionSP selection)
{
    m_selection = new KisSelection();
    KisFillPainter gc(m_selection.data());
    KisColorSpace * cs = KisMetaRegistry::instance()->csRegistry()->getRGB8();

    if (selection) {
        gc.bitBlt(0, 0, COMPOSITE_COPY, selection.data(),
                  0, 0, image()->bounds().width(), image()->bounds().height());
    } else {
        gc.fillRect(image()->bounds(), KisColor(Qt::white, cs), MAX_SELECTED);
    }

    gc.end();

    m_selection->setParentLayer(this);
}


Q_INT32 KisAdjustmentLayer::x() const
{
    if (m_selection)
        return m_selection->getX();
    else
        return 0;
}

void KisAdjustmentLayer::setX(Q_INT32 x)
{
    if (m_selection) {
        m_selection->setX(x);
        resetCache();
    }

}

Q_INT32 KisAdjustmentLayer::y() const
{
    if (m_selection)
        return m_selection->getY();
    else
        return 0;
}

void KisAdjustmentLayer::setY(Q_INT32 y)
{
    if (m_selection) {
        m_selection->setY(y);
        resetCache();
    }
}

QRect KisAdjustmentLayer::extent() const
{
    if (m_selection)
        return m_selection->extent();
    else if (image())
        return image()->bounds();
    else
        return QRect();
}

QRect KisAdjustmentLayer::exactBounds() const
{
    if (m_selection)
        return m_selection->exactBounds();
    else if (image())
        return image()->bounds();
    else
        return QRect();
}

bool KisAdjustmentLayer::accept(KisLayerVisitor & v)
{
    return v.visit( this );
}

void KisAdjustmentLayer::paintSelection(QImage &img, Q_INT32 x, Q_INT32 y, Q_INT32 w, Q_INT32 h)
{
    if (showSelection() && selection())
        selection()->paintSelection(img, x, y, w, h);
}

void KisAdjustmentLayer::paintSelection(QImage &img, const QRect& scaledImageRect, const QSize& scaledImageSize, const QSize& imageSize)
{
    if (showSelection() && selection())
        selection()->paintSelection(img, scaledImageRect, scaledImageSize, imageSize);
}

QImage KisAdjustmentLayer::createThumbnail(Q_INT32 w, Q_INT32 h)
{
    if (!selection())
        return QImage();

    int srcw, srch;
    if( image() )
    {
        srcw = image()->width();
        srch = image()->height();
    }
    else
    {
        const QRect e = extent();
        srcw = e.width();
        srch = e.height();
    }

    if (w > srcw)
    {
        w = srcw;
        h = Q_INT32(double(srcw) / w * h);
    }
    if (h > srch)
    {
        h = srch;
        w = Q_INT32(double(srch) / h * w);
    }

    if (srcw > srch)
        h = Q_INT32(double(srch) / srcw * w);
    else if (srch > srcw)
        w = Q_INT32(double(srcw) / srch * h);

    QColor c;
    Q_UINT8 opacity;
    QImage img(w,h,32);

    for (Q_INT32 y=0; y < h; ++y) {
        Q_INT32 iY = (y * srch ) / h;
        for (Q_INT32 x=0; x < w; ++x) {
            Q_INT32 iX = (x * srcw ) / w;
            m_selection->pixel(iX, iY, &c, &opacity);
            img.setPixel(x, y, qRgb(opacity, opacity, opacity));
        }
    }

    return img;
}

#include "kis_adjustment_layer.moc"
