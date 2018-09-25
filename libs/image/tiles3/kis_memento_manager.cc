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


/**
 * The class is supposed to store the changes of the paint device
 * it is associated with. The history of changes is presented in form
 * of transactions (revisions). If you purge the history of one
 * transaction (revision) with purgeHistory() we won't be able to undo
 * the changes made by this transactions.
 *
 * The Memento Manager can be in two states:
 *     - Named Transaction is in progress - it means the caller
 *       has explicitly requested creation of a new transaction.
 *       The handle for the transaction is stored on a side of
 *       the caller. And the history will be automatically purged
 *       when the handler dies.
 *     - Anonymous Transaction is in progress - the caller isn't
 *       bothered about transactions at all. We pretend as we do
 *       not support any versioning and do not have any historical
 *       information. The history of such transactions is not purged
 *       automatically, but it is free'd when younger named transaction
 *       is purged.
 */

#define blockRegistration() (m_registrationBlocked = true)
#define unblockRegistration() (m_registrationBlocked = false)
#define registrationBlocked() (m_registrationBlocked)

#define namedTransactionInProgress() ((bool)m_currentMemento)

KisMementoManager::KisMementoManager()
    : m_index(0),
      m_headsHashTable(0),
      m_registrationBlocked(false)
{
    /**
     * Tile change/delete registration is enabled for all
     * devices by default. It can't be delayed.
     */
}

KisMementoManager::KisMementoManager(const KisMementoManager& rhs)
    : m_index(rhs.m_index, 0),
        m_revisions(rhs.m_revisions),
        m_cancelledRevisions(rhs.m_cancelledRevisions),
        m_headsHashTable(rhs.m_headsHashTable, 0),
        m_currentMemento(rhs.m_currentMemento),
        m_registrationBlocked(rhs.m_registrationBlocked)
{
    Q_ASSERT_X(!m_registrationBlocked,
               "KisMementoManager", "(impossible happened) "
               "The device has been copied while registration was blocked");
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
 *    manager at stages 1 and 5 do not coincide.
 * This will not lead to any memory leaks or bugs seen, it just
 * not good from a theoretical perspective.
 */

void KisMementoManager::registerTileChange(KisTile *tile)
{
    if (registrationBlocked()) return;

    DEBUG_LOG_TILE_ACTION("reg. [C]", tile, tile->col(), tile->row());

    KisMementoItemSP mi = m_index.getExistingTile(tile->col(), tile->row());

    if(!mi) {
        mi = new KisMementoItem();
        mi->changeTile(tile);
        m_index.addTile(mi);

        if(namedTransactionInProgress())
            m_currentMemento->updateExtent(mi->col(), mi->row());
    }
    else {
        mi->reset();
        mi->changeTile(tile);
    }
}

void KisMementoManager::registerTileDeleted(KisTile *tile)
{
    if (registrationBlocked()) return;

    DEBUG_LOG_TILE_ACTION("reg. [D]", tile, tile->col(), tile->row());

    KisMementoItemSP mi = m_index.getExistingTile(tile->col(), tile->row());

    if(!mi) {
        mi = new KisMementoItem();
        mi->deleteTile(tile, m_headsHashTable.defaultTileData());
        m_index.addTile(mi);

        if(namedTransactionInProgress())
            m_currentMemento->updateExtent(mi->col(), mi->row());
    }
    else {
        mi->reset();
        mi->deleteTile(tile, m_headsHashTable.defaultTileData());
    }
}

void KisMementoManager::commit()
{
    if (m_index.isEmpty()) {
        if(namedTransactionInProgress()) {
            //warnTiles << "Named Transaction is empty";
            /**
             * We still need to continue commit, because
             * a named transaction may be reverted by the user
             */
        }
        else {
            m_currentMemento = 0;
            return;
        }
    }

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
        //iter.next(); // previous line does this for us
    }

    KisHistoryItem hItem;
    hItem.itemList = revisionList;
    hItem.memento = m_currentMemento.data();
    m_revisions.append(hItem);

    m_currentMemento = 0;
    Q_ASSERT(m_index.isEmpty());

    DEBUG_DUMP_MESSAGE("COMMIT_DONE");

    // Waking up pooler to prepare copies for us
    KisTileDataStore::instance()->kickPooler();
}

KisTileSP KisMementoManager::getCommitedTile(qint32 col, qint32 row, bool &existingTile)
{
    /**
     * Our getOldTile mechanism is supposed to return current
     * tile, if the history is disabled. So we return zero if
     * no named transaction is in progress.
     */
    if(!namedTransactionInProgress())
        return KisTileSP();

    KisMementoItemSP mi = m_headsHashTable.getReadOnlyTileLazy(col, row, existingTile);
    Q_ASSERT(mi);
    return mi->tile(0);
}

KisMementoSP KisMementoManager::getMemento()
{
    /**
     * We do not allow nested transactions
     */
    Q_ASSERT(!namedTransactionInProgress());

    // Clear redo() information
    m_cancelledRevisions.clear();

    commit();
    m_currentMemento = new KisMemento(this);

    DEBUG_LOG_SIMPLE_ACTION("GET_MEMENTO_DONE");

    return m_currentMemento;
}

KisMementoSP KisMementoManager::currentMemento() {
    return m_currentMemento;
}

#define forEachReversed(iter, list) \
        for(iter=list.end(); iter-- != list.begin();)


void KisMementoManager::rollback(KisTileHashTable *ht)
{
    commit();

    if (! m_revisions.size()) return;

    KisHistoryItem changeList = m_revisions.takeLast();

    KisMementoItemSP mi;
    KisMementoItemSP parentMI;
    KisMementoItemList::iterator iter;

    blockRegistration();
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
     * We have just deleted some tiles from the original hash table.
     * And they accurately reported to us about their death. Should
     * have reported... But we have prevented their registration with
     * explicitly blocking the process. So all the dead tiles are
     * going to /dev/null :)
     *
     * PS: It could cause some race condition... But we insist on
     * serialization  of rollback()/rollforward() requests. There is
     * not much sense in calling rollback() concurrently.
     */
    unblockRegistration();

    // We have just emulated a commit so:
    m_currentMemento = 0;
    Q_ASSERT(!namedTransactionInProgress());

    m_cancelledRevisions.prepend(changeList);
    DEBUG_DUMP_MESSAGE("UNDONE");

    // Waking up pooler to prepare copies for us
    KisTileDataStore::instance()->kickPooler();
}

void KisMementoManager::rollforward(KisTileHashTable *ht)
{
    Q_ASSERT(m_index.isEmpty());

    if (!m_cancelledRevisions.size()) return;

    KisHistoryItem changeList = m_cancelledRevisions.takeFirst();

    KisMementoItemSP mi;

    blockRegistration();
    Q_FOREACH (mi, changeList.itemList) {
        if (mi->parent()->type() == KisMementoItem::CHANGED)
            ht->deleteTile(mi->col(), mi->row());
        if (mi->type() == KisMementoItem::CHANGED)
            ht->addTile(mi->tile(this));

        m_index.addTile(mi);
    }
    // see comment in rollback()

    m_currentMemento = changeList.memento;
    commit();
    unblockRegistration();
    DEBUG_DUMP_MESSAGE("REDONE");
}

void KisMementoManager::purgeHistory(KisMementoSP oldestMemento)
{
    if (m_currentMemento == oldestMemento) {
        commit();
    }

    qint32 revisionIndex = findRevisionByMemento(oldestMemento);
    if (revisionIndex < 0) return;

    for(; revisionIndex > 0; revisionIndex--) {
        resetRevisionHistory(m_revisions.first().itemList);
        m_revisions.removeFirst();
    }

    Q_ASSERT(m_revisions.first().memento == oldestMemento);
    resetRevisionHistory(m_revisions.first().itemList);

    DEBUG_DUMP_MESSAGE("PURGE_HISTORY");
}

qint32 KisMementoManager::findRevisionByMemento(KisMementoSP memento) const
{
    qint32 index = -1;
    for(qint32 i = 0; i < m_revisions.size(); i++) {
        if (m_revisions[i].memento == memento) {
            index = i;
            break;
        }
    }
    return index;
}

void KisMementoManager::resetRevisionHistory(KisMementoItemList list)
{
    KisMementoItemSP parentMI;
    KisMementoItemSP mi;

    Q_FOREACH (mi, list) {
        parentMI = mi->parent();
        if(!parentMI) continue;

        while (parentMI->parent()) {
            parentMI = parentMI->parent();
        }
        mi->setParent(parentMI);
    }
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
    KisMementoItemSP mi;
    KisMementoItemHashTableIteratorConst iter(&m_index);

    while ((mi = iter.tile())) {
        mi->debugPrintInfo();
        iter.next();
    }

    printf("Revisions list:\n");
    qint32 i = 0;
    Q_FOREACH (const KisHistoryItem &changeList, m_revisions) {
        printf("--- revision #%d ---\n", i++);
        Q_FOREACH (mi, changeList.itemList) {
            mi->debugPrintInfo();
        }
    }

    printf("\nCancelled revisions list:\n");
    i = 0;
    Q_FOREACH (const KisHistoryItem &changeList, m_cancelledRevisions) {
        printf("--- revision #%d ---\n", m_revisions.size() + i++);
        Q_FOREACH (mi, changeList.itemList) {
            mi->debugPrintInfo();
        }
    }

    printf("----------------\n");
    m_headsHashTable.debugPrintInfo();
}
