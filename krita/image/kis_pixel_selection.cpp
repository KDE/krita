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
#include "kis_datamanager.h"
#include "kis_fill_painter.h"

struct KisPixelSelection::Private {
    KisPaintDeviceWSP parentPaintDevice;
    bool interestedInDirtyness;
};

KisPixelSelection::KisPixelSelection()
        : KisPaintDevice(KoColorSpaceRegistry::instance()->alpha8())
        , m_d(new Private)
{
    m_d->parentPaintDevice = 0;
    m_d->interestedInDirtyness = true;

}

KisPixelSelection::KisPixelSelection(KisPaintDeviceSP dev)
        : KisPaintDevice(KoColorSpaceRegistry::instance()->alpha8())
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

KisSelectionComponent* KisPixelSelection::clone()
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
        int x2 = 0;
        while (!it.isDone()) {
            quint8 s = MAX_SELECTED - *(it.rawData());
            qint32 c = qRgb(s, s, s);
            image.setPixel(x2, y2, c);
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
    if (*(m_datamanager->defaultPixel()) != MIN_SELECTED) {
        KisFillPainter painter(KisPaintDeviceSP(this));
        const KoColorSpace * cs = KoColorSpaceRegistry::instance()->rgb8();
        painter.fillRect(r, KoColor(Qt::white, cs), MIN_SELECTED);
    } else {
        m_datamanager->clear(r.x(), r.y(), r.width(), r.height(), m_datamanager->defaultPixel());
    }
}

void KisPixelSelection::clear()
{
    quint8 defPixel = MIN_SELECTED;
    m_datamanager->setDefaultPixel(&defPixel);
    m_datamanager->clear();
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
    quint8 defPixel = MAX_SELECTED - *(m_datamanager->defaultPixel());
    m_datamanager->setDefaultPixel(&defPixel);
}

bool KisPixelSelection::isTotallyUnselected(const QRect & r) const
{
    if (*(m_datamanager->defaultPixel()) != MIN_SELECTED)
        return false;
    QRect sr = selectedExactRect();
    return ! r.intersects(sr);
}

bool KisPixelSelection::isProbablyTotallyUnselected(const QRect & r) const
{
    if (*(m_datamanager->defaultPixel()) != MIN_SELECTED)
        return false;
    QRect sr = selectedRect();
    return ! r.intersects(sr);
}


QRect KisPixelSelection::selectedRect() const
{
    if (*(m_datamanager->defaultPixel()) == MIN_SELECTED) {
        if (m_d->parentPaintDevice) {
            // The selected exact rect is the area of this selection that overlaps
            // with the parent paint device.
            return m_d->parentPaintDevice->extent().intersected(extent());
        } else {
            return extent();
        }
    } else {
        if (m_d->parentPaintDevice) {
            // By default all pixels are selected, to the size of the parent paint device.
            return m_d->parentPaintDevice->extent();
        } else {
            // By default all pixels are selected; no matter how many pixels are
            // marked as deselected, there are always by-default-selected pixels
            // around the deselected pixels.
            return QRect(0, 0, qint32_MAX, qint32_MAX);
        }
    }
}

QRect KisPixelSelection::selectedExactRect() const
{
    if (*(m_datamanager->defaultPixel()) == MIN_SELECTED) {
        if (m_d->parentPaintDevice) {
            // The selected exact rect is the area of this selection that overlaps
            // with the parent paint device.
            return m_d->parentPaintDevice->exactBounds().intersected(exactBounds());
        } else {
            return exactBounds();
        }
    } else {
        if (m_d->parentPaintDevice) {
            // By default all pixels are selected, to the size of the parent paint device.
            return m_d->parentPaintDevice->exactBounds();
        } else {
            // By default all pixels are selected; no matter how many pixels are
            // marked as deselected, there are always by-default-selected pixels
            // around the deselected pixels.
            return QRect(0, 0, qint32_MAX, qint32_MAX);
        }
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
    quint8 defaultPixel = *(m_datamanager->defaultPixel());
    QRect selectionExtent = exactBounds();
    qint32 xOffset = selectionExtent.x();
    qint32 yOffset = selectionExtent.y();
    qint32 width = selectionExtent.width();
    qint32 height = selectionExtent.height();

    quint8* buffer = new quint8[width*height];
    quint8* marks = new quint8[width*height];
    for (int i = 0; i < width*height; i++) {
        marks[i] = 0;
    }
    QVector<QPolygon> paths;

    readBytes(buffer, xOffset, yOffset, width, height);

    int nodes = 0;
    for (qint32 y = 0; y < height; y++) {
        for (qint32 x = 0; x < width; x++) {

            if (buffer[y*width+x] == defaultPixel)
                continue;

            EdgeType startEdge = TopEdge;

            EdgeType edge = startEdge;
            while (edge != NoEdge && (marks[y*width+x] & (1 << edge) || !isOutlineEdge(edge, x, y, buffer, width, height))) {
                edge = nextEdge(edge);
                if (edge == startEdge)
                    edge = NoEdge;
            }

            if (edge != NoEdge) {
                QPolygon path;
                path << QPoint(x + xOffset, y + yOffset);

// XXX: Unused? (BSAR)
//                bool clockwise = edge == BottomEdge;

                qint32 row = y, col = x;
                EdgeType currentEdge = edge;
                EdgeType lastEdge = currentEdge;
                do {
                    //While following a strait line no points nead to be added
                    if (lastEdge != currentEdge) {
                        appendCoordinate(&path, col + xOffset, row + yOffset, currentEdge);
                        nodes++;
                        lastEdge = currentEdge;
                    }

                    marks[row*width+col] |= 1 << currentEdge;
                    nextOutlineEdge(&currentEdge, &row, &col, buffer, width, height);
                } while (row != y || col != x || currentEdge != edge);

                paths.push_back(path);
            }
        }
    }
    delete[] buffer;
    delete[] marks;

    return paths;
}
bool KisPixelSelection::isOutlineEdge(EdgeType edge, qint32 x, qint32 y, quint8* buffer, qint32 bufWidth, qint32 bufHeight)
{
    quint8 defaultPixel = *(m_datamanager->defaultPixel());
    if (buffer[y*bufWidth+x] == defaultPixel)
        return false;

    switch (edge) {
    case LeftEdge:
        return x == 0 || buffer[y*bufWidth+(x - 1)] == defaultPixel;
    case TopEdge:
        return y == 0 || buffer[(y - 1)*bufWidth+x] == defaultPixel;
    case RightEdge:
        return x == bufWidth - 1 || buffer[y*bufWidth+(x + 1)] == defaultPixel;
    case BottomEdge:
        return y == bufHeight - 1 || buffer[(y + 1)*bufWidth+x] == defaultPixel;
    case NoEdge:
        return false;
    }
    return false;
}

#define TRY_PIXEL(deltaRow, deltaCol, test_edge)                                                \
    {                                                                                               \
        int test_row = *row + deltaRow;                                                             \
        int test_col = *col + deltaCol;                                                             \
        if ( (0 <= (test_row) && (test_row) < height && 0 <= (test_col) && (test_col) < width) &&   \
                isOutlineEdge (test_edge, test_col, test_row, buffer, width, height))                  \
        {                                                                                           \
            *row = test_row;                                                                        \
            *col = test_col;                                                                        \
            *edge = test_edge;                                                                      \
            break;                                                                                  \
        }                                                                                       \
    }

void KisPixelSelection::nextOutlineEdge(EdgeType *edge, qint32 *row, qint32 *col, quint8* buffer, qint32 width, qint32 height)
{
    int original_row = *row;
    int original_col = *col;

    switch (*edge) {
    case RightEdge:
        TRY_PIXEL(-1, 0, RightEdge);
        TRY_PIXEL(-1, 1, BottomEdge);
        break;

    case TopEdge:
        TRY_PIXEL(0, -1, TopEdge);
        TRY_PIXEL(-1, -1, RightEdge);
        break;

    case LeftEdge:
        TRY_PIXEL(1, 0, LeftEdge);
        TRY_PIXEL(1, -1, TopEdge);
        break;

    case BottomEdge:
        TRY_PIXEL(0, 1, BottomEdge);
        TRY_PIXEL(1, 1, LeftEdge);
        break;

    default:
        break;

    }

    if (*row == original_row && *col == original_col)
        *edge = nextEdge(*edge);
}

void KisPixelSelection::appendCoordinate(QPolygon * path, int x, int y, EdgeType edge)
{
    switch (edge) {
    case TopEdge:
        x++;
        break;
    case RightEdge:
        x++;
        y++;
        break;
    case BottomEdge:
        y++;
        break;
    case LeftEdge:
    case NoEdge:
        break;

    }
    *path << QPoint(x, y);
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
