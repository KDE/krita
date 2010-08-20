/*
 *  Copyright (c) 2004 Boudewijn Rempt <boud@valdyas.org>
 *  Copyright (c) 2007 Sven Langkamp <sven.langkamp@gmail.com>
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

#include "kis_pixel_selection.h"


#include <QImage>
#include <QVector>

#include <QPoint>
#include <QPolygon>

#include <KoColorSpaceRegistry.h>
#include <KoColorSpace.h>
#include <KoIntegerMaths.h>
#include <KoCompositeOp.h>

#include "kis_layer.h"
#include "kis_debug.h"
#include "kis_types.h"
#include "kis_image.h"
#include "kis_fill_painter.h"
#include "kis_outline_generator.h"

struct KisPixelSelection::Private {
    KisPaintDeviceWSP parentPaintDevice;
    bool interestedInDirtyness;
};

KisPixelSelection::KisPixelSelection(KisDefaultBounds defaultBounds)
        : KisPaintDevice(0, KoColorSpaceRegistry::instance()->alpha8(), defaultBounds)
        , m_d(new Private)
{
    m_d->parentPaintDevice = 0;
    m_d->interestedInDirtyness = true;

}

KisPixelSelection::KisPixelSelection(KisPaintDeviceSP dev, KisDefaultBounds defaultBounds)
        : KisPaintDevice(0, KoColorSpaceRegistry::instance()->alpha8(), defaultBounds)
        , m_d(new Private)
{
    Q_ASSERT(dev);
    m_d->parentPaintDevice = dev;
    m_d->interestedInDirtyness = true;
}

KisPixelSelection::KisPixelSelection(const KisPixelSelection& rhs)
        : KisPaintDevice(rhs)
        , KisSelectionComponent(rhs)
        , m_d(new Private)
{
    m_d->parentPaintDevice = rhs.m_d->parentPaintDevice.data();
    m_d->interestedInDirtyness = rhs.m_d->interestedInDirtyness;

}

KisSelectionComponent* KisPixelSelection::clone(KisSelection*)
{
    return new KisPixelSelection(*this);
}

KisPixelSelection::~KisPixelSelection()
{
    delete m_d;
}

quint8 KisPixelSelection::selected(qint32 x, qint32 y) const
{
    KisHLineConstIteratorPixel iter = createHLineConstIterator(x, y, 1);

    const quint8 *pix = iter.rawData();

    return *pix;
}

void KisPixelSelection::setSelected(qint32 x, qint32 y, quint8 s)
{
    KisHLineIteratorPixel iter = createHLineIterator(x, y, 1);

    quint8 *pix = iter.rawData();

    *pix = s;
}

QImage KisPixelSelection::maskImage(const QRect & rc) const
{
    // If part of a KisAdjustmentLayer, there may be no parent device.
    QImage image;
    QRect bounds;
    if (m_d->parentPaintDevice) {
        bounds = m_d->parentPaintDevice->exactBounds();
        bounds = bounds.intersect(rc);
        image = QImage(bounds.width(), bounds.height(), QImage::Format_RGB32);
    } else {
        bounds = rc;
        image = QImage(bounds.width(), bounds.height(), QImage::Format_RGB32);
    }

    KisHLineConstIteratorPixel it = createHLineConstIterator(bounds.x(), bounds.y(), bounds.width());
    for (int y2 = bounds.y(); y2 < bounds.height() - bounds.y(); ++y2) {
        QRgb *pixel= reinterpret_cast<QRgb *>(image.scanLine(y2));
        int x2 = 0;
        while (!it.isDone()) {
            quint8 s = MAX_SELECTED - *(it.rawData());
            qint32 c = qRgb(s, s, s);
            pixel[x2] = c;
            ++x2;
            ++it;
        }
        it.nextRow(); // XXX: Why wasn't this line here? Used to be
        // present in 1.6.
    }
    return image;
}
void KisPixelSelection::select(const QRect & rc, quint8 selectedness)
{
    QRect r = rc.normalized();
    if (r.width() > 0 && r.height() > 0) {
        KisFillPainter painter(KisPaintDeviceSP(this));
        const KoColorSpace * cs = KoColorSpaceRegistry::instance()->rgb8();
        painter.fillRect(r, KoColor(Qt::white, cs), selectedness);
    }
}

void KisPixelSelection::applySelection(KisPixelSelectionSP selection, selectionAction action)
{
    switch (action) {
    case SELECTION_REPLACE:
        // XXX: Shouldn't we actually replace the selection, instead of falling through to add?
    case SELECTION_ADD:
        addSelection(selection);
        break;
    case SELECTION_SUBTRACT:
        subtractSelection(selection);
        break;
    case SELECTION_INTERSECT:
        intersectSelection(selection);
        break;
    default:
        break;
    }
}

void KisPixelSelection::addSelection(KisPixelSelectionSP selection)
{
    QRect r = selection->selectedRect();
    KisHLineIteratorPixel dst = createHLineIterator(r.x(), r.y(), r.width());
    KisHLineConstIteratorPixel src = selection->createHLineConstIterator(r.x(), r.y(), r.width());
    for (int i = 0; i < r.height(); ++i) {
        while (!src.isDone()) {
            if (*src.rawData() + *dst.rawData() < MAX_SELECTED)
                *dst.rawData() = *src.rawData() + *dst.rawData();
            else
                *dst.rawData() = MAX_SELECTED;
            ++src;
            ++dst;
        }
        dst.nextRow();
        src.nextRow();
    }

}

void KisPixelSelection::subtractSelection(KisPixelSelectionSP selection)
{
    QRect r = selection->selectedRect();
    KisHLineIteratorPixel dst = createHLineIterator(r.x(), r.y(), r.width());
    KisHLineConstIteratorPixel src = selection->createHLineConstIterator(r.x(), r.y(), r.width());
    for (int i = 0; i < r.height(); ++i) {
        while (!src.isDone()) {
            if (*dst.rawData() - *src.rawData() > MIN_SELECTED)
                *dst.rawData() = *dst.rawData() - *src.rawData();
            else
                *dst.rawData() = MIN_SELECTED;
            ++src;
            ++dst;
        }
        dst.nextRow();
        src.nextRow();
    }

}

void KisPixelSelection::intersectSelection(KisPixelSelectionSP selection)
{
    QRect r = selection->selectedRect().united(selectedRect());

    KisHLineIteratorPixel dst = createHLineIterator(r.x(), r.y(), r.width());
    KisHLineConstIteratorPixel src = selection->createHLineConstIterator(r.x(), r.y(), r.width());
    for (int i = 0; i < r.height(); ++i) {
        while (!src.isDone()) {
            if (*dst.rawData() == MAX_SELECTED && *src.rawData() == MAX_SELECTED)
                *dst.rawData() = MAX_SELECTED;
            else
                *dst.rawData() = MIN_SELECTED;
            ++src;
            ++dst;
        }
        dst.nextRow();
        src.nextRow();
    }
}

void KisPixelSelection::clear(const QRect & r)
{
    if (*defaultPixel() != MIN_SELECTED) {
        KisFillPainter painter(KisPaintDeviceSP(this));
        const KoColorSpace * cs = KoColorSpaceRegistry::instance()->rgb8();
        painter.fillRect(r, KoColor(Qt::white, cs), MIN_SELECTED);
    } else {
        KisPaintDevice::clear(r);
    }
}

void KisPixelSelection::clear()
{
    quint8 defPixel = MIN_SELECTED;
    setDefaultPixel(&defPixel);
    KisPaintDevice::clear();
}

void KisPixelSelection::invert()
{
    // Extent is needed here (not exactBounds), because unselected but existing pixel
    // need to be inverted too
    QRect rc = extent();

    KisRectIterator it = createRectIterator(rc.x(), rc.y(), rc.width(), rc.height());
    while (! it.isDone()) {
        *(it.rawData()) = MAX_SELECTED - *(it.rawData());
        ++it;
    }
    quint8 defPixel = MAX_SELECTED - *defaultPixel();
    setDefaultPixel(&defPixel);
}

bool KisPixelSelection::isTotallyUnselected(const QRect & r) const
{
    if (*defaultPixel() != MIN_SELECTED)
        return false;
    QRect sr = selectedExactRect();
    return ! r.intersects(sr);
}

bool KisPixelSelection::isProbablyTotallyUnselected(const QRect & r) const
{
    if (*defaultPixel() != MIN_SELECTED)
        return false;
    QRect sr = selectedRect();
    return ! r.intersects(sr);
}


QRect KisPixelSelection::selectedRect() const
{
    if (*defaultPixel() == MIN_SELECTED) {
        return extent().intersected(defaultBounds().bounds());
    } else {
        return defaultBounds().bounds();
    }
}

QRect KisPixelSelection::selectedExactRect() const
{
    if (*defaultPixel() == MIN_SELECTED) {
        return exactBounds().intersected(defaultBounds().bounds());
    } else {
        return defaultBounds().bounds();
    }
}

void KisPixelSelection::setInterestedInDirtyness(bool b)
{
    m_d->interestedInDirtyness = b;
}

bool KisPixelSelection::interestedInDirtyness() const
{
    return m_d->interestedInDirtyness;
}

void KisPixelSelection::setDirty(const QRect& rc)
{
    if (m_d->interestedInDirtyness)
        KisPaintDevice::setDirty(rc);
}

void KisPixelSelection::setDirty(const QRegion & region)
{
    if (m_d->interestedInDirtyness)
        KisPaintDevice::setDirty(region);
}

void KisPixelSelection::setDirty()
{
    if (m_d->interestedInDirtyness)
        KisPaintDevice::setDirty();
}

QVector<QPolygon> KisPixelSelection::outline()
{
    QRect selectionExtent = selectedExactRect();
    qint32 xOffset = selectionExtent.x();
    qint32 yOffset = selectionExtent.y();
    qint32 width = selectionExtent.width();
    qint32 height = selectionExtent.height();

    quint8* buffer = new quint8[width*height];

    readBytes(buffer, xOffset, yOffset, width, height);

    KisOutlineGenerator generator(colorSpace(), *defaultPixel());
    QVector<QPolygon> paths = generator.outline(buffer, xOffset, yOffset, width, height);
    
    delete[] buffer;

    return paths;
}

void KisPixelSelection::renderToProjection(KisSelection* projection)
{
    QRect updateRect = selectedExactRect();
    KisPainter painter(projection);
    painter.setCompositeOp(colorSpace()->compositeOp(COMPOSITE_COPY));

    painter.bitBlt(updateRect.topLeft(), KisPaintDeviceSP(this), updateRect);
    painter.end();
}

void KisPixelSelection::renderToProjection(KisSelection* projection, const QRect& r)
{
    QRect updateRect = r.intersected(selectedExactRect());
    if (updateRect.isValid()) {
        KisPainter painter(projection);
        painter.setCompositeOp(COMPOSITE_COPY);
        painter.bitBlt(updateRect.x(), updateRect.y(), KisPaintDeviceSP(this),
                       updateRect.x(), updateRect.y(), updateRect.width(), updateRect.height());
        painter.end();
    }
}
