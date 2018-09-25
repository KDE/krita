/*
 *  Copyright (c) 2004 Boudewijn Rempt <boud@valdyas.org>
 *  Copyright (c) 2007,2010 Sven Langkamp <sven.langkamp@gmail.com>
 *
 *  Outline algorithm based of the limn of fontutils
 *  Copyright (c) 1992 Karl Berry <karl@cs.umb.edu>
 *  Copyright (c) 1992 Kathryn Hargreaves <letters@cs.umb.edu>
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

#include "kis_outline_generator.h"
#include <KoColorSpace.h>
#include <KoColorSpaceRegistry.h>

#include "kis_paint_device.h"
#include <kis_iterator_ng.h>
#include <kis_random_accessor_ng.h>

class LinearStorage
{
public:
    typedef quint8* StorageType;
public:
    LinearStorage(quint8 *buffer, int width, int height, int pixelSize)
        : m_buffer(buffer),
          m_width(width),
          m_pixelSize(pixelSize)
    {
        m_marks.reset(new quint8[width * height]);
        memset(m_marks.data(), 0, width * height);
    }

    quint8* pickPixel(int x, int y) {
        return m_buffer + (m_width * y + x) * m_pixelSize;
    }

    quint8* pickMark(int x, int y) {
        return m_marks.data() + m_width * y + x;
    }

private:
    QScopedArrayPointer<quint8> m_marks;
    quint8 *m_buffer;
    int m_width;
    int m_pixelSize;
};

class PaintDeviceStorage
{
public:
    typedef const KisPaintDevice* StorageType;
public:
    PaintDeviceStorage(const KisPaintDevice *device, int /*width*/, int /*height*/, int /*pixelSize*/)
        : m_device(device)
    {
        m_deviceIt = m_device->createRandomConstAccessorNG(0, 0);

        const KoColorSpace *alphaCs = KoColorSpaceRegistry::instance()->alpha8();
        m_marks = new KisPaintDevice(alphaCs);
        m_marksIt = m_marks->createRandomAccessorNG(0, 0);
    }

    const quint8* pickPixel(int x, int y) {
        m_deviceIt->moveTo(x, y);
        return m_deviceIt->rawDataConst();
    }

    quint8* pickMark(int x, int y) {
        m_marksIt->moveTo(x, y);
        return m_marksIt->rawData();
    }

private:
    KisPaintDeviceSP m_marks;
    const KisPaintDevice *m_device;
    KisRandomConstAccessorSP m_deviceIt;
    KisRandomAccessorSP m_marksIt;
};


/******************* class KisOutlineGenerator *******************/

KisOutlineGenerator::KisOutlineGenerator(const KoColorSpace* cs, quint8 defaultOpacity)
    : m_cs(cs), m_defaultOpacity(defaultOpacity), m_simple(false)
{
}

template <class StorageStrategy>
QVector<QPolygon> KisOutlineGenerator::outlineImpl(typename StorageStrategy::StorageType buffer,
                                                   qint32 xOffset, qint32 yOffset,
                                                   qint32 width, qint32 height)
{
    QVector<QPolygon> paths;

    try {
        StorageStrategy storage(buffer, width, height, m_cs->pixelSize());

        for (qint32 y = 0; y < height; y++) {
            for (qint32 x = 0; x < width; x++) {

                if (m_cs->opacityU8(storage.pickPixel(x, y)) == m_defaultOpacity)
                    continue;

                EdgeType startEdge = TopEdge;

                EdgeType edge = startEdge;
                while (edge != NoEdge &&
                       (*storage.pickMark(x, y) & (1 << edge) ||
                        !isOutlineEdge(storage, edge, x, y, width, height))) {

                    edge = nextEdge(edge);
                    if (edge == startEdge)
                        edge = NoEdge;
                }

                if (edge != NoEdge) {
                    QPolygon path;
                    path << QPoint(x + xOffset, y + yOffset);

                    bool clockwise = edge == BottomEdge;

                    qint32 row = y, col = x;
                    EdgeType currentEdge = edge;
                    EdgeType lastEdge = currentEdge;

                    forever {
                        //While following a straight line no points need to be added
                        if (lastEdge != currentEdge) {
                            appendCoordinate(&path, col + xOffset, row + yOffset, currentEdge);
                            lastEdge = currentEdge;
                        }

                        *storage.pickMark(col, row) |= 1 << currentEdge;
                        nextOutlineEdge(storage, &currentEdge, &row, &col, width, height);

                        if (row == y && col == x && currentEdge == edge) {
                            // add last point of the polygon
                            appendCoordinate(&path, col + xOffset, row + yOffset, currentEdge);
                            break;
                        }
                    }

                    if(!m_simple || !clockwise)
                        paths.push_back(path);
                }
            }
        }
    }
    catch(const std::bad_alloc&) {
        warnKrita << "KisOutlineGenerator::outline ran out of memory allocating " <<  width << "*" << height << "marks";
    }

    return paths;
}

QVector<QPolygon> KisOutlineGenerator::outline(quint8* buffer, qint32 xOffset, qint32 yOffset, qint32 width, qint32 height)
{
    return outlineImpl<LinearStorage>(buffer, xOffset, yOffset, width, height);
}

QVector<QPolygon> KisOutlineGenerator::outline(const KisPaintDevice *buffer, qint32 xOffset, qint32 yOffset, qint32 width, qint32 height)
{
    return outlineImpl<PaintDeviceStorage>(buffer, xOffset, yOffset, width, height);
}

template <class StorageStrategy>
bool KisOutlineGenerator::isOutlineEdge(StorageStrategy &storage, EdgeType edge, qint32 x, qint32 y, qint32 bufWidth, qint32 bufHeight)
{
    if (m_cs->opacityU8(storage.pickPixel(x, y)) == m_defaultOpacity)
        return false;

    switch (edge) {
    case LeftEdge:
        return x == 0 || m_cs->opacityU8(storage.pickPixel(x - 1, y)) == m_defaultOpacity;
    case TopEdge:
        return y == 0 || m_cs->opacityU8(storage.pickPixel(x, y - 1)) == m_defaultOpacity;
    case RightEdge:
        return x == bufWidth - 1 || m_cs->opacityU8(storage.pickPixel(x + 1, y)) == m_defaultOpacity;
    case BottomEdge:
        return y == bufHeight - 1 || m_cs->opacityU8(storage.pickPixel(x, y + 1)) == m_defaultOpacity;
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
             isOutlineEdge (storage, test_edge, test_col, test_row, width, height)) \
        {                                                                                           \
            *row = test_row;                                                                        \
            *col = test_col;                                                                        \
            *edge = test_edge;                                                                      \
            break;                                                                                  \
        }                                                                                       \
    }

template <class StorageStrategy>
void KisOutlineGenerator::nextOutlineEdge(StorageStrategy &storage, EdgeType *edge, qint32 *row, qint32 *col, qint32 width, qint32 height)
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

void KisOutlineGenerator::appendCoordinate(QPolygon * path, int x, int y, EdgeType edge)
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

void KisOutlineGenerator::setSimpleOutline(bool simple)
{
    m_simple = simple;
}
