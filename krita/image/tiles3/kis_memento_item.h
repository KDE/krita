/*
 *  Copyright (c) 2009 Dmitry Kazakov <dimula73@gmail.com>
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

#ifndef KIS_MEMENTO_ITEM_H_
#define KIS_MEMENTO_ITEM_H_

#include <kis_shared.h>
#include <kis_shared_ptr.h>
#include "kis_tile_data_store.h"
#include "kis_tile.h"


class KisMementoItem;
typedef KisSharedPtr<KisMementoItem> KisMementoItemSP;

class KisMementoItem : public KisShared
{
public:
    enum enumType {
        CHANGED  = 0x0,
        DELETED  = 0x1,
    };

public:
    KisMementoItem()
            : m_tileData(0), m_commitedFlag(false) {
    }

    KisMementoItem(const KisMementoItem& rhs)
            : KisShared(rhs),
            m_tileData(rhs.m_tileData),
            m_commitedFlag(rhs.m_commitedFlag),
            m_type(rhs.m_type),
            m_col(rhs.m_col),
            m_row(rhs.m_row),
            m_next(0),
            m_parent(0) {
        if (m_tileData) {
            if (m_commitedFlag)
                globalTileDataStore.acquireTileData(m_tileData);
            else
                globalTileDataStore.refTileData(m_tileData);
        }
    }

    /**
     * Automatically called by Kis..HashTable. It means that this mementoItem
     * is a root item of parental hierarchy. So m_parent should be NULL.
     */
    KisMementoItem(qint32 col, qint32 row, KisTileData* defaultTileData, KisMementoManager *mm) {
        Q_UNUSED(mm);
        m_tileData = defaultTileData;
        /* Setting counter: m_refCount++ */
        globalTileDataStore.refTileData(m_tileData);
        m_col = col;
        m_row = row;
        m_type = CHANGED;
        m_parent = 0;
        m_commitedFlag = false;
    }
    KisMementoItem(const KisMementoItem &rhs, KisMementoManager *mm) {
        Q_UNUSED(mm);
        m_tileData = rhs.m_tileData;
        /* Setting counter: m_refCount++ */
        globalTileDataStore.refTileData(m_tileData);
        m_col = rhs.m_col;
        m_row = rhs.m_row;
        m_type = CHANGED;
        m_parent = 0;
        m_commitedFlag = false;
    }

    ~KisMementoItem() {
        if (m_tileData) {
            if (m_commitedFlag)
                globalTileDataStore.releaseTileData(m_tileData);
            else
                globalTileDataStore.derefTileData(m_tileData);
        }
    }

    void deleteTile(KisTile* tile, KisTileData* defaultTileData) {
        m_tileData = defaultTileData;
        /* Setting counter: m_refCount++ */
        globalTileDataStore.refTileData(m_tileData);

        m_col = tile->col();
        m_row = tile->row();
        m_type = DELETED;
    }

    void changeTile(KisTile* tile) {
        m_tileData = tile->tileData();
        /* Setting counter: m_refCount++ */
        globalTileDataStore.refTileData(m_tileData);
        m_col = tile->col();
        m_row = tile->row();
        m_type = CHANGED;
    }

    void commit() {
        if (m_commitedFlag) return;
        if (m_tileData) {
            /**
             * Setting counters to proper values:
             * m_refCount++, m_usersCount++;
             * m_refCount--
             */
            globalTileDataStore.acquireTileData(m_tileData);
            globalTileDataStore.derefTileData(m_tileData);
        }
        m_commitedFlag = true;
    }

    inline KisTileSP tile(KisMementoManager *mm) {
        Q_ASSERT(m_tileData);
        return new KisTile(m_col, m_row, m_tileData, mm);
    }

    inline enumType type() {
        return m_type;
    }

    inline void setParent(KisMementoItemSP parent) {
        m_parent = parent;
    }
    inline KisMementoItemSP parent() {
        return m_parent;
    }

    // Stuff for Kis..HashTable
    inline void setNext(KisMementoItemSP next) {
        m_next = next;
    }
    inline KisMementoItemSP next() {
        return m_next;
    }
    inline qint32 col() {
        return m_col;
    }
    inline qint32 row() {
        return m_row;
    }

    void debugPrintInfo() {
        QString s = QString("------\n"
                   "Memento item:\t\t0x%1 (0x%2)\n"
                   "   status:\t(%3,%4) %5%6\n"
                   "   parent:\t0x%7 (0x%8)\n"
                   "   next:\t0x%9 (0x%10)\n")
                .arg((quintptr)this)
                .arg((quintptr)m_tileData)
                .arg(m_col)
                .arg(m_row)
                .arg((m_type == CHANGED) ? 'W' : 'D')
                .arg(m_commitedFlag ? 'C' : '-')
                .arg((quintptr)m_parent.data())
                .arg(m_parent ? (quintptr)m_parent->m_tileData : 0)
                .arg((quintptr)m_next.data())
                .arg(m_next ? (quintptr)m_next->m_tileData : 0);
        qDebug() << s;
    }

protected:
    KisTileData *m_tileData;
    bool m_commitedFlag;
    enumType m_type;

    qint32 m_col;
    qint32 m_row;

    KisMementoItemSP m_next;
    KisMementoItemSP m_parent;
private:
};



#endif /* KIS_MEMENTO_ITEM_H_ */

