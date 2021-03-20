/*
 *  SPDX-FileCopyrightText: 2011 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef __KIS_MODEL_INDEX_CONVERTER_BASE_H
#define __KIS_MODEL_INDEX_CONVERTER_BASE_H


#include <QModelIndex>
#include "kritaui_export.h"
class KisNodeDummy;

/**
 * The base class for converting objects to/from QModelIndex used
 * in KisNodeModel and  KisNodeDummy used in KisDummiesFacadeBase
 * (KisShapeController).
 *
 * This is not a trivial task, because the indexing of nodes is
 * reversed in KisNodeModel.
 */

class KRITAUI_EXPORT KisModelIndexConverterBase
{
public:
    virtual ~KisModelIndexConverterBase();

    /**
     * Returns the dummy staying in the specified \p row of a \p parent
     * May return null in case of inconsistency
     */
    virtual KisNodeDummy* dummyFromRow(int row, QModelIndex parent) = 0;

    /**
     * Returns the dummy associated with the \p index
     * WARNING: \p index must be valid
     * \note cannot return null
     */
    virtual KisNodeDummy* dummyFromIndex(QModelIndex index) = 0;

    /**
     * Returns the index corresponding to the position of the \p dummy
     * in the model. Will return invalid index if the dummy should be hidden
     */
    virtual QModelIndex indexFromDummy(KisNodeDummy *dummy) = 0;

    /**
     * Calculates the parent and the position in the model for newly created dummy
     * \param parentDummy the dummy parent
     * \param index the dummy index
     * \param newNodeMetaObjectType is a class name of a newly added node
     *        This name is got from Qt's meta object system so you must
     *        compare this value against a corresponding staticMetaObject
     *        object only.
     *        We do not pass a pointer to a real node to limit the access to
     *        real nodes.
     * \param parentIndex the parent index
     * \param row the dummy row
     * \return whether the new dummy will be shown in the model
     */
    virtual bool indexFromAddedDummy(KisNodeDummy *parentDummy, int index, const QString &newNodeMetaObjectType, QModelIndex &parentIndex, int &row) = 0;

    /**
     * Returns the number of children of the given index of the model
     */
    virtual int rowCount(QModelIndex parent) = 0;
};

#endif /* __KIS_MODEL_INDEX_CONVERTER_BASE_H */
