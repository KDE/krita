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

#define KIS_GAP_MAP_MEASURE_ELAPSED_TIME 0

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
     *  @param outOpacityPtr the memory location where opacity data should be written back;
     *         It points to fill region's (0, 0) coordinate and one row is equal to
     *         fill region's width.
     *  @param rect the bounds within the fill region (image) that are requested.
     *  @return true, if at least one pixel within the tile is opaque
     */
    typedef std::function<bool(quint8* outOpacityPtr, const QRect& rect)> FillOpacityFunc;

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

    /** May free some resources allocated by the constructor.
     *  Some data is cached for reuse. */
    ~KisGapMap();

    /** Query the gap distance at a pixel.
     *  (x, y) are the filled region's coordinates, always starting at (0, 0).
     *
     *  Important: This function is not thread-safe.
     */
    ALWAYS_INLINE quint16 distance(int x, int y)
    {
        if (isDistanceAvailable(x, y)) {
            return *distancePtr(x, y);
        } else {
            return lazyDistance(x, y);
        }
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

    typedef QPoint (*CoordinateTransform)(int x, int y, int xOffset, int yOffset);

    typedef quint8 TileFlags;
    enum TileFlagBits
    {
        TILE_DISTANCE_LOADED     = 0x1,      ///< Distance data is available
        TILE_OPACITY_LOADED      = 0x2,      ///< Opacity data is available
        TILE_HAS_OPAQUE_PIXELS   = 0x4,      ///< Some pixels of the loaded tile are opaque
    };

    void loadOpacityTiles(const QRect& tileRect);
    void loadDistanceTile(const QPoint& tile, const QRect& nearbyTilesRect, int guardBand);
    void distanceSearchRowInnerLoop(bool boundsCheck, int y, int x1, int x2);
    quint16 lazyDistance(int x, int y);

    // Templates are used to generate optimized versions of the same function
    // (i.e., the if conditions can be removed at compilation time).

    template<bool BoundsCheck> void gapDistanceSearch(int x, int y, CoordinateTransform op);
    template<bool BoundsCheck> void updateDistance(const QPoint& p, quint16 newDistance);
    template<bool BoundsCheck> ALWAYS_INLINE bool isOpaque(int x, int y) const;
    template<bool BoundsCheck> ALWAYS_INLINE bool isOpaque(const QPoint& p) const;

    ALWAYS_INLINE bool isDistanceAvailable(int x, int y) const
    {
        return (*tileFlagsPtr(x / TileSize, y / TileSize) & TILE_DISTANCE_LOADED) != 0;
    }

    ALWAYS_INLINE quint16* distancePtr(int x, int y) const
    {
        return m_distancePtr + x + y * m_size.width();
    }

    ALWAYS_INLINE TileFlags* tileFlagsPtr(int tileX, int tileY) const
    {
        return m_tileFlags + tileX + tileY * m_numTiles.width();
    }

    const int m_gapSize;                      ///< Gap size in pixels for this map
    const QSize m_size;                       ///< Size in pixels of the opacity/gap map
    const QSize m_numTiles;                   ///< Map size in tiles
    const FillOpacityFunc m_fillOpacityFunc;  ///< A callback to get the opacity data from the fill class

    quint16* m_distancePtr;                   ///< The buffer containing the distance data
    quint8* m_opacityPtr;                     ///< The buffer containing the opacity data
    TileFlags* m_tileFlags;                   ///< The buffer containing the tile metadata
};

#endif /* __KIS_GAP_MAP_H */
