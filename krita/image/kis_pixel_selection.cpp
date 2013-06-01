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

#include <KoColorSpace.h>
#include <KoColorSpaceRegistry.h>
#include <KoColorModelStandardIds.h>
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
    QPainterPath outlineCache;
    bool outlineCacheValid;
};

KisPixelSelection::KisPixelSelection(KisDefaultBoundsBaseSP defaultBounds)
        : KisPaintDevice(0, KoColorSpaceRegistry::instance()->alpha8(), defaultBounds)
        , m_d(new Private)
{
    m_d->outlineCacheValid = true;
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

const KoColorSpace *KisPixelSelection::compositionSourceColorSpace() const
{
    return KoColorSpaceRegistry::instance()->
        colorSpace(GrayAColorModelID.id(),
                   Integer8BitsColorDepthID.id(),
                   QString());
}

void KisPixelSelection::select(const QRect & rc, quint8 selectedness)
{
    QRect r = rc.normalized();
    if (r.isEmpty()) return;

    KisFillPainter painter(KisPaintDeviceSP(this));
    const KoColorSpace * cs = KoColorSpaceRegistry::instance()->rgb8();
    painter.fillRect(r, KoColor(Qt::white, cs), selectedness);

    if (m_d->outlineCacheValid) {
        QPainterPath path;
        path.addRect(r);

        if (selectedness != MIN_SELECTED) {
            m_d->outlineCache += path;
        } else {
            m_d->outlineCache -= path;
        }
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
    if (r.isEmpty()) return;

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

    m_d->outlineCacheValid &= selection->outlineCacheValid();

    if (m_d->outlineCacheValid) {
        m_d->outlineCache += selection->outlineCache();
    }
}

void KisPixelSelection::subtractSelection(KisPixelSelectionSP selection)
{
    QRect r = selection->selectedRect();
    if (r.isEmpty()) return;


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

    m_d->outlineCacheValid &= selection->outlineCacheValid();

    if (m_d->outlineCacheValid) {
        m_d->outlineCache -= selection->outlineCache();
    }
}

void KisPixelSelection::intersectSelection(KisPixelSelectionSP selection)
{    
    QRect r = selection->selectedRect().united(selectedRect());
    if (r.isEmpty()) return;

    KisHLineIteratorSP dst = createHLineIteratorNG(r.x(), r.y(), r.width());
    KisHLineConstIteratorSP src = selection->createHLineConstIteratorNG(r.x(), r.y(), r.width());
    for (int i = 0; i < r.height(); ++i) {
        do {
            *dst->rawData() = qMin(*dst->rawData(), *src->oldRawData());
        }  while (src->nextPixel() && dst->nextPixel());
        dst->nextRow();
        src->nextRow();
    }

    m_d->outlineCacheValid &= selection->outlineCacheValid();

    if (m_d->outlineCacheValid) {
        m_d->outlineCache &= selection->outlineCache();
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

    if (m_d->outlineCacheValid) {
        QPainterPath path;
        path.addRect(r);

        m_d->outlineCache -= path;
    }
}

void KisPixelSelection::clear()
{
    quint8 defPixel = MIN_SELECTED;
    setDefaultPixel(&defPixel);
    KisPaintDevice::clear();

    m_d->outlineCacheValid = true;
    m_d->outlineCache = QPainterPath();
}

void KisPixelSelection::invert()
{
    // Region is needed here (not exactBounds or extent), because
    // unselected but existing pixels need to be inverted too
    QRect rc = region().boundingRect();

    if (!rc.isEmpty()) {
        KisRectIteratorSP it = createRectIteratorNG(rc.x(), rc.y(), rc.width(), rc.height());
        do {
            *(it->rawData()) = MAX_SELECTED - *(it->rawData());
        } while (it->nextPixel());
    }
    quint8 defPixel = MAX_SELECTED - *defaultPixel();
    setDefaultPixel(&defPixel);

    if (m_d->outlineCacheValid) {
        QPainterPath path;
        path.addRect(defaultBounds()->bounds());

        m_d->outlineCache = path - m_d->outlineCache;
    }
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

QPainterPath KisPixelSelection::outlineCache() const
{
    return m_d->outlineCache;
}

void KisPixelSelection::setOutlineCache(const QPainterPath &cache)
{
    m_d->outlineCache = cache;
    m_d->outlineCacheValid = true;
}

bool KisPixelSelection::outlineCacheValid() const
{
    return m_d->outlineCacheValid;
}

void KisPixelSelection::invalidateOutlineCache()
{
    m_d->outlineCacheValid = false;
}

void KisPixelSelection::recalculateOutlineCache()
{
    foreach (const QPolygon &polygon, outline()) {
        m_d->outlineCache.addPolygon(polygon);
        m_d->outlineCacheValid = true;
    }
}

void KisPixelSelection::renderToProjection(KisPaintDeviceSP projection)
{
    renderToProjection(projection, selectedExactRect());
}

void KisPixelSelection::notifySelectionChanged()
{
    KisNodeSP parentNode;
    if (!(parentNode = this->parentNode())) return;

    KisNodeGraphListener *listener;
    if (!(listener = parentNode->graphListener())) return;

    listener->notifySelectionChanged();
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
