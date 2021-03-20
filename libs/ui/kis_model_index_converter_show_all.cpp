/*
 *  SPDX-FileCopyrightText: 2011 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_model_index_converter_show_all.h"

#include "kis_node_dummies_graph.h"
#include "kis_dummies_facade_base.h"
#include "kis_node_model.h"


KisModelIndexConverterShowAll::KisModelIndexConverterShowAll(KisDummiesFacadeBase *dummiesFacade,
                                                             KisNodeModel *model)
    : m_dummiesFacade(dummiesFacade),
      m_model(model)
{
}

KisNodeDummy* KisModelIndexConverterShowAll::dummyFromRow(int row, QModelIndex parent)
{
    Q_ASSERT(parent.isValid() || !row);

    if(!parent.isValid()) return m_dummiesFacade->rootDummy();

    KisNodeDummy *parentDummy = dummyFromIndex(parent);

    int rowCount = parentDummy->childCount();
    int index = rowCount - row - 1;
    return parentDummy->at(index);
}

KisNodeDummy* KisModelIndexConverterShowAll::dummyFromIndex(QModelIndex index)
{
    Q_ASSERT(index.isValid());
    Q_ASSERT(index.internalPointer());
    return static_cast<KisNodeDummy*>(index.internalPointer());
}

QModelIndex KisModelIndexConverterShowAll::indexFromDummy(KisNodeDummy *dummy)
{
    int row = 0;

    KisNodeDummy *parentDummy = dummy->parent();

    if(parentDummy) {
        int rowCount = parentDummy->childCount();
        int index = parentDummy->indexOf(dummy);
        row = rowCount - index - 1;
    }

    return m_model->createIndex(row, 0, (void*)dummy);
}

bool KisModelIndexConverterShowAll::indexFromAddedDummy(KisNodeDummy *parentDummy,
                                                        int index,
                                                        const QString &newNodeMetaObjectType,
                                                        QModelIndex &parentIndex,
                                                        int &row)
{
    Q_UNUSED(newNodeMetaObjectType);

    Q_ASSERT(parentDummy || !index);

    if(!parentDummy) {
        row = 0;
        parentIndex = QModelIndex();
    }
    else {
        int rowCount = parentDummy->childCount();
        row = rowCount - index;
        parentIndex = indexFromDummy(parentDummy);
    }
    return true;
}

int KisModelIndexConverterShowAll::rowCount(QModelIndex parent)
{
    if(!parent.isValid()) return 1;

    KisNodeDummy *parentDummy = dummyFromIndex(parent);

    return parentDummy->childCount();
}
