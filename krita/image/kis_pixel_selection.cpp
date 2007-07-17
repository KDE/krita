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

#include <kdebug.h>
#include <QPoint>
#include <QPolygon>

#include "KoColorSpaceRegistry.h"
#include "KoIntegerMaths.h"

#include "kis_layer.h"
#include "kis_debug_areas.h"
#include "kis_types.h"
#include "kis_fill_painter.h"
#include "kis_iterators_pixel.h"
#include "kis_image.h"
#include "kis_datamanager.h"
#include "kis_fill_painter.h"
#include "kis_mask.h"

KisPixelSelection::KisPixelSelection(KisPaintDeviceSP dev)
    : KisMask(dev,
              QString("selection for ") + dev->objectName())
    , m_parentPaintDevice(dev)
    , m_dirty(false)
{
    Q_ASSERT(dev);
}


KisPixelSelection::KisPixelSelection( KisPaintDeviceSP parent, KisMaskSP mask )
    : KisMask( parent, "selection from mask" )
    , m_parentPaintDevice( parent )
    , m_dirty( false )
{
    m_datamanager = mask->dataManager();
}

KisPixelSelection::KisPixelSelection()
    : KisMask("anonymous selection")
    , m_parentPaintDevice(0)
    , m_dirty(false)
{
}

KisPixelSelection::KisPixelSelection(const KisPixelSelection& rhs)
    : KisMask(rhs)
    , m_parentPaintDevice(rhs.m_parentPaintDevice), m_dirty(false)
{
}

KisPixelSelection::~KisPixelSelection()
{
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

QImage KisPixelSelection::maskImage( KisImageSP image ) const
{
    // If part of a KisAdjustmentLayer, there may be no parent device.
    QImage img;
    QRect bounds;
    if (m_parentPaintDevice) {

        bounds = m_parentPaintDevice->exactBounds();
        bounds = bounds.intersect( image->bounds() );
        img = QImage(bounds.width(), bounds.height(), QImage::Format_RGB32);
    }
    else {
        bounds = QRect( 0, 0, image->width(), image->height());
        img = QImage(bounds.width(), bounds.height(), QImage::Format_RGB32);
    }

    KisHLineConstIteratorPixel it = createHLineConstIterator(bounds.x(), bounds.y(), bounds.width());
    for (int y2 = bounds.y(); y2 < bounds.height() - bounds.y(); ++y2) {
        int x2 = 0;
        while (!it.isDone()) {
            quint8 s = MAX_SELECTED - *(it.rawData());
            qint32 c = qRgb(s, s, s);
            img.setPixel(x2, y2, c);
            ++x2;
            ++it;
        }
        it.nextRow(); // XXX: Why wasn't this line here? Used to be
                      // present in 1.6.
    }
    return img;
}
void KisPixelSelection::select(QRect r)
{
    KisFillPainter painter(KisPaintDeviceSP(this));
    KoColorSpace * cs = KoColorSpaceRegistry::instance()->rgb8();
    painter.fillRect(r, KoColor(Qt::white, cs), MAX_SELECTED);
    qint32 x, y, w, h;
    extent(x, y, w, h);
}

void KisPixelSelection::clear(QRect r)
{
    KisFillPainter painter(KisPaintDeviceSP(this));
    KoColorSpace * cs = KoColorSpaceRegistry::instance()->rgb8();
    painter.fillRect(r, KoColor(Qt::white, cs), MIN_SELECTED);
}

void KisPixelSelection::clear()
{
    quint8 defPixel = MIN_SELECTED;
    m_datamanager->setDefaultPixel(&defPixel);
    m_datamanager->clear();
}

void KisPixelSelection::invert()
{
    qint32 x,y,w,h;

    extent(x, y, w, h);
    KisRectIterator it = createRectIterator(x, y, w, h);
    while ( ! it.isDone() )
    {
        // CBR this is wrong only first byte is inverted
        // BSAR: But we have always only one byte in this color model :-).
        *(it.rawData()) = MAX_SELECTED - *(it.rawData());
        ++it;
    }
    quint8 defPixel = MAX_SELECTED - *(m_datamanager->defaultPixel());
    m_datamanager->setDefaultPixel(&defPixel);
}

bool KisPixelSelection::isTotallyUnselected(QRect r) const
{
    if(*(m_datamanager->defaultPixel()) != MIN_SELECTED)
        return false;
    QRect sr = selectedExactRect();
    return ! r.intersects(sr);
}

bool KisPixelSelection::isProbablyTotallyUnselected(QRect r) const
{
    if(*(m_datamanager->defaultPixel()) != MIN_SELECTED)
        return false;
    QRect sr = selectedRect();
    return ! r.intersects(sr);
}


QRect KisPixelSelection::selectedRect() const
{
    if(*(m_datamanager->defaultPixel()) == MIN_SELECTED || !m_parentPaintDevice)
        return extent();
    else
        return extent().unite(m_parentPaintDevice->extent());
}

QRect KisPixelSelection::selectedExactRect() const
{
    if(*(m_datamanager->defaultPixel()) == MIN_SELECTED || !m_parentPaintDevice)
        return exactBounds();
    else
        return exactBounds().unite(m_parentPaintDevice->exactBounds());
}

void KisPixelSelection::setDirty(const QRect& rc)
{
    if (m_dirty)
        KisMask::setDirty(rc);
}

void KisPixelSelection::setDirty()
{
    if (m_dirty)
        KisMask::setDirty();
}

QVector<QPolygon> KisPixelSelection::outline()
{
    QTime t;
    t.start();

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

            if(buffer[y*width+x]== defaultPixel)
                continue;

            EdgeType startEdge = TopEdge;

            EdgeType edge = startEdge;
            while( edge != NoEdge && (marks[y*width+x] & (1 << edge) || !isOutlineEdge(edge, x, y, buffer, width, height)))
            {
                edge = nextEdge(edge);
                if (edge == startEdge)
                    edge = NoEdge;
            }

            if (edge != NoEdge)
            {
                QPolygon path;
                path << QPoint(x+xOffset, y+yOffset);

                bool clockwise = edge == BottomEdge;

                qint32 row = y, col = x;
                EdgeType currentEdge = edge;
                EdgeType lastEdge = currentEdge;
                do {
                    //While following a strait line no points nead to be added
                    if(lastEdge != currentEdge){
                        appendCoordinate(&path, col+xOffset, row+yOffset, currentEdge);
                        nodes++;
                        lastEdge = currentEdge;
                    }

                    marks[row*width+col] |= 1 << currentEdge;
                    nextOutlineEdge( &currentEdge, &row, &col, buffer, width, height);
                }
                while (row != y || col != x || currentEdge != edge);

                paths.push_back(path);
            }
        }
    }
    delete[] buffer;
    delete[] marks;

    return paths;
}



bool KisPixelSelection::isOutlineEdge(EdgeType edge, qint32 x, qint32 y, quint8* buffer, qint32 bufWidth, qint32 bufHeight )
{
    quint8 defaultPixel = *(m_datamanager->defaultPixel());
    if(buffer[y*bufWidth+x] == defaultPixel)
        return false;

    switch(edge){
        case LeftEdge:
            return x == 0 || buffer[y*bufWidth+(x - 1)] == defaultPixel;
        case TopEdge:
            return y == 0 || buffer[(y - 1)*bufWidth+x] == defaultPixel;
        case RightEdge:
            return x == bufWidth -1 || buffer[y*bufWidth+(x + 1)] == defaultPixel;
        case BottomEdge:
            return y == bufHeight -1 || buffer[(y + 1)*bufWidth+x] == defaultPixel;
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

  switch (*edge){
    case RightEdge:
      TRY_PIXEL( -1, 0, RightEdge);
      TRY_PIXEL( -1, 1, BottomEdge);
      break;

    case TopEdge:
      TRY_PIXEL( 0, -1, TopEdge);
      TRY_PIXEL( -1, -1, RightEdge);
      break;

    case LeftEdge:
      TRY_PIXEL( 1, 0, LeftEdge);
      TRY_PIXEL( 1, -1, TopEdge);
      break;

    case BottomEdge:
      TRY_PIXEL( 0, 1, BottomEdge);
      TRY_PIXEL( 1, 1, LeftEdge);
      break;

    default:
        break;

    }

  if (*row == original_row && *col == original_col)
    *edge = nextEdge (*edge);
}

void KisPixelSelection::appendCoordinate(QPolygon * path, int x, int y, EdgeType edge)
{
  switch (edge)
    {
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
      break;

    }
    *path << QPoint(x, y);
}

void KisPixelSelection::renderToProjection(KisSelection* projection)
{
    KisPainter painter(projection);
    QRect r = selectedExactRect();
    painter.bitBlt(r.x(), r.y(), COMPOSITE_COPY, KisPaintDeviceSP(this), r.x(), r.y(), r.width(), r.height());
    painter.end();
}
