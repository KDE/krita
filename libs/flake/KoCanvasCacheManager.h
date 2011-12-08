/* This file is part of the KDE project
 * Copyright (C) 2011 Boudewijn Rempt <boud@kogmbh.com>
 * Copyright (C) 2011 Marijn Kruisselbrink <mkruisselbrink@kde.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */
#ifndef KWPAGECACHEMANAGER_H
#define KWPAGECACHEMANAGER_H

#include <QCache>
#include <QRectF>
#include <QImage>
#include <QQueue>
#include <QSize>
#include <QObject>

static const int MAX_TILE_SIZE = 1024;

template <class T>
class KoCanvasCacheManager;

/**
 * A tile-based cache for a single page that can contain flake shapes.
 */
class KoCanvasCache {

public:

    /// create a new pagecache object with a new QImage
    KoCanvasCache(int w, int h)
        : m_tilesx(1), m_tilesy(1), m_size(w, h), allExposed(true)
    {
        if (w > MAX_TILE_SIZE || h > MAX_TILE_SIZE) {
            m_tilesx = w / MAX_TILE_SIZE;
            if (w % MAX_TILE_SIZE != 0) m_tilesx++;
            m_tilesy = h / MAX_TILE_SIZE;
            if (h % MAX_TILE_SIZE != 0) m_tilesy++;

            for (int x = 0; x < m_tilesx; x++) {
                for (int y = 0; y < m_tilesy; y++) {
                    int tilew = qMin(MAX_TILE_SIZE, w - x * MAX_TILE_SIZE);
                    int tileh = qMin(MAX_TILE_SIZE, h - y * MAX_TILE_SIZE);
                    cache.push_back(QImage(tilew, tileh, QImage::Format_RGB16));
                }
            }
        } else {
            cache.push_back(QImage(w, h, QImage::Format_RGB16));
        }
    }

    ~KoCanvasCache()
    {
    }

    QList<QImage> cache;
    int m_tilesx, m_tilesy;
    QSize m_size;
    // List of logical exposed rects in view coordinates
    // These are the rects that are queued for updating, not
    // the rects that have already been painted.
    QVector<QRect> exposed;
    // true if the whole page should be repainted
    bool allExposed;
};

/**
 * A generic cache manager that can handle tile-based caches for flake-based documents.
 */
template <class T>
class KoCanvasCacheManager {

public:

    KoCanvasCacheManager(int cacheSize)
        : m_cache(cacheSize)
    {
    }

    ~KoCanvasCacheManager()
    {
        clear();
    }


    KoCanvasCache *take(const T page)
    {
        KoCanvasCache *cache = 0;
        if (m_cache.contains(page)) {
            cache = m_cache.take(page);
        }
        return cache;
    }


    void insert(const T page, KoCanvasCache *cache)
    {
        QSize size = cache->m_size;
        // make sure always at least two pages can be cached
        m_cache.insert(page, cache, qMin(m_cache.maxCost() / 2, size.width() * size.height()));
    }

    KoCanvasCache *cache(QSize size)
    {
        KoCanvasCache *cache = 0;
        if (!cache){
            cache = new KoCanvasCache(size.width(), size.height());
        }
        return cache;
    }

    void clear()
    {
        m_cache.clear();
    }

private:
    QCache<T, KoCanvasCache> m_cache;
    friend class KoCanvasCache;
};

#endif
