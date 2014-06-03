/*
 *  Copyright (c) 2007 Boudewijn Rempt <boud@valdyas.org>
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
#ifndef KIS_NODE_MODEL
#define KIS_NODE_MODEL

#include <KoDocumentSectionModel.h>
#include "krita_export.h"
#include <kis_types.h>

class KisDummiesFacadeBase;
class KisNodeDummy;
class KisShapeController;

/**
 * KisNodeModel offers a Qt model-view compatible view on the node
 * hierarchy.
 *
 * Note that there's a discrepancy between the krita node tree model
 * and the model Qt wants to see: we hide the root node from Qt.
 *
 * The node model shows an inverse view on the layer tree: we want the
 * first layer to show up at the bottom.
 */
class KRITAUI_EXPORT KisNodeModel : public KoDocumentSectionModel
{

    Q_OBJECT

public: // from QAbstractItemModel

    KisNodeModel(QObject * parent);
    ~KisNodeModel();

    void setDummiesFacade(KisDummiesFacadeBase *dummiesFacade, KisImageWSP image, KisShapeController *shapeController);
    KisNodeSP nodeFromIndex(const QModelIndex &index) const;
    QModelIndex indexFromNode(KisNodeSP node) const;

public:

    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    int columnCount(const QModelIndex &parent = QModelIndex()) const;
    QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const;
    QModelIndex parent(const QModelIndex &index) const;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
    Qt::ItemFlags flags(const QModelIndex &index) const;
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole);
    QStringList mimeTypes() const;
    QMimeData* mimeData(const QModelIndexList & indexes) const;
    bool dropMimeData(const QMimeData * data, Qt::DropAction action, int row, int column, const QModelIndex & parent);
    Qt::DropActions supportedDragActions() const;
    Qt::DropActions supportedDropActions() const;

signals:

    void nodeActivated(KisNodeSP);

    void requestAddNode(KisNodeSP node, KisNodeSP parent, KisNodeSP aboveThis);
    void requestMoveNode(KisNodeSP node, KisNodeSP parent, KisNodeSP aboveThis);

private slots:
    void slotBeginInsertDummy(KisNodeDummy *parent, int index, const QString &metaObjectType);
    void slotEndInsertDummy(KisNodeDummy *dummy);
    void slotBeginRemoveDummy(KisNodeDummy *dummy);
    void slotEndRemoveDummy();
    void slotDummyChanged(KisNodeDummy *dummy);

    void slotIsolatedModeChanged();

    void updateSettings();
    void processUpdateQueue();
    void progressPercentageChanged(int, const KisNodeSP);

private:
    friend class KisModelIndexConverter;
    friend class KisModelIndexConverterShowAll;

    void connectDummy(KisNodeDummy *dummy, bool needConnect);
    void connectDummies(KisNodeDummy *dummy, bool needConnect);

    void resetIndexConverter();

    bool correctNewNodeLocation(KisNodeSP node,
                                KisNodeDummy* &parentDummy,
                                KisNodeDummy* &aboveThisDummy);

    void regenerateItems(KisNodeDummy *dummy);
    bool belongsToIsolatedGroup(KisNodeSP node) const;

private:

    struct Private;
    Private * const m_d;
};

#endif
