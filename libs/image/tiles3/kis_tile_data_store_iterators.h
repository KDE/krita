/*
 *  SPDX-FileCopyrightText: 2010 Dmitry Kazakov <dimula73@gmail.com>
 *  SPDX-FileCopyrightText: 2018 Andrey Kamakin <a.kamakin@icloud.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
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

