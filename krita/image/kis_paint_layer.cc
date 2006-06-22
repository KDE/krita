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
#include <QIcon>
#include <QImage>

#include "kis_debug_areas.h"
#include "kis_image.h"
#include "kis_paint_layer.h"
#include "kis_selection.h"
#include "kis_painter.h"
#include "kis_undo_adapter.h"
#include "kis_iterators_pixel.h"
#include "kis_paint_device.h"


KisPaintLayer::KisPaintLayer(KisImage *img, const QString& name, quint8 opacity, KisPaintDeviceSP dev)
    : super(img, name, opacity)
{
    Q_ASSERT(img);
    Q_ASSERT(dev);
    m_paintdev = dev;
    m_paintdev->setParentLayer(this);
    m_paintdev->startBackgroundFilters();
}


KisPaintLayer::KisPaintLayer(KisImage *img, const QString& name, quint8 opacity)
    : super(img, name, opacity)
{
    Q_ASSERT(img);
    m_paintdev = new KisPaintDevice(this, img->colorSpace(), name);
    m_paintdev->startBackgroundFilters();
}

KisPaintLayer::KisPaintLayer(KisImage *img, const QString& name, quint8 opacity, KoColorSpace * colorSpace)
    : super(img, name, opacity)
{
    Q_ASSERT(img);
    Q_ASSERT(colorSpace);
    m_paintdev = new KisPaintDevice(this, colorSpace, name);
    m_paintdev->startBackgroundFilters();
}

KisPaintLayer::KisPaintLayer(const KisPaintLayer& rhs) : KisLayer(rhs)
{
    m_paintdev = new KisPaintDevice( *rhs.m_paintdev.data() );
    m_paintdev->setParentLayer(this);
    m_paintdev->startBackgroundFilters();
}

QIcon KisPaintLayer::icon() const
{
    return QIcon();
}

KoDocumentSectionModel::PropertyList KisPaintLayer::properties() const
{
    PropertyList l = super::properties();
    l << Property(i18n("Colorspace"), paintDevice()->colorSpace()->id().name());
    if( KoColorProfile *profile = paintDevice()->colorSpace()->getProfile() )
        l << Property(i18n("Profile"), profile->productName());
    return l;
}

KisLayerSP KisPaintLayer::clone() const
{
    return KisLayerSP(new KisPaintLayer(*this));
}

KisPaintLayer::~KisPaintLayer()
{
    if (!m_paintdev.isNull()) {
        m_paintdev->setParentLayer(0);
    }
}

void KisPaintLayer::paintSelection(QImage &img, qint32 x, qint32 y, qint32 w, qint32 h)
{
    if (m_paintdev->hasSelection())
            m_paintdev->selection()->paintSelection(img, x, y, w, h);
}

void KisPaintLayer::paintSelection(QImage &img, const QRect& scaledImageRect, const QSize& scaledImageSize, const QSize& imageSize)
{
    if (m_paintdev && m_paintdev->hasSelection()) {
        m_paintdev->selection()->paintSelection(img, scaledImageRect, scaledImageSize, imageSize);
    }
}

void KisPaintLayer::paintMaskInactiveLayers(QImage &img, qint32 x, qint32 y, qint32 w, qint32 h)
{
    uchar *j = img.bits();

    KoColorSpace *cs = m_paintdev->colorSpace();

    for (qint32 y2 = y; y2 < h + y; ++y2) {
        KisHLineIteratorPixel it = m_paintdev->createHLineIterator(x, y2, w, false);
        while ( ! it.isDone()) {
            quint8 s = cs->getAlpha(it.rawData());
            if(s==0)
            {
                quint8 g = (*(j + 0)  + *(j + 1 ) + *(j + 2 )) / 9;

                *(j+0) = 128+g ;
                *(j+1) = 165+g;
                *(j+2) = 128+g;
            }
            j+=4;
            ++it;
        }
    }
}

QImage KisPaintLayer::createThumbnail(qint32 w, qint32 h)
{
    if (m_paintdev)
        return m_paintdev->createThumbnail(w, h);
    else
        return QImage();
}


qint32 KisPaintLayer::x() const { if (m_paintdev) return m_paintdev->getX(); else return 0; }

void KisPaintLayer::setX(qint32 x)
{
    if (m_paintdev)
        m_paintdev->setX(x);
}

qint32 KisPaintLayer::y() const { if (m_paintdev) return m_paintdev->getY(); else return 0;  }
void KisPaintLayer::setY(qint32 y) { if (m_paintdev) m_paintdev->setY(y); }

QRect KisPaintLayer::extent() const { if (m_paintdev) return m_paintdev->extent(); else return QRect(); }
QRect KisPaintLayer::exactBounds() const { if (m_paintdev) return m_paintdev->exactBounds(); else return QRect(); }

#include "kis_paint_layer.moc"
