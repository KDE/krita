/*
 *  Copyright (c) 2015 Jouni Pentikäinen <joupent@gmail.com>
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

#include "kis_model_index_converter_animated_layers.h"
#include "kis_node_dummies_graph.h"
#include "kis_dummies_facade.h"
#include "kis_node_model.h"


KisModelIndexConverterAnimatedLayers::KisModelIndexConverterAnimatedLayers(KisDummiesFacadeBase *dummiesFacade, KisNodeModel *model)
    : m_dummiesFacade(dummiesFacade),
      m_model(model)
{}

KisNodeDummy *KisModelIndexConverterAnimatedLayers::dummyFromRow(int row, QModelIndex parent)
{
    KisNodeDummy *parentDummy = parent.isValid() ?
        dummyFromIndex(parent) : m_dummiesFacade->rootDummy();

    if(!parentDummy) return 0;

    KisNodeDummy *resultDummy = 0;

    KisNodeDummy *currentDummy = parentDummy->lastChild();
    while (currentDummy) {
        if (currentDummy->node()->isAnimated()) {
            if (!row) {
                resultDummy = currentDummy;
                break;
            }
            row--;
        }
        currentDummy = currentDummy->prevSibling();
    }

    return resultDummy;
}

KisNodeDummy *KisModelIndexConverterAnimatedLayers::dummyFromIndex(QModelIndex index)
{
    Q_ASSERT(index.isValid());
    Q_ASSERT(index.internalPointer());
    return static_cast<KisNodeDummy*>(index.internalPointer());
}

QModelIndex KisModelIndexConverterAnimatedLayers::indexFromDummy(KisNodeDummy *dummy)
{
    Q_ASSERT(dummy);
    KisNodeDummy *parentDummy = dummy->parent();

    // a root node
    if (!parentDummy) return QModelIndex();
    if (!dummy->node()->isAnimated()) return QModelIndex();

    int row = 0;
    KisNodeDummy *currentDummy = parentDummy->lastChild();
    while (currentDummy && currentDummy != dummy) {
        if(currentDummy->node()->isAnimated()) {
            row++;
        }
        currentDummy = currentDummy->prevSibling();
    }

    return m_model->createIndex(row, 0, (void*)dummy);
}

bool KisModelIndexConverterAnimatedLayers::indexFromAddedDummy(KisNodeDummy *parentDummy, int index, const QString &newNodeMetaObjectType, QModelIndex &parentIndex, int &row)
{
    Q_UNUSED(newNodeMetaObjectType);

    // adding a root node
    if (!parentDummy) {
        Q_ASSERT(!index);
        return false;
    }

    row = 0;

    parentIndex = indexFromDummy(parentDummy);
    KisNodeDummy *dummy = parentDummy->lastChild();
    int toScan = parentDummy->childCount() - index;
    while (dummy && toScan > 0) {
        if(dummy->node()->isAnimated()) {
            row++;
        }
        dummy = dummy->prevSibling();
        toScan--;
    }

    return true;
}

int KisModelIndexConverterAnimatedLayers::rowCount(QModelIndex parent)
{
    KisNodeDummy *dummy = parent.isValid() ?
        dummyFromIndex(parent) : m_dummiesFacade->rootDummy();

    // a root node (hidden)
    if(!dummy) return 0;

    int numChildren = 0;
    KisNodeDummy *currentDummy = dummy->lastChild();
    while(currentDummy) {
        if(currentDummy->node()->isAnimated()) {
            numChildren++;
        }

        currentDummy = currentDummy->prevSibling();
    }

    return numChildren;
}
