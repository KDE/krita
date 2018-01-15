/*
 *  Copyright (c) 2011 Dmitry Kazakov <dimula73@gmail.com>
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

#include "kis_model_index_converter.h"

#include "kis_selection_mask.h"
#include "kis_node_dummies_graph.h"
#include "kis_dummies_facade_base.h"
#include "kis_node_model.h"
#include "kis_node_manager.h"
#include "KisReferenceImagesLayer.h"


KisModelIndexConverter::KisModelIndexConverter(KisDummiesFacadeBase *dummiesFacade,
                                               KisNodeModel *model,
                                               bool showGlobalSelection)
    : m_dummiesFacade(dummiesFacade),
      m_model(model),
      m_showGlobalSelection(showGlobalSelection)
{
}

inline bool KisModelIndexConverter::checkDummyType(KisNodeDummy *dummy)
{
    return !KisNodeManager::isNodeHidden(dummy->node(), !m_showGlobalSelection);
}

inline bool KisModelIndexConverter::checkDummyMetaObjectType(const QString &type)
{
    // TODO: refactor too?
    if (m_showGlobalSelection) return true;

    QString selectionMaskType = KisSelectionMask::staticMetaObject.className();
    QString referencesLayerType = KisReferenceImagesLayer::staticMetaObject.className();
    return type != selectionMaskType && type != referencesLayerType;
}

KisNodeDummy* KisModelIndexConverter::dummyFromRow(int row, QModelIndex parent)
{

    KisNodeDummy *parentDummy = parent.isValid() ?
        dummyFromIndex(parent) : m_dummiesFacade->rootDummy();

    if(!parentDummy) return 0;

    KisNodeDummy *resultDummy = 0;

    // a child of the root node
    if(!parentDummy->parent()) {
        KisNodeDummy *currentDummy = parentDummy->lastChild();
        while(currentDummy) {
            if(checkDummyType(currentDummy)) {
                if(!row) {
                    resultDummy = currentDummy;
                    break;
                }
                row--;
            }
            currentDummy = currentDummy->prevSibling();
        }
    }
    // a child of other layer
    else {
        int rowCount = parentDummy->childCount();
        int index = rowCount - row - 1;
        resultDummy = parentDummy->at(index);
    }


    return resultDummy;
}

KisNodeDummy* KisModelIndexConverter::dummyFromIndex(QModelIndex index)
{
    Q_ASSERT(index.isValid());
    Q_ASSERT(index.internalPointer());
    return static_cast<KisNodeDummy*>(index.internalPointer());
}

QModelIndex KisModelIndexConverter::indexFromDummy(KisNodeDummy *dummy)
{
    Q_ASSERT(dummy);
    KisNodeDummy *parentDummy = dummy->parent();

    // a root node
    if(!parentDummy) return QModelIndex();

    int row = 0;

    // a child of the root node
    if(!parentDummy->parent()) {
        if(!checkDummyType(dummy)) return QModelIndex();

        KisNodeDummy *currentDummy = parentDummy->lastChild();
        while(currentDummy && currentDummy != dummy) {
            if(checkDummyType(currentDummy)) {
                row++;
            }
            currentDummy = currentDummy->prevSibling();
        }
    }
    // a child of other layer
    else {
        int rowCount = parentDummy->childCount();
        int index = parentDummy->indexOf(dummy);
        row = rowCount - index - 1;
    }

    return m_model->createIndex(row, 0, (void*)dummy);
}

bool KisModelIndexConverter::indexFromAddedDummy(KisNodeDummy *parentDummy,
                                                 int index,
                                                 const QString &newNodeMetaObjectType,
                                                 QModelIndex &parentIndex,
                                                 int &row)
{
    // adding a root node
    if(!parentDummy) {
        Q_ASSERT(!index);
        return false;
    }

    // adding a child of the root node
    if(!parentDummy->parent()) {
        if(!checkDummyMetaObjectType(newNodeMetaObjectType)) {
            return false;
        }

        row = 0;

        parentIndex = QModelIndex();
        KisNodeDummy *dummy = parentDummy->lastChild();
        int toScan = parentDummy->childCount() - index;
        while(dummy && toScan > 0) {
            if(checkDummyType(dummy)) {
                row++;
            }
            dummy = dummy->prevSibling();
            toScan--;
        }
    }
    // everything else
    else {
        parentIndex = indexFromDummy(parentDummy);
        int rowCount = parentDummy->childCount();
        row = rowCount - index;
    }

    return true;
}

int KisModelIndexConverter::rowCount(QModelIndex parent)
{
    KisNodeDummy *dummy = parent.isValid() ?
        dummyFromIndex(parent) : m_dummiesFacade->rootDummy();

    // a root node (hidden)
    if(!dummy) return 0;

    int numChildren = 0;

    // children of the root node
    if(!dummy->parent()) {
        KisNodeDummy *currentDummy = dummy->lastChild();
        while(currentDummy) {
            if(checkDummyType(currentDummy)) {
                numChildren++;
            }

            currentDummy = currentDummy->prevSibling();
        }
    }
    // children of other nodes
    else {
        numChildren = dummy->childCount();
    }

    return numChildren;
}
