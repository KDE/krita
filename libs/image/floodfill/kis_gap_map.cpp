/*
 *  SPDX-FileCopyrightText: 2024 Maciej Jesionowski <yavnrh@gmail.com>
 *  SPDX-FileCopyrightText: 2018 by the MyPaint Development Team.
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

/*
 * Attribution and license notice.
 *
 * The gap distance calculation method is adapted from MyPaint source code,
 * originally implemented by Jesper Lloyd with the following copyright:
 *
 * Copyright (C) 2018 by the MyPaint Development Team.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#include "kis_gap_map.h"
#include <qglobal.h>
#include <QtMath>
#include <QMutex>
#include <QMutexLocker>
#include <vector>

#if KIS_GAP_MAP_MEASURE_ELAPSED_TIME
#include <QElapsedTimer>
#endif

// Asserts are disabled by default in performance-critical code.
#define DEBUG_LOGGING_AND_ASSERTS 0

namespace
{

 /** A persistent, global cache of "memory blocks" that gap filling can reuse.
  *  It will try to use the same block (resized, if needed), unless multiple
  *  allocations are needed at the same time, which should only happen if
  *  Krita has more than one document opened and filled at the same time.
  *
  *  TODO: Is there a similar existing cache that could be used for this?
  */
class BufferCache
{
public:
    static BufferCache* instance();

    /** Get a block of memory from the cache.
     *  Will allocate, if no suitable block is available.
     */
    void* acquire(std::size_t size);

    /** Return the block to the cache.
     *  blockPtr MUST be a pointer previously returned by acquire(), or a nullptr.
     */
    void release(void* dataPtr);

private:
    struct Buffer
    {
        std::vector<quint8> data;
        bool available;

        explicit Buffer(std::size_t size) : data(size), available(false) {}
    };

    using BufferSP = std::shared_ptr<Buffer>;

    std::vector<BufferSP> m_buffers;
    QMutex m_mutex;
};

Q_GLOBAL_STATIC(BufferCache, s_bufferCache);

BufferCache* BufferCache::instance()
{
    return s_bufferCache;
}

void* BufferCache::acquire(std::size_t size)
{
    const QMutexLocker lock(&m_mutex);

    // Try to fetch an existing buffer.
    auto availableBufferIter = std::find_if(
        m_buffers.begin(),
        m_buffers.end(),
        [](const auto& bufferSP){ return bufferSP->available; }
    );

    if (availableBufferIter != m_buffers.end()) {
        auto& buffer = **availableBufferIter;
        buffer.available = false;
        buffer.data.resize(size);

        return buffer.data.data();
    }

    // We need to create a new buffer.
    m_buffers.emplace_back(
        std::make_shared<Buffer>(size));

    return m_buffers.back()->data.data();
}

void BufferCache::release(void* dataPtr)
{
    if (dataPtr == nullptr) {
        return;
    }

    const QMutexLocker lock(&m_mutex);

    // Only keep one buffer in the cache.
    // It's very rare that there are two or more fill operations
    // executed at the same time.

    BufferSP releasedBuffer;
    auto it = m_buffers.begin();

    while (it != m_buffers.end()) {
        if ((*it)->available) {
            // Unused, free it.
            it = m_buffers.erase(it);
        } else if ((*it)->data.data() == dataPtr) {
            // Finish iterating first to not delete it right away.
            releasedBuffer = *it;
            ++it;
        } else {
            // Still in use.
            ++it;
        }
    }

    // Return to the pool.
    if (releasedBuffer) {
        releasedBuffer->available = true;
    }

#if 0
    // Very unlikely to happen, can be enabled during development.
    KIS_SAFE_ASSERT_RECOVER_NOOP(releasedBuffer &&
                                 "FATAL: KisGapMap cache released pointer invalid");
#endif
}


// The following four transformation functions are used to parametrize the distance search algorithm.
// All used together help cover the four octants (a 1/8th circular sector) of a half-circle covering a part of the map.

ALWAYS_INLINE QPoint TransformNone(int x, int y, int xOffset, int yOffset)
{
    return {x + xOffset, y + yOffset};
}

ALWAYS_INLINE QPoint TransformRotateClockwiseMirrorHorizontally(int x, int y, int xOffset, int yOffset)
{
    return {x - yOffset, y - xOffset};
}

ALWAYS_INLINE QPoint TransformRotateClockwise(int x, int y, int xOffset, int yOffset)
{
    return {x - yOffset, y + xOffset};
}

ALWAYS_INLINE QPoint TransformMirrorHorizontally(int x, int y, int xOffset, int yOffset)
{
    return {x + xOffset, y - yOffset};
}

} // anonymous namespace

template<bool BoundsCheck>
bool KisGapMap::isOpaque(int x, int y) const
{
#if DEBUG_LOGGING_AND_ASSERTS
    const TileFlags flags = *tileFlagsPtr(x / TileSize, y / TileSize);
    KIS_SAFE_ASSERT_RECOVER((flags & TILE_OPACITY_LOADED) != 0) {
        qDebug() << "ERROR: opacity at (" << x << "," << y << ") not loaded";
        return false;
    }
#endif
    if (BoundsCheck) {
        if ((x >= 0) && (x < m_size.width()) && (y >= 0) && (y < m_size.height())) {
            return *(m_opacityPtr + y * m_size.width() + x) == 0;
        } else {
            return false;
        }
    } else {
        return *(m_opacityPtr + y * m_size.width() + x) == 0;
    }
}

template<bool BoundsCheck>
bool KisGapMap::isOpaque(const QPoint& p) const
{
    return isOpaque<BoundsCheck>(p.x(), p.y());
}

KisGapMap::KisGapMap(int gapSize,
                     const QRect& mapBounds,
                     const FillOpacityFunc& fillOpacityFunc)
    :
    m_gapSize(gapSize),
    m_size(mapBounds.size()),
    m_numTiles(qCeil(static_cast<float>(m_size.width()) / TileSize),
               qCeil(static_cast<float>(m_size.height()) / TileSize)),
    m_fillOpacityFunc(fillOpacityFunc),
    m_distancePtr(nullptr),
    m_opacityPtr(nullptr)
{
    // Ensure the scanline fill uses the same coordinates.
    KIS_ASSERT((mapBounds.x() == 0) && (mapBounds.y() == 0) &&
               "Gap closing fill assumes x and y start at coordinate (0, 0)");

    // Request cached memory. Use distancePtr as the owning "handle".
    const quint32 numPixels = m_size.width() * m_size.height();
    const quint32 numTiles = m_numTiles.width() * m_numTiles.height();
    const std::size_t distanceMemSize = sizeof(quint16) * numPixels;
    const std::size_t opacityMemSize = sizeof(quint8) * numPixels;
    const std::size_t tileFlagsMemSize = sizeof(TileFlags) * numTiles;

    // We are placing the distance map first, as it should have at least a quint16 alignment.
    m_distancePtr = static_cast<quint16*>(BufferCache::instance()->acquire(distanceMemSize +
                                                                           opacityMemSize +
                                                                           tileFlagsMemSize));
    m_opacityPtr = reinterpret_cast<quint8*>(m_distancePtr + numPixels);
    m_tileFlags  = m_opacityPtr + numPixels;

    // Tile loading is deferred. All flags are cleared in the initial state.
    memset(m_tileFlags, 0, tileFlagsMemSize);
}

KisGapMap::~KisGapMap()
{
    BufferCache::instance()->release(m_distancePtr);
}

void KisGapMap::loadOpacityTiles(const QRect& tileRect)
{
#if KIS_GAP_MAP_MEASURE_ELAPSED_TIME
    QElapsedTimer timer;
    timer.start();
#endif

    for (int ty = tileRect.top(); ty <= tileRect.bottom(); ++ty) {
        for (int tx = tileRect.left(); tx <= tileRect.right(); ++tx) {
            TileFlags* const pFlags = tileFlagsPtr(tx, ty);
            if ((*pFlags & TILE_OPACITY_LOADED) == 0) {
                // Resize and clamp to image bounds.
                QRect rect(tx * TileSize, ty * TileSize, TileSize, TileSize);
                rect.setRight(qMin(rect.right(), m_size.width() - 1));
                rect.setBottom(qMin(rect.bottom(), m_size.height() - 1));

#if DEBUG_LOGGING_AND_ASSERTS
                qDebug() << "loadOpacityTiles()" << rect;
#endif
                const bool hasOpaquePixels = m_fillOpacityFunc(m_opacityPtr, rect);

                // This tile is now loaded.
                *pFlags |= TILE_OPACITY_LOADED | (hasOpaquePixels ? TILE_HAS_OPAQUE_PIXELS : 0);
            }
        }
    }

#if KIS_GAP_MAP_MEASURE_ELAPSED_TIME
    m_opacityElapsedNanos += timer.nsecsElapsed();
#endif
}

/** This is a part of loadDistanceTile() implementation. */
void KisGapMap::distanceSearchRowInnerLoop(bool boundsCheck, int y, int x1, int x2)
{
    if (boundsCheck) {
        for (int x = x1; x <= x2; ++x) {
            if (isOpaque<true>(x, y)) {
                gapDistanceSearch<true>(x, y, TransformNone);
                gapDistanceSearch<true>(x, y, TransformRotateClockwiseMirrorHorizontally);
                gapDistanceSearch<true>(x, y, TransformRotateClockwise);
                gapDistanceSearch<true>(x, y, TransformMirrorHorizontally);
            }
        }
    } else {
        for (int x = x1; x <= x2; ++x) {
            if (isOpaque<false>(x, y)) {
                gapDistanceSearch<false>(x, y, TransformNone);
                gapDistanceSearch<false>(x, y, TransformRotateClockwiseMirrorHorizontally);
                gapDistanceSearch<false>(x, y, TransformRotateClockwise);
                gapDistanceSearch<false>(x, y, TransformMirrorHorizontally);
            }
        }
    }
}

/** Calculate the gap distance data in the specified rect.
 *  NOTE: Opacity data must have been loaded already.
 *
 *  @see gapDistanceSearch() for an explanation of which distance map pixels
 *  are be affected by this operation.
 *
 *  If the rect is smaller than the whole fill region, then the guardBand is needed
 *  and must be at least equal to the gap size. We need to do calculations in
 *  a larger region in order to compute correct distances within the requested rect.
 */
void KisGapMap::loadDistanceTile(const QPoint& tile, const QRect& nearbyTilesRect, int guardBand)
{
#if KIS_GAP_MAP_MEASURE_ELAPSED_TIME
    QElapsedTimer timer;
    timer.start();
#endif

    // The area of the image covered only by this tile.
    QRect rect(tile.x() * TileSize, tile.y() * TileSize, TileSize, TileSize);
    rect.setRight(qMin(rect.right(), m_size.width() - 1));
    rect.setBottom(qMin(rect.bottom(), m_size.height() - 1));

    // We only care about the distance data inside the rect, so we can leave the
    // distance pixels in the guard band uninitialized. They will be overwritten anyway
    // when their respective tile(s) are loaded.

    if (rect.width() == m_size.width()) {
        std::fill_n(m_distancePtr + rect.top() * m_size.width(), m_size.width() * rect.height(), DISTANCE_INFINITE);
    } else {
        quint16* pData = m_distancePtr + rect.left() + rect.top() * m_size.width();
        for (int y = rect.top(); y <= rect.bottom(); ++y) {
            std::fill_n(pData, rect.width(), DISTANCE_INFINITE);
            pData += m_size.width();
        }
    }

    TileFlags* const pFlags = tileFlagsPtr(tile.x(), tile.y());

    // This tile is now considered loaded.
    *pFlags |= TILE_DISTANCE_LOADED;

    // Optimization: If a tile is completely transparent (TILE_HAS_OPAQUE_PIXELS == 0), then
    // we can skip the distance calculation for it. Unfortunately, with the guard bands we need
    // to check the flags of the neighboring tiles as well.

    const bool tileOpaque           = (*pFlags & TILE_HAS_OPAQUE_PIXELS) != 0;
    const bool tileOpaqueLeft       = (nearbyTilesRect.left()   == tile.x()) ?                                           false : (*tileFlagsPtr(tile.x() - 1, tile.y())     & TILE_HAS_OPAQUE_PIXELS) != 0;
    const bool tileOpaqueTopLeft    = (nearbyTilesRect.left()   == tile.x()) || (nearbyTilesRect.top()    == tile.y()) ? false : (*tileFlagsPtr(tile.x() - 1, tile.y() - 1) & TILE_HAS_OPAQUE_PIXELS) != 0;
    const bool tileOpaqueBottomLeft = (nearbyTilesRect.left()   == tile.x()) || (nearbyTilesRect.bottom() == tile.y()) ? false : (*tileFlagsPtr(tile.x() - 1, tile.y() + 1) & TILE_HAS_OPAQUE_PIXELS) != 0;
    const bool tileOpaqueTop        = (nearbyTilesRect.top()    == tile.y()) ?                                           false : (*tileFlagsPtr(tile.x(),     tile.y() - 1) & TILE_HAS_OPAQUE_PIXELS) != 0;
    const bool tileOpaqueBottom     = (nearbyTilesRect.bottom() == tile.y()) ?                                           false : (*tileFlagsPtr(tile.x(),     tile.y() + 1) & TILE_HAS_OPAQUE_PIXELS) != 0;

    if (! (tileOpaqueTopLeft || tileOpaqueTop || tileOpaqueLeft || tileOpaque || tileOpaqueBottomLeft || tileOpaqueBottom)) {
#if KIS_GAP_MAP_MEASURE_ELAPSED_TIME
        m_distanceElapsedNanos += timer.nsecsElapsed();
#endif
        // This tile as well as its surroundings are transparent, we can finally exit.
        return;
    }

    // Compromise: At the tile size 64 px and the gap size 32 px, the guard band must be
    // 31 px at most, because opacity is sampled in gap size + 1 radius, which could be two tiles
    // away and that tile might not have been loaded yet. To avoid loading one row of that tile,
    // we can clamp the guard band to 31. The error intruduced by it should not be noticeable.

    const int guardBandVertical = qMin(guardBand, 31);
    const int y1       = tileOpaqueTopLeft    || tileOpaqueTop    ? qMax(0, rect.top() - guardBandVertical) : rect.top();
    const int y2       = tileOpaqueBottomLeft || tileOpaqueBottom ? qMin(rect.bottom() + guardBandVertical, m_size.height() - 1) : rect.bottom();
    const int x1Top    = tileOpaqueTopLeft                        ? qMax(0, rect.left() - guardBand) : rect.left();
    const int x1Middle = tileOpaqueLeft                           ? qMax(0, rect.left() - guardBand) : rect.left();
    const int x1Bottom = tileOpaqueBottomLeft                     ? qMax(0, rect.left() - guardBand) : rect.left();
    const int x2Top    = tileOpaqueTop                            ? rect.right() : rect.left() - 1;
    const int x2Middle = tileOpaque                               ? rect.right() : rect.left() - 1;
    const int x2Bottom = tileOpaqueBottom                         ? rect.right() : rect.left() - 1;

    // Apply conservative bounds checking. +1 for opacity.
    const bool boundsCheck =
        (rect.right() + (m_gapSize + 1) >= m_size.width()) ||  // no risk of accessing x<0
        (y1 - (m_gapSize + 1) < 0) || (y2 + (m_gapSize + 1) >= m_size.height());

    // Process the tile and its neighborhood in three passes:
    // Top (the top guard bands)
    for (int y = y1; y <= rect.top() - 1; ++y) {
        distanceSearchRowInnerLoop(boundsCheck, y, x1Top, x2Top);
    }
    // Middle (the left guard band and the tile)
    for (int y = rect.top(); y <= rect.bottom(); ++y) {
        distanceSearchRowInnerLoop(boundsCheck, y, x1Middle, x2Middle);
    }
    // Bottom (the bottom guard bands)
    for (int y = rect.bottom() + 1; y <= y2; ++y) {
        distanceSearchRowInnerLoop(boundsCheck, y, x1Bottom, x2Bottom);
    }

#if KIS_GAP_MAP_MEASURE_ELAPSED_TIME
    m_distanceElapsedNanos += timer.nsecsElapsed();
#endif
}

/**
 * Update the distance map in an octant (a 45-degree sector of a half-circle) originating from the (x, y) point.
 *
 * Important: The radius of the right half-circle is equal to the (gap size) for distance map,
 * and the (gap size + 1) for the opacity map.
 *
 * Which octant is used depends on the transform function. With 0 degrees being the -y axis (up):
 *    0-45 deg - TransformNone
 *   45-90 deg - TransformRotateClockwiseMirrorHorizontally
 *  90-135 deg - TransformRotateClockwise
 * 135-180 deg - TransformMirrorHorizontally
 *
 * This function is a bit tricky to reason about:
 *
 * - Calling it for a point at (x, y) will modify the RIGHT half-circle of the distance map, excluding the point.
 * - Conversely, to fully determine the distance at any specific point (mx, my) in the map,
 *   this function must be called for all points in the LEFT half-circle, that is the rect
 *   (mx - gap, my - gap, gap + 1, gap + 1).
 * - Lastly, only some points in the half circle will be modified, it depends on the opacity checks.
 */
template<bool BoundsCheck>
void KisGapMap::gapDistanceSearch(int x, int y, CoordinateTransform op)
{
    if (isOpaque<BoundsCheck>(op(x, y, 0, -1)) ||
        isOpaque<BoundsCheck>(op(x, y, 1, -1))) {
        return;
    }

    for (int yoffs = 2; yoffs < m_gapSize + 2; ++yoffs) {
        const int yDistanceSq = (yoffs - 1) * (yoffs - 1);

        for (int xoffs = 0; xoffs <= yoffs; ++xoffs) {
            const int offsetDistance = yDistanceSq + xoffs * xoffs;

            if (offsetDistance >= 1 + m_gapSize * m_gapSize) {
                break;
            }

            if (isOpaque<BoundsCheck>(op(x, y, xoffs, -yoffs))) {
                const float dx = static_cast<float>(xoffs) / (yoffs - 1);
                float tx = 0;
                int cx = 0;

                for (int cy = 1; cy < yoffs; ++cy) {
                    updateDistance<BoundsCheck>(op(x, y, cx, -cy), offsetDistance);

                    tx += dx;
                    if (static_cast<int>(tx) > cx) {
                        cx++;
                        updateDistance<BoundsCheck>(op(x, y, cx, -cy), offsetDistance);
                    }

                    updateDistance<BoundsCheck>(op(x, y, cx + 1, -cy), offsetDistance);
                }
            }
        }
    }
}

template<bool BoundsCheck>
void KisGapMap::updateDistance(const QPoint& p, quint16 newDistance)
{
    if (BoundsCheck) {
        if ((p.x() < 0) || (p.x() >= m_size.width()) || (p.y() < 0) || (p.y() >= m_size.height())) {
            return;
        }
    }
    quint16* const dataPtr = distancePtr(p.x(), p.y());
    if (*dataPtr > newDistance) {
        *dataPtr = newDistance;
    }
}

/** Load the required tiles and return pixel's distance data. */
quint16 KisGapMap::lazyDistance(int x, int y)
{
#if DEBUG_LOGGING_AND_ASSERTS
    qDebug() << "lazyDistance() at (" << x << "," << y << ")";
#endif

    const int tx = x / TileSize;
    const int ty = y / TileSize;

    // Clamped tile neighborhood.
    const QPoint topLeft(qMax(0, tx - 1),
                         qMax(0, ty - 1));
    const QPoint bottomRight(qMin(tx + 1, m_numTiles.width() - 1),
                             qMin(ty + 1, m_numTiles.height() - 1));
    const QRect nearbyTiles = QRect(topLeft, bottomRight);

    // For opacity data, we always load all the adjacent tiles (up to 9 tiles in total).
    loadOpacityTiles(nearbyTiles);

    // For distance data, we always load a single tile.
    loadDistanceTile(QPoint(tx, ty), nearbyTiles, m_gapSize);

    // The data is now ready to be returned.
    return *distancePtr(x, y);
}
