/*
 *  Copyright (c) 2010 Dmitry Kazakov <dimula73@gmail.com>
 *  Copyright (c) 2018 Andrey Kamakin <a.kamakin@icloud.com>
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

#ifndef KIS_TILE_DATA_STORE_ITERATORS_H_
#define KIS_TILE_DATA_STORE_ITERATORS_H_

#include "kis_tile_data.h"
#include "kis_debug.h"

/**
 * KisTileDataStoreIterator,
 * KisTileDataStoreReverseIterator,
 * KisTileDataStoreClockIterator
 * - are general iterators for the contents of KisTileDataStore.
 * The store starts holding a lock when returns one of such
 * iterators, so no one will be able to change the list while
 * you are iterating.
 *
 * But be careful! You can't change the list while iterating either,
 * because it can invalidate the iterator. This is a general rule.
 */


class KisTileDataStoreIterator
{
public:
    KisTileDataStoreIterator(ConcurrentMap<int, KisTileData*> &map, KisTileDataStore *store)
        : m_map(map),
          m_store(store)
    {
        m_iterator.setMap(m_map);
    }

    inline KisTileData* peekNext()
    {
        return m_iterator.getValue();
    }

    inline KisTileData* next()
    {
        KisTileData *current = m_iterator.getValue();
        m_iterator.next();
        return current;
    }

    inline bool hasNext() const
    {
        return m_iterator.isValid();
    }

    inline bool trySwapOut(KisTileData *td)
    {
        if (td == m_iterator.getValue()) {
            m_iterator.next();
        }

        return m_store->trySwapTileData(td);
    }

private:
    ConcurrentMap<int, KisTileData*> &m_map;
    ConcurrentMap<int, KisTileData*>::Iterator m_iterator;
    KisTileDataStore *m_store;
};

class KisTileDataStoreReverseIterator : public KisTileDataStoreIterator
{
public:
    KisTileDataStoreReverseIterator(ConcurrentMap<int, KisTileData*> &map, KisTileDataStore *store)
        : KisTileDataStoreIterator(map, store)
    {
    }
};

class KisTileDataStoreClockIterator
{
public:
    KisTileDataStoreClockIterator(ConcurrentMap<int, KisTileData*> &map,
                                  int startIndex,
                                  KisTileDataStore *store)
        : m_map(map),
          m_store(store)
    {
        m_iterator.setMap(m_map);
        m_finalPosition = m_iterator.getValue()->m_tileNumber;
        m_startItem = m_map.get(startIndex);

        if (m_iterator.getValue() == m_startItem || !m_startItem) {
            m_startItem = 0;
            m_endReached = true;
        } else {
            while (m_iterator.getValue() != m_startItem) {
                m_iterator.next();
            }
            m_endReached = false;
        }
    }

    inline KisTileData* peekNext()
    {
        if (!m_iterator.isValid()) {
            m_iterator.setMap(m_map);
            m_endReached = true;
        }

        return m_iterator.getValue();
    }

    inline KisTileData* next()
    {
        if (!m_iterator.isValid()) {
            m_iterator.setMap(m_map);
            m_endReached = true;
        }

        KisTileData *current = m_iterator.getValue();
        m_iterator.next();
        return current;
    }

    inline bool hasNext() const
    {
        return !(m_endReached && m_iterator.getValue() == m_startItem);
    }

    inline bool trySwapOut(KisTileData *td)
    {
        if (td == m_iterator.getValue()) {
            m_iterator.next();
        }

        return m_store->trySwapTileData(td);
    }

private:
    friend class KisTileDataStore;
    inline int getFinalPosition()
    {
        if (!m_iterator.isValid()) {
            return m_finalPosition;
        }

        return m_iterator.getValue()->m_tileNumber;
    }

private:
    ConcurrentMap<int, KisTileData*> &m_map;
    ConcurrentMap<int, KisTileData*>::Iterator m_iterator;
    KisTileData *m_startItem;
    bool m_endReached;
    KisTileDataStore *m_store;
    int m_finalPosition;
};

#endif /* KIS_TILE_DATA_STORE_ITERATORS_H_ */

