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
     */
    typedef std::function<void(quint8* outOpacityPtr, const QRect& rect)> FillOpacityFunc;

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

    void loadOpacityTiles(const QRect& tileRect);
    void loadDistanceRect(const QRect& rect, int guardBand);
    quint16 lazyDistance(int x, int y);

    // Templates are used to generate optimized versions of the same function
    // (i.e., the if conditions can be removed at compilation time).

    template<bool BoundsCheck> void gapDistanceSearch(int x, int y, CoordinateTransform op);
    template<bool BoundsCheck> void updateDistance(const QPoint& p, quint16 newDistance);
    template<bool BoundsCheck> ALWAYS_INLINE bool isOpaque(int x, int y) const;
    template<bool BoundsCheck> ALWAYS_INLINE bool isOpaque(const QPoint& p) const;

    ALWAYS_INLINE bool isDistanceAvailable(int x, int y) const
    {
        return m_distanceTiles[x / TileSize + (y / TileSize) * m_numTiles.width()];
    }

    ALWAYS_INLINE quint16* distancePtr(int x, int y)
    {
        return m_distancePtr + y * m_size.width() + x;
    }

    const int m_gapSize;                      ///< Gap size in pixels for this map
    const QSize m_size;                       ///< Size in pixels of the opacity/gap map
    const QSize m_numTiles;                   ///< Map size in tiles
    const FillOpacityFunc m_fillOpacityFunc;  ///< A callback to get the opacity data from the fill class

    std::vector<bool> m_opacityTiles;         ///< Keeps track of the loaded opacity tiles
    std::vector<bool> m_distanceTiles;        ///< Keeps track of the loaded distance tiles
    quint16* m_distancePtr;                   ///< The buffer containing the distance data
    quint8* m_opacityPtr;                     ///< The buffer containing the opacity data
};

#endif /* __KIS_GAP_MAP_H */
