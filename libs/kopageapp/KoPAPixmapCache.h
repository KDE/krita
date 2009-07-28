/* This file is part of the KDE project
   Copyright (C) 2009 Thorsten Zachmann <zachmann@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
*/

#ifndef KOPAPIXMAPCACHE_H
#define KOPAPIXMAPCACHE_H

#include <QMap>

class QString;
class QSize;
class QPixmap;

/**
 * This class is a cache for pixmaps which will be cached for different sizes 
 * of the same pixmap If a key is removed from the cache all cached sizes will
 * be removed from the cache.
 *
 * The API is similar to QPixmpaCache. The only addition is that you need to 
 * specify the size of the pixmap when you search it.
 *
 * The implementation uses QPixmapCache.
 *
 * This class is a singleton.
 */
class KoPAPixmapCache
{
public:
    class Singleton;

    /**
     * Get the pixmap cache singleton
     */
    static KoPAPixmapCache * instance();

    ~KoPAPixmapCache();

    /**
     * Returns the cache limit (in kilobytes)
     */
    int cacheLimit();

    /**
     * Removes all pixmaps from the cache.
     *
     * @param all If true QPixmpaCache::clear will be called. 
     *            If false only the pixmaps which were added via this object
     *            will be removed
     */
    void clear( bool all = true );

    /**
     * Looks for a cached pixmap associated with the key in the cache. 
     *
     * If the pixmap is found, the function sets pm to that pixmap and returns true; 
     * otherwise it leaves pm alone and returns false.
     *
     * @param key the key of the pixmap 
     * @param size the size you want to have the pixmap
     * @param pm the pixmap
     */
    bool find( const QString & key, const QSize & size, QPixmap & pm );

    /**
     * Insert a copy of the pixmap into the cache.
     *
     * The size is taken from the pixmap.
     */
    bool insert( const QString & key, const QPixmap & pm );

    /**
     * Remove all pixmaps associated with key from the cache
     */
    void remove( const QString & key );

    /**
     * Sets the cache limit to n kilobytes
     */
    void setCacheLimit( int n );

private:
    KoPAPixmapCache();
    KoPAPixmapCache( const KoPAPixmapCache & );
    KoPAPixmapCache operator=( const KoPAPixmapCache & );

    QString generateKey( const QString &key, const QSize & size );
    QMap<QString, QList<QSize> > m_keySize;
};

#endif /* KOPAPIXMAPCACHE_H */
