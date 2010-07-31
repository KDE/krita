/*
 *  Copyright (c) 2010 Dmitry Kazakov <dimula73@gmail.com>
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

/**
 * KisTileDataStoreIterator,
 * KisTileDataStoreReverseIterator,
 * KisTileDataStoreClockIterator
 * - are general iterators for the contents of KisTileDataStore.
 * The store starts holding a lock when returns one of such
 * iterators, so noone will be able to change the list while
 * you are iterating.
 *
 * But be careful! You can't change the list while iterating either,
 * because it can invalidate the iterator. This is a general rule.
 */


class KisTileDataStoreIterator
{
public:
    KisTileDataStoreIterator(KisTileDataList &list, KisTileDataStore *store)
        : m_list(list),
          m_store(store)
    {
        m_iterator = m_list.begin();
        m_end = m_list.end();
    }

    inline KisTileData* peekNext() {
        return *m_iterator;
    }

    inline KisTileData* next() {
        return *(m_iterator++);
    }

    inline bool hasNext() const {
        return m_iterator != m_end;
    }

    inline bool trySwapOut(KisTileData *td) {
        if(td->m_listIterator == m_iterator)
            m_iterator++;

        return m_store->trySwapTileData(td);
    }

private:
    KisTileDataList &m_list;
    KisTileDataListIterator m_iterator;
    KisTileDataListIterator m_end;
    KisTileDataStore *m_store;
};

class KisTileDataStoreReverseIterator
{
public:
    KisTileDataStoreReverseIterator(KisTileDataList &list, KisTileDataStore *store)
        : m_list(list),
          m_store(store)
    {
        m_iterator = m_list.end();
        m_begin = m_list.begin();
    }

    inline KisTileData* peekNext() {
        return *(m_iterator-1);
    }

    inline KisTileData* next() {
        return *(--m_iterator);
    }

    inline bool hasNext() const {
        return m_iterator != m_begin;
    }

    inline bool trySwapOut(KisTileData *td) {
        if(td->m_listIterator == m_iterator)
            m_iterator++;

        return m_store->trySwapTileData(td);
    }

private:
    KisTileDataList &m_list;
    KisTileDataListIterator m_iterator;
    KisTileDataListIterator m_begin;
    KisTileDataStore *m_store;
};

class KisTileDataStoreClockIterator
{
public:
    KisTileDataStoreClockIterator(KisTileDataListIterator startItem, KisTileDataList &list, KisTileDataStore *store)
        : m_list(list),
          m_store(store)
    {
        m_end = m_list.end();

        if(startItem == m_list.begin() ||
           startItem == m_end) {
            m_iterator = m_list.begin();
            m_startItem = m_end;
            m_endReached = true;
        }
        else  {
            m_startItem = startItem;
            m_iterator = startItem;
            m_endReached = false;
        }
    }

    inline KisTileData* peekNext() {
        if(m_iterator == m_end) {
            m_iterator = m_list.begin();
            m_endReached = true;
        }

        return *m_iterator;
    }

    inline KisTileData* next() {
        if(m_iterator == m_end) {
            m_iterator = m_list.begin();
            m_endReached = true;
        }

        return *(m_iterator++);
    }

    inline bool hasNext() const {
        return !(m_endReached && m_iterator == m_startItem);
    }

    inline bool trySwapOut(KisTileData *td) {
        if(td->m_listIterator == m_iterator)
            m_iterator++;

        return m_store->trySwapTileData(td);
    }

private:
    friend class KisTileDataStore;
    inline KisTileDataListIterator getFinalPosition() {
        return m_iterator;
    }

private:
    KisTileDataList &m_list;
    bool m_endReached;
    KisTileDataListIterator m_iterator;
    KisTileDataListIterator m_startItem;
    KisTileDataListIterator m_end;
    KisTileDataStore *m_store;
};

#endif /* KIS_TILE_DATA_STORE_ITERATORS_H_ */

