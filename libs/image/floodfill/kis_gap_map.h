/*
 *  SPDX-FileCopyrightText: 2024 Maciej Jesionowski <yavnrh@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef __KIS_GAP_MAP_H
#define __KIS_GAP_MAP_H

#include <kis_types.h>
#include <KoAlwaysInline.h>
#include <kis_shared.h>
#include <QRect>
#include <kis_paint_device.h>
#include <kis_random_accessor_ng.h>

#define KIS_GAP_MAP_MEASURE_ELAPSED_TIME 0

// Asserts are disabled by default in performance-critical code.
#define KIS_GAP_MAP_DEBUG_LOGGING_AND_ASSERTS 0

class KisTileOptimizedAccessor
{
public:
    KisTileOptimizedAccessor(KisPaintDeviceSP& paintDevice)
        : m_pixelSize(paintDevice->pixelSize())
        , m_accessor(paintDevice->createRandomAccessorNG())
        , m_tile(-1, -1)
        , m_tileRawData(nullptr)
    {
      // We start in an invalid state ((-1,-1) tile), so that the data pointer is fetched
      // after the paint device has been set up (e.g. default color and fill).
    }

    quint8* tileRawData(int tileX, int tileY)
    {
        if ((m_tile.x() != tileX) || (m_tile.y() != tileY)) {
            m_accessor->moveTo(tileX * TileSize, tileY * TileSize);
            m_tileRawData = m_accessor->rawData();

            m_tile.setX(tileX);
            m_tile.setY(tileY);

#if KIS_GAP_MAP_DEBUG_LOGGING_AND_ASSERTS
            KIS_ASSERT(m_accessor->numContiguousRows(tileY * TileSize) == TileSize);
            KIS_ASSERT(m_accessor->numContiguousColumns(tileX * TileSize) == TileSize);
#endif
        }

        return m_tileRawData;
    }

    ALWAYS_INLINE quint8* rawData(int x, int y)
    {
        const int tileX = x / TileSize;
        const int tileY = y / TileSize;
        const int localX = x & (TileSize - 1);
        const int localY = y & (TileSize - 1);

        m_tileRawData = tileRawData(tileX, tileY);

        return m_tileRawData + m_pixelSize * (localX + TileSize * localY);
    }

private:
    static constexpr int TileSize = 64;

    const quint32 m_pixelSize;

    KisRandomAccessorSP m_accessor;
    QPoint m_tile;
    quint8* m_tileRawData;
};

/**
 * Creates a "gap map", which is a pixel map of distances from lineart gaps
 * (discontinuities in opaque lines), that helps detect these gaps and stop
 * a bucket fill before spilling.
 */
class KRITAIMAGE_EXPORT KisGapMap : public KisShared
{
public:
    /** A magic number to express a pixel that is very far from any gaps. */
    static constexpr quint16 DISTANCE_INFINITE = UINT16_MAX;

    /** A callback to request opacity data for pixels in the image.
     *  It can be called at any time distance() function is invoked.
     *
     *  @param devicePtr our underlying paint device, it contains opacity among other data.
     *         For each pixel, the offset of opacity quint8 data is 2 bytes.
     *  @param rect the bounds within the fill region (image) that are requested.
     *  @return true, if at least one pixel within the tile is opaque
     */
    typedef std::function<bool(KisPaintDevice* devicePtr, const QRect& rect)> FillOpacityFunc;

    /** Create a new gap distance map object and prepare it for lazy initialization.
     *  Some memory allocation will happen upfront, but most of the calculations
     *  are deferred until distance() function is called.
     *
     *  @param gapSize maximum size of lineart gap to look for.
     *  @param mapBounds must begin in (0,0) and must have the same size as the filled region.
     *  @param fillOpacityFunc a callback to obtain the opacity of pixels
     */
    KisGapMap(int gapSize,
              const QRect& mapBounds,
              const FillOpacityFunc& fillOpacityFunc);

    /** Query the gap distance at a pixel.
     *  (x, y) are the filled region's coordinates, always starting at (0, 0).
     *
     *  Important: This function is not thread-safe.
     */
    ALWAYS_INLINE quint16 distance(int x, int y)
    {
        if (isDistanceAvailable(x, y)) {
            return dataPtr(x, y)->distance;
        } else {
            return lazyDistance(x, y);
        }
    }

    ALWAYS_INLINE int gapSize() const
    {
        return m_gapSize;
    }

#if KIS_GAP_MAP_MEASURE_ELAPSED_TIME
public:
    quint64 opacityElapsedMillis() const
    {
        return m_opacityElapsedNanos / 1000000ull;
    }

    quint64 distanceElapsedMillis() const
    {
        return m_distanceElapsedNanos / 1000000ull;
    }

private:
    quint64 m_opacityElapsedNanos = 0;
    quint64 m_distanceElapsedNanos = 0;
#endif

private:
    Q_DISABLE_COPY(KisGapMap);

    /** For the purpose of lazy loading, the data is fetched in tile increments. */
    static constexpr int TileSize = 64;

    typedef quint8 TileFlags;
    enum TileFlagBits
    {
        TILE_DISTANCE_LOADED     = 0x1,      ///< Distance data is available
        TILE_OPACITY_LOADED      = 0x2,      ///< Opacity data is available
        TILE_HAS_OPAQUE_PIXELS   = 0x4,      ///< Some pixels of the loaded tile are opaque
    };

    struct Data
    {
        quint16     distance;
        quint8      opacity;
        TileFlags   flags;
    };
    static_assert(sizeof(Data) == sizeof(quint32));

    void loadOpacityTiles(const QRect& tileRect);
    void loadDistanceTile(const QPoint& tile, const QRect& nearbyTilesRect, int guardBand);
    void distanceSearchRowInnerLoop(bool boundsCheck, int y, int x1, int x2);
    quint16 lazyDistance(int x, int y);

    // Templates are used to generate optimized versions of the same function
    // (i.e., the if conditions can be removed at compilation time).

    template<bool BoundsCheck, typename CoordinateTransform>
    void gapDistanceSearch(int x, int y, CoordinateTransform op);

    template<bool BoundsCheck> ALWAYS_INLINE bool isOpaque(int x, int y);
    template<bool BoundsCheck> ALWAYS_INLINE bool isOpaque(const QPoint& p);
    void updateDistance(const QPoint& globalPosition, quint16 newDistance);

    ALWAYS_INLINE bool isDistanceAvailable(int x, int y)
    {
        return (*tileFlagsPtr(x / TileSize, y / TileSize) & TILE_DISTANCE_LOADED) != 0;
    }

    ALWAYS_INLINE Data* dataPtr(int x, int y)
    {
        return reinterpret_cast<Data*>(m_accessor->rawData(x, y));
    }

    ALWAYS_INLINE TileFlags* tileFlagsPtr(int tileX, int tileY)
    {
        return reinterpret_cast<TileFlags*>(
            m_accessor->tileRawData(tileX, tileY) + offsetof(Data, flags));
    }

    const int m_gapSize;                      ///< Gap size in pixels for this map
    const QSize m_size;                       ///< Size in pixels of the opacity/gap map
    const QSize m_numTiles;                   ///< Map size in tiles
    const FillOpacityFunc m_fillOpacityFunc;  ///< A callback to get the opacity data from the fill class

    QPoint m_tilePosition;                    ///< The position of the currently computed tile compared to the whole region
    Data* m_tileDataPtr;                      ///< The pointer to the currently computed tile data

    KisPaintDeviceSP m_deviceSp;                            ///< A 32-bit per pixel paint device that holds the distance and other data
    std::unique_ptr<KisTileOptimizedAccessor> m_accessor;   ///< An accessor for the paint device
};

#endif /* __KIS_GAP_MAP_H */
