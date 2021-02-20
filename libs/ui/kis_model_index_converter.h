/*
 *  SPDX-FileCopyrightText: 2011 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef __KIS_MODEL_INDEX_CONVERTER_H
#define __KIS_MODEL_INDEX_CONVERTER_H

#include "kis_model_index_converter_base.h"

class KisDummiesFacadeBase;
class KisNodeModel;


/**
 * The class for converting to/from QModelIndex and KisNodeDummy when
 * the root node is *hidden* (ShowRootLayer == *false*). All the selection
 * masks owned by the root layer are hidden as well.
 */

class KRITAUI_EXPORT KisModelIndexConverter : public KisModelIndexConverterBase
{
public:
    KisModelIndexConverter(KisDummiesFacadeBase *dummiesFacade,
                           KisNodeModel *model,
                           bool showGlobalSelection);

    KisNodeDummy* dummyFromRow(int row, QModelIndex parent) override;
    KisNodeDummy* dummyFromIndex(QModelIndex index) override;

    QModelIndex indexFromDummy(KisNodeDummy *dummy) override;
    bool indexFromAddedDummy(KisNodeDummy *parentDummy, int index,
                             const QString &newNodeMetaObjectType,
                             QModelIndex &parentIndex, int &row) override;

    int rowCount(QModelIndex parent) override;

private:
    inline bool checkDummyType(KisNodeDummy *dummy);
    inline bool checkDummyMetaObjectType(const QString &type);

private:
    KisDummiesFacadeBase *m_dummiesFacade;
    KisNodeModel *m_model;
    bool m_showGlobalSelection;
};

#endif /* __KIS_MODEL_INDEX_CONVERTER_H */
