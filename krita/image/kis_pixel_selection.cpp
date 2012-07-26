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
#include <kis_iterator_ng.h>

struct KisPixelSelection::Private {
};

KisPixelSelection::KisPixelSelection(KisDefaultBoundsBaseSP defaultBounds)
        : KisPaintDevice(0, KoColorSpaceRegistry::instance()->alpha8(), defaultBounds)
        , m_d(new Private)
{
}

KisPixelSelection::KisPixelSelection(const KisPixelSelection& rhs)
        : KisPaintDevice(rhs)
        , KisSelectionComponent(rhs)
        , m_d(new Private)
{
}

KisSelectionComponent* KisPixelSelection::clone(KisSelection*)
{
    return new KisPixelSelection(*this);
}

KisPixelSelection::~KisPixelSelection()
{
    delete m_d;
}

KisPaintDeviceSP KisPixelSelection::createThumbnailDevice(qint32 w, qint32 h, QRect rect) const
{
    KisPaintDeviceSP dev =
        KisPaintDevice::createThumbnailDevice(w, h, rect);

    QRect bounds = dev->exactBounds();
    KisHLineIteratorSP it = dev->createHLineIteratorNG(bounds.x(), bounds.y(), bounds.width());

    for (int y2 = bounds.y(); y2 < bounds.height() + bounds.y(); ++y2) {
        do {
            *(it->rawData()) = MAX_SELECTED - *(it->rawData());
        } while (it->nextPixel());
        it->nextRow();
    }
    return dev;
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

void KisPixelSelection::applySelection(KisPixelSelectionSP selection, SelectionAction action)
{
    switch (action) {
    case SELECTION_REPLACE:
        clear();
        addSelection(selection);
        break;
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
    KisHLineIteratorSP dst = createHLineIteratorNG(r.x(), r.y(), r.width());
    KisHLineConstIteratorSP src = selection->createHLineConstIteratorNG(r.x(), r.y(), r.width());
    for (int i = 0; i < r.height(); ++i) {
        do {
            if (*src->oldRawData() + *dst->rawData() < MAX_SELECTED)
                *dst->rawData() = *src->oldRawData() + *dst->rawData();
            else
                *dst->rawData() = MAX_SELECTED;

        } while (src->nextPixel() && dst->nextPixel());
        dst->nextRow();
        src->nextRow();
    }
}

void KisPixelSelection::subtractSelection(KisPixelSelectionSP selection)
{
    QRect r = selection->selectedRect();
    KisHLineIteratorSP dst = createHLineIteratorNG(r.x(), r.y(), r.width());
    KisHLineConstIteratorSP src = selection->createHLineConstIteratorNG(r.x(), r.y(), r.width());
    for (int i = 0; i < r.height(); ++i) {
        do {
            if (*dst->rawData() - *src->oldRawData() > MIN_SELECTED)
                *dst->rawData() = *dst->rawData() - *src->oldRawData();
            else
                *dst->rawData() = MIN_SELECTED;

        } while (src->nextPixel() && dst->nextPixel());
        dst->nextRow();
        src->nextRow();
    }
}

void KisPixelSelection::intersectSelection(KisPixelSelectionSP selection)
{
    QRect r = selection->selectedRect().united(selectedRect());

    KisHLineIteratorSP dst = createHLineIteratorNG(r.x(), r.y(), r.width());
    KisHLineConstIteratorSP src = selection->createHLineConstIteratorNG(r.x(), r.y(), r.width());
    for (int i = 0; i < r.height(); ++i) {
        do {
            *dst->rawData() = qMin(*dst->rawData(), *src->oldRawData());
        }  while (src->nextPixel() && dst->nextPixel());
        dst->nextRow();
        src->nextRow();
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
    // Region is needed here (not exactBounds or extent), because
    // unselected but existing pixels need to be inverted too
    QRect rc = region().boundingRect();

    if (!rc.isEmpty()) {
#if 0
        quint8 *bytes = new quint8[rc.width()];
        for(int row = rc.y(); row < rc.height(); ++row) {
            readBytes(bytes, rc.x(), row, rc.width(), 1);
            for (int i = 0; i < rc.width(); ++i) {
                bytes[i] = MAX_SELECTED - bytes[i];
            }
            writeBytes(bytes, rc.x(), row, rc.width(), 1);
        }
        delete []bytes;
#else
        KisRectIteratorSP it = createRectIteratorNG(rc.x(), rc.y(), rc.width(), rc.height());
        do {
            *(it->rawData()) = MAX_SELECTED - *(it->rawData());
        } while (it->nextPixel());
#endif
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

QRect KisPixelSelection::selectedRect() const
{
    return extent();
}

QRect KisPixelSelection::selectedExactRect() const
{
    return exactBounds();
}

QVector<QPolygon> KisPixelSelection::outline() const
{
    QRect selectionExtent = selectedExactRect();
    qint32 xOffset = selectionExtent.x();
    qint32 yOffset = selectionExtent.y();
    qint32 width = selectionExtent.width();
    qint32 height = selectionExtent.height();

    KisOutlineGenerator generator(colorSpace(), MIN_SELECTED);
    // If the selection is small using a buffer is much fast
    if (width*height < 5000000) {
        quint8* buffer = new quint8[width*height];
        readBytes(buffer, xOffset, yOffset, width, height);

        QVector<QPolygon> paths = generator.outline(buffer, xOffset, yOffset, width, height);

        delete[] buffer;
        return paths;
    }
    return generator.outline(this, xOffset, yOffset, width, height);
}

void KisPixelSelection::renderToProjection(KisPaintDeviceSP projection)
{
    renderToProjection(projection, selectedExactRect());
}

void KisPixelSelection::renderToProjection(KisPaintDeviceSP projection, const QRect& rc)
{
    QRect updateRect = rc & selectedExactRect();

    if (updateRect.isValid()) {
        KisPainter painter(projection);
        painter.setCompositeOp(COMPOSITE_COPY);
        painter.bitBlt(updateRect.topLeft(), KisPaintDeviceSP(this), updateRect);
        painter.end();
    }
}
