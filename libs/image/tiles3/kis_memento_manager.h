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

#ifndef KIS_MEMENTO_MANAGER_
#define KIS_MEMENTO_MANAGER_

#include <QList>

#include "kis_memento_item.h"
#include "config-hash-table-implementaion.h"

typedef QList<KisMementoItemSP> KisMementoItemList;
typedef QListIterator<KisMementoItemSP> KisMementoItemListIterator;

class KisMemento;
struct KisHistoryItem {
    KisMemento* memento;
    KisMementoItemList itemList;
};

typedef QList<KisHistoryItem> KisHistoryList;

class KisMemento;
typedef KisSharedPtr<KisMemento> KisMementoSP;

#ifdef USE_LOCK_FREE_HASH_TABLE
#include "kis_tile_hash_table2.h"

typedef KisTileHashTableTraits2<KisMementoItem> KisMementoItemHashTable;
typedef KisTileHashTableIteratorTraits2<KisMementoItem> KisMementoItemHashTableIterator;
typedef KisTileHashTableIteratorTraits2<KisMementoItem> KisMementoItemHashTableIteratorConst;
#else
#include "kis_tile_hash_table.h"

typedef KisTileHashTableTraits<KisMementoItem> KisMementoItemHashTable;
typedef KisTileHashTableIteratorTraits<KisMementoItem, QWriteLocker> KisMementoItemHashTableIterator;
typedef KisTileHashTableIteratorTraits<KisMementoItem, QReadLocker> KisMementoItemHashTableIteratorConst;
#endif // USE_LOCK_FREE_HASH_TABLE


class KRITAIMAGE_EXPORT KisMementoManager
{
public:
    KisMementoManager();
    KisMementoManager(const KisMementoManager& rhs);
    ~KisMementoManager();

    /**
     * Most tricky part. This function is called by a tile, when  it gets new
     * tile-data through  COW. The Memento Manager wraps  this tile-data into
     * KisMementoItem class  and waits until  commit() order given.   By this
     * time KisMementoItem doesn't take part  in COW mechanism. It only holds
     * tileData->m_refCount counter to ensure tile isn't deleted from memory.
     * When commit()  comes, KisMementoItem grabs  tileData->m_usersCount and
     * since that  moment it is a  rightful co-owner of the  tileData and COW
     * participant.  It means that tileData won't be ever changed since then.
     * Every  write request  to the  original tile  will lead  to duplicating
     * tileData and registering it here again...
     */
    void registerTileChange(KisTile *tile);

    /**
     * Called when a tile  deleted. Creates empty KisMementoItem showing that
     * there was a tile one day
     */
    void registerTileDeleted(KisTile *tile);


    /**
     * Commits changes, made in  INDEX: appends m_index into m_revisions list
     * and owes all modified tileDatas.
     */
    void commit();

    /**
     * Undo and Redo stuff respectively.
     *
     * When calling them, INDEX list should be empty, so to say, "working
     * copy should be clean".
     */
    void rollback(KisTileHashTable *ht);
    void rollforward(KisTileHashTable *ht);

    /**
     * Get old tile, whose memento is in the HEAD revision.
     * \p existingTile returns if the tile is actually an existing
     *                 non-default tile or it was created on the fly
     *                 from the default tile data
     */
    KisTileSP getCommitedTile(qint32 col, qint32 row, bool &existingTile);

    KisMementoSP getMemento();

    bool hasCurrentMemento() {
        return m_currentMemento;
    }

    KisMementoSP currentMemento();

    void setDefaultTileData(KisTileData *defaultTileData);

    void debugPrintInfo();


    /**
     * Removes all the history that preceds the revision
     * pointed by oldestMemento. That is after calling to
     * purgeHistory(someMemento) you won't be able to do
     * rollback(someMemento) anymore.
     */
    void purgeHistory(KisMementoSP oldestMemento);

protected:
    qint32 findRevisionByMemento(KisMementoSP memento) const;
    void resetRevisionHistory(KisMementoItemList list);

protected:
    /**
     * INDEX of tiles to be committed with next commit()
     * We use a hash table to be able to check that
     * we have the only memento item for a tile
     * per commit efficiently
     */
    KisMementoItemHashTable m_index;

    /**
     * Main list that stores every commit ever done
     */
    KisHistoryList m_revisions;

    /**
     * List of revisions temporarily undone while rollback()
     */
    KisHistoryList m_cancelledRevisions;

    /**
     * A hash table, that stores the most recently updated
     * versions of tiles. Say, HEAD revision :)
     */
    KisMementoItemHashTable m_headsHashTable;

    /**
     * Stores extent of current INDEX.
     * It is the "name" of current named transaction
     */
    KisMementoSP m_currentMemento;

    /**
     * The flag that blocks registration of changes on tiles.
     * This is a temporary state of the memento manager, that
     * is used for traveling in history
     *
     * \see rollback()
     * \see rollforward()
     */
    bool m_registrationBlocked;
};

#endif /* KIS_MEMENTO_MANAGER_ */
