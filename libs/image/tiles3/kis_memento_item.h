/*
 *  SPDX-FileCopyrightText: 2009 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KIS_MEMENTO_ITEM_H_
#define KIS_MEMENTO_ITEM_H_

#include <kis_shared.h>
#include <kis_shared_ptr.h>
#include "kis_tile.h"


class KisMementoItem;
typedef KisSharedPtr<KisMementoItem> KisMementoItemSP;

class KisMementoItem : public KisShared
{
public:
    enum enumType {
        CHANGED  = 0x0,
        DELETED  = 0x1
    };

public:
    KisMementoItem()
            : m_tileData(0), m_committedFlag(false) {
    }

    KisMementoItem(const KisMementoItem& rhs)
            : KisShared(),
            m_tileData(rhs.m_tileData),
            m_committedFlag(rhs.m_committedFlag),
            m_type(rhs.m_type),
            m_col(rhs.m_col),
            m_row(rhs.m_row),
            m_next(0),
            m_parent(0) {
        if (m_tileData) {
            if (m_committedFlag)
                m_tileData->acquire();
            else
                m_tileData->ref();
        }
    }

    /**
     * Automatically called by Kis..HashTable. It means that
     * this mementoItem is a root item of parental hierarchy.
     * So m_parent should be NULL. Taking into account the tile
     * was not present before, the status of the item
     * should be 'DELETED'.
     * This memmento item is considered as committed, so we acquire
     * the tile data right at the beginning.
     */
    KisMementoItem(qint32 col, qint32 row, KisTileData* defaultTileData, KisMementoManager *mm) {
        Q_UNUSED(mm);
        m_tileData = defaultTileData;
        /* acquire the tileData deliberately and completely */
        m_tileData->acquire();
        m_col = col;
        m_row = row;
        m_type = DELETED;
        m_parent = 0;
        m_committedFlag = true; /* yes, we've committed it */
    }

    /**
     * FIXME: Not sure this function has any particular usecase.
     * Just leave it for compatibility with a hash table
     */
    KisMementoItem(const KisMementoItem &rhs, KisMementoManager *mm) {
        Q_UNUSED(mm);
        m_tileData = rhs.m_tileData;
        /* Setting counter: m_refCount++ */
        m_tileData->ref();
        m_col = rhs.m_col;
        m_row = rhs.m_row;
        m_type = CHANGED;
        m_parent = 0;
        m_committedFlag = false;
    }

    ~KisMementoItem() {
        releaseTileData();
    }

    void notifyDetachedFromDataManager() {
        // just to resemble KisTile...
    }

    void notifyDeadWithoutDetaching() {
        // just to resemble KisTile...
    }

    void notifyAttachedToDataManager(KisMementoManager *mm) {
        Q_UNUSED(mm);
        // just to resemble KisTile...
    }


    void reset() {
        releaseTileData();
        m_tileData = 0;
        m_committedFlag = false;
    }

    void deleteTile(KisTile* tile, KisTileData* defaultTileData) {
        m_tileData = defaultTileData;
        /* Setting counter: m_refCount++ */
        m_tileData->ref();

        m_col = tile->col();
        m_row = tile->row();
        m_type = DELETED;
    }

    void changeTile(KisTile* tile) {
        m_tileData = tile->tileData();
        /* Setting counter: m_refCount++ */
        m_tileData->ref();
        m_col = tile->col();
        m_row = tile->row();
        m_type = CHANGED;
    }

    void commit() {
        if (m_committedFlag) return;
        if (m_tileData) {
            /**
             * Setting counters to proper values:
             * m_refCount++, m_usersCount++;
             * m_refCount--
             */
            m_tileData->acquire();
            m_tileData->deref();

            m_tileData->setMementoed(true);
        }
        m_committedFlag = true;
    }

    inline KisTileSP tile(KisMementoManager *mm) {
        Q_ASSERT(m_tileData);
        return KisTileSP(new KisTile(m_col, m_row, m_tileData, mm));
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
    inline KisMementoItemSP next() const {
        return m_next;
    }
    inline qint32 col() const {
        return m_col;
    }
    inline qint32 row() const {
        return m_row;
    }
    inline KisTileData* tileData() const {
        return m_tileData;
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
                .arg(m_committedFlag ? 'C' : '-')
                .arg((quintptr)m_parent.data())
                .arg(m_parent ? (quintptr)m_parent->m_tileData : 0)
                .arg((quintptr)m_next.data())
                .arg(m_next ? (quintptr)m_next->m_tileData : 0);
        dbgKrita << s;
    }

protected:
    void releaseTileData() {
        if (m_tileData) {
            if (m_committedFlag) {
                m_tileData->setMementoed(false);
                m_tileData->release();
            }
            else {
                m_tileData->deref();
            }
        }
    }

protected:
    KisTileData *m_tileData {0};
    bool m_committedFlag {false};
    enumType m_type {CHANGED};

    qint32 m_col {0};
    qint32 m_row {0};

    KisMementoItemSP m_next;
    KisMementoItemSP m_parent;
private:
};


#endif /* KIS_MEMENTO_ITEM_H_ */

