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

#include <QtGlobal>
#include "kis_memento_manager.h"
#include "kis_memento.h"


//#define DEBUG_MM

#ifdef DEBUG_MM
#define DEBUG_LOG_TILE_ACTION(action, tile, col, row)                   \
    printf("### MementoManager (0x%X): %s "             \
           "\ttile:\t0x%X (%d, %d) ###\n", (quintptr)this, action,  \
           (quintptr)tile, col, row)

#define DEBUG_LOG_SIMPLE_ACTION(action)                 \
    printf("### MementoManager (0x%X): %s\n", (quintptr)this, action)

#define DEBUG_DUMP_MESSAGE(action) do {                                 \
        printf("\n### MementoManager (0x%X): %s \t\t##########\n",  \
               (quintptr)this, action);                                 \
        debugPrintInfo();                                               \
        printf("##################################################################\n\n"); \
    } while(0)
#else

#define DEBUG_LOG_TILE_ACTION(action, tile, col, row)
#define DEBUG_LOG_SIMPLE_ACTION(action)
#define DEBUG_DUMP_MESSAGE(action)
#endif


KisMementoManager::KisMementoManager()
    : m_index(0),
      m_headsHashTable(0)
{
    /**
     * Get an initial memento to show we want history for ALL the devices.
     * If we don't do it, we won't be able to start history later,
     * as we don't have any special api for this.
     * It shouldn't create much overhead for unversioned devices as
     * no tile-duplication accurs without calling a commit()
     * method regularily.
     */
    (void) getMemento();
}

KisMementoManager::KisMementoManager(const KisMementoManager& rhs)
    : m_index(rhs.m_index, 0),
        m_revisions(rhs.m_revisions),
        m_cancelledRevisions(rhs.m_cancelledRevisions),
        m_headsHashTable(rhs.m_headsHashTable, 0),
        m_currentMemento(rhs.m_currentMemento)
{
    Q_ASSERT_X(m_currentMemento,
               "KisMementoManager", "(impossible happened) "
               "Seems like a device was created without history support");
}

KisMementoManager::~KisMementoManager()
{
    // Nothing to be done here. Happily...
    // Everything is done by QList and KisSharedPtr...
    DEBUG_LOG_SIMPLE_ACTION("died\n");
}

/**
 * NOTE: We don't assume that the registerTileChange/Delete
 * can be called once a commit only. Reverse can happen when we
 * do sequential clears of the device. In such a case the tiles
 * will be removed and added several times during a commit.
 *
 * TODO: There is an 'uncomfortable' state for the tile possible
 * 1) Imagine we have a clear device
 * 2) Then we painted something in a tile
 * 3) It registered itself using registerTileChange()
 * 4) Then we called clear() and getMemento() [==commit()]
 * 5) The tile will be registered as deleted and successfully
 *    committed to a revision. That means the states of the memento
 *    manager at stages 1 and 5 do not coinside.
 * This will not lead to any memory leaks or bugs seen, it just
 * not good from a theoretical perspective.
 */

void KisMementoManager::registerTileChange(KisTile *tile)
{
    if (!m_currentMemento) return;
    m_cancelledRevisions.clear();

    DEBUG_LOG_TILE_ACTION("reg. [C]", tile, tile->col(), tile->row());

    KisMementoItemSP mi = m_index.getExistedTile(tile->col(), tile->row());

    if(!mi) {
        mi = new KisMementoItem();
        mi->changeTile(tile);
        m_index.addTile(mi);
        m_currentMemento->updateExtent(mi->col(), mi->row());
    }
    else {
        mi->reset();
        mi->changeTile(tile);
    }
}

void KisMementoManager::registerTileDeleted(KisTile *tile)
{
    if (!m_currentMemento) return;
    m_cancelledRevisions.clear();

    DEBUG_LOG_TILE_ACTION("reg. [D]", tile, tile->col(), tile->row());

    KisMementoItemSP mi = m_index.getExistedTile(tile->col(), tile->row());

    if(!mi) {
        mi = new KisMementoItem();
        mi->deleteTile(tile, m_headsHashTable.defaultTileData());
        m_index.addTile(mi);
        m_currentMemento->updateExtent(mi->col(), mi->row());
    }
    else {
        mi->reset();
        mi->deleteTile(tile, m_headsHashTable.defaultTileData());
    }
}

void KisMementoManager::commit()
{
    if (m_index.isEmpty()) return;

    KisMementoItemList revisionList;
    KisMementoItemSP mi;
    KisMementoItemSP parentMI;
    bool newTile;

    KisMementoItemHashTableIterator iter(&m_index);
    while ((mi = iter.tile())) {
        parentMI = m_headsHashTable.getTileLazy(mi->col(), mi->row(), newTile);

        mi->setParent(parentMI);
        mi->commit();
        revisionList.append(mi);

        m_headsHashTable.deleteTile(mi->col(), mi->row());

        iter.moveCurrentToHashTable(&m_headsHashTable);
        //++iter; // previous line does this for us
    }

    KisHistoryItem hItem;
    hItem.itemList = revisionList;
    hItem.memento = m_currentMemento.data();
    m_revisions.append(hItem);

    Q_ASSERT(m_index.isEmpty());

    // Waking up pooler to prepare copies for us
    globalTileDataStore.kickPooler();
}

KisTileSP KisMementoManager::getCommitedTile(qint32 col, qint32 row)
{
    /**
     * Our getOldTile mechanism is supposed to return current tile, if
     * the history is disabled. So we return zero if no transaction
     * is in progress.
     */
    if(!m_currentMemento || !m_currentMemento->valid())
        return 0;

    KisMementoItemSP mi = m_headsHashTable.getReadOnlyTileLazy(col, row);
    Q_ASSERT(mi);
    return mi->tile(0);
}

KisMementoSP KisMementoManager::getMemento()
{
    commit();
    m_currentMemento = new KisMemento(this);

    DEBUG_DUMP_MESSAGE("GET_MEMENTO");

    return m_currentMemento;
}

#define forEachReversed(iter, list) \
        for(iter=list.end(); iter-- != list.begin();)

#define saveAndClearMemento(memento) KisMementoSP _mem = (memento); memento=0
#define restoreMemento(memento) (memento) = _mem

void KisMementoManager::rollback(KisTileHashTable *ht)
{
    commit();

    if (! m_revisions.size()) return;

    KisHistoryItem changeList = m_revisions.takeLast();

    KisMementoItemSP mi;
    KisMementoItemSP parentMI;
    KisMementoItemList::iterator iter;

    saveAndClearMemento(m_currentMemento);
    forEachReversed(iter, changeList.itemList) {
        mi=*iter;
        parentMI = mi->parent();

        if (mi->type() == KisMementoItem::CHANGED)
            ht->deleteTile(mi->col(), mi->row());
        if (parentMI->type() == KisMementoItem::CHANGED)
            ht->addTile(parentMI->tile(this));

        m_headsHashTable.deleteTile(parentMI->col(), parentMI->row());
        m_headsHashTable.addTile(parentMI);

        // This is not necessary
        //mi->setParent(0);
    }
    /**
     * NOTE: tricky hack alert.
     * We have  just deleted some tiles  from original hash  table.  And they
     * accurately reported to us about  their death. Should have reported...
     * But we   prevented   addition   of  their   bodies   with   zeroing
     * m_currentMemento. All dead tiles are going to /dev/null :)
     *
     * PS: It could cause some race condition... But for now we can insist on
     * serialization  of rollback()/rollforward()  requests.Speaking  truly i
     * can't see sny sense in calling rollback() concurrently.
     */
    restoreMemento(m_currentMemento);

    m_cancelledRevisions.prepend(changeList);
    DEBUG_DUMP_MESSAGE("UNDONE");
}

void KisMementoManager::rollforward(KisTileHashTable *ht)
{
    commit();

    if (! m_cancelledRevisions.size()) return;

    KisHistoryItem changeList = m_cancelledRevisions.takeFirst();

    KisMementoItemSP mi;

    saveAndClearMemento(m_currentMemento);
    foreach(mi, changeList.itemList) {
        if (mi->parent()->type() == KisMementoItem::CHANGED)
            ht->deleteTile(mi->col(), mi->row());
        if (mi->type() == KisMementoItem::CHANGED)
            ht->addTile(mi->tile(this));

        m_index.addTile(mi);
    }
    /**
     * NOTE: tricky hack alert.
     * We have  just deleted some tiles  from original hash  table.  And they
     * accurately reported to us about  their death. Should have reported...
     * But we   prevented   addition   of  their   bodies   with   zeroing
     * m_currentMemento. All dead tiles are going to /dev/null :)
     *
     * PS: It could cause some race condition... But for now we can insist on
     * serialization  of rollback()/rollforward()  requests.Speaking  truly i
     * can't see sny sense in calling rollback() concurrently.
     */

    m_currentMemento = changeList.memento;
    commit();
    restoreMemento(m_currentMemento);
    DEBUG_DUMP_MESSAGE("REDONE");
}

void KisMementoManager::setDefaultTileData(KisTileData *defaultTileData)
{
    m_headsHashTable.setDefaultTileData(defaultTileData);
    m_index.setDefaultTileData(defaultTileData);
}

void KisMementoManager::debugPrintInfo()
{
    printf("KisMementoManager stats:\n");
    printf("Index list\n");
    KisHistoryItem changeList;
    KisMementoItemSP mi;
    KisMementoItemHashTableIterator iter(&m_index);

    while ((mi = iter.tile())) {
        mi->debugPrintInfo();
        ++iter;
    }

    printf("Revisions list:\n");
    qint32 i = 0;
    foreach(changeList, m_revisions) {
        printf("--- revision #%d ---\n", i++);
        foreach(mi, changeList.itemList) {
            mi->debugPrintInfo();
        }
    }

    printf("\nCancelled revisions list:\n");
    i = 0;
    foreach(changeList, m_cancelledRevisions) {
        printf("--- revision #%d ---\n", m_revisions.size() + i++);
        foreach(mi, changeList.itemList) {
            mi->debugPrintInfo();
        }
    }

    printf("----------------\n");
    m_headsHashTable.debugPrintInfo();
}

void KisMementoManager::removeMemento(KisMemento* memento)
{
    if (memento == m_currentMemento) { // This happen when a memento is not put on the stack
        commit();
    }
    int lastIndex = -1;
    for (int i = 0; i < m_revisions.size(); ++i) {
        if (m_revisions[i].memento == memento) {
          lastIndex = i + 1;
          break;
        }
    }
    Q_ASSERT(lastIndex <= 2);
    for (int i = 0; i < lastIndex; ++i) {
        foreach(KisMementoItemSP item, m_revisions[0].itemList)
        {
            item->setParent(0);
        }
        m_revisions.takeAt(0);
    }
}
