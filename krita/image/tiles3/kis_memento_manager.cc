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

KisMementoManager::KisMementoManager()
{
}

KisMementoManager::KisMementoManager(const KisMementoManager& rhs)
    :m_index(rhs.m_index),
     m_revisions(rhs.m_revisions),
     m_cancelledRevisions(rhs.m_cancelledRevisions),
     m_headsHashTable(rhs.m_headsHashTable)
{
}

KisMementoManager::~KisMementoManager()
{
    // Nothing to be done here. Happily...
    // Everything is done by QList and KisSharedPtr...
}

/**
 *  TODO: We assume that a tile won't be COWed until commit.
 *        So to  say,  we assume  that  registerTileChange() 
 *        will be called once a commit
 */
void KisMementoManager::registerTileChange(KisTile *tile)
{
    m_cancelledRevisions.clear();
    KisMementoItemSP mi = new KisMementoItem();
    mi->changeTile(tile);
    m_index.append(mi);
}

void KisMementoManager::registerTileDeleted(KisTile *tile)
{
    m_cancelledRevisions.clear();
    KisMementoItemSP mi = new KisMementoItem();
    mi->deleteTile(tile);
    m_index.append(mi);
}
void KisMementoManager::commit()
{
    KisMementoItemSP mi;
    bool newTile;
    foreach(mi, m_index) {
        mi->setParent(m_headsHashTable.getTileLazy(mi->col(), mi->row(), newTile));
        mi->commit();
        
        m_headsHashTable.deleteTile(mi->col(), mi->row());
        m_headsHashTable.addTile(mi);
    }
    m_revisions.append(m_index);
    m_index.clear();
}
 
void KisMementoManager::rollback(KisTileHashTable *ht)
{
    Q_ASSERT_X(!m_index.size(),"KisMementoManager::rollback" ,
               "You can't rollback with dirty working copy!");
    
    if(! m_revisions.size()) return;
    
    KisMementoItemList changeList = m_revisions.takeLast();
    
    KisMementoItemSP mi;
    KisMementoItemSP parentMI;

    foreach(mi, changeList) {
        parentMI = mi->parent();
        
        if(mi->type() == KisMementoItem::CHANGED)
            ht->deleteTile(mi->col(), mi->row());
        if(parentMI->type() == KisMementoItem::CHANGED)
            ht->addTile(parentMI->tile(this));

        m_headsHashTable.deleteTile(parentMI->col(), parentMI->row());
        m_headsHashTable.addTile(parentMI);
       
        // This is not necessary
        //mi->setParent(0);
    }
    /**
     * FIXME: tricky hack alert.
     * We have  just deleted some tiles  from original hash  table.  And they
     * accurately reported to us about  their death, adding their bodies into
     * m_index list. We don't need them actually... :)
     *
     * PS: It could cause some race condition... But for now we can insist on
     * serialization  of rollback()/rollforward()  requests.Speaking  truly i
     * can't see sny sense in calling rollback() concurrently.
     */
    m_index.clear();

    m_cancelledRevisions.prepend(changeList);
}

void KisMementoManager::rollforward(KisTileHashTable *ht)
{
    Q_ASSERT_X(!m_index.size(), "KisMementoManager::rollforward",
               "You can't rollback with dirty working copy!");

    if(! m_cancelledRevisions.size()) return;

    KisMementoItemList changeList = m_cancelledRevisions.takeFirst();

    KisMementoItemSP mi;
    foreach(mi, changeList) {
        if(mi->parent()->type() == KisMementoItem::CHANGED)
            ht->deleteTile(mi->col(), mi->row());
        if(mi->type() == KisMementoItem::CHANGED)
            ht->addTile(mi->tile(this));
    }
    /**
     * FIXME: tricky hack alert.
     * We have  just deleted some tiles  from original hash  table.  And they
     * accurately reported to us about  their death, adding their bodies into
     * m_index list. We don't need them actually... :)
     *
     * PS: It could cause some race condition... But for now we can insist on
     * serialization  of rollback()/rollforward()  requests.Speaking  truly i
     * can't see sny sense in calling rollback() concurrently.
     */
    m_index.clear();

    m_index = changeList;
    commit();
}

void KisMementoManager::setDefaultTileData(KisTileData *defaultTileData)
{
    m_headsHashTable.setDefaultTileData(defaultTileData);
}

void KisMementoManager::debugPrintInfo() {
    printf("KisMementoManager stats:\n");
    printf("Index list\n");
    KisMementoItemList changeList;
    KisMementoItemSP mi;
    foreach(mi, m_index) {
        mi->debugPrintInfo();
    }

    printf("Revisions list:\n");
    qint32 i=0;
    foreach(changeList, m_revisions) {
        printf("--- revision #%d ---\n", i++);
        foreach(mi,changeList) {
            mi->debugPrintInfo();
        }
    }

    printf("\nCancelled revisions list:\n");
    i=0;
    foreach(changeList, m_cancelledRevisions) {
        printf("--- revision #%d ---\n", m_revisions.size() + i++);
        foreach(mi,changeList) {
            mi->debugPrintInfo();
        }
    }

    printf("----------------\n");
    m_headsHashTable.debugPrintInfo();
}


