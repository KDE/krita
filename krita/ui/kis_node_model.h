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

    void setImage(KisImageWSP image);

    KisNodeSP nodeFromIndex(const QModelIndex &index);
    vKisNodeSP nodesFromIndexes(const QModelIndexList &list);
    virtual QModelIndex indexFromNode(const KisNodeSP node) const;

    virtual int rowCount(const QModelIndex &parent = QModelIndex()) const;
    virtual int columnCount(const QModelIndex &parent = QModelIndex()) const;
    virtual QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const;
    virtual QModelIndex parent(const QModelIndex &index) const;
    virtual QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
    virtual Qt::ItemFlags flags(const QModelIndex &index) const;
    virtual bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole);
    virtual QStringList mimeTypes() const;
    QMimeData * mimeData(const QModelIndexList & indexes) const;
    virtual bool dropMimeData(const QMimeData * data, Qt::DropAction action, int row, int column, const QModelIndex & parent);
    virtual Qt::DropActions supportedDragActions() const;
    virtual Qt::DropActions supportedDropActions() const;

signals:

    void nodeActivated(KisNodeSP);
    void requestAddNode(KisNodeSP node, KisNodeSP activeNode);
    void requestAddNode(KisNodeSP node, KisNodeSP parent, int index);
    void requestMoveNode(KisNodeSP node, KisNodeSP activeNode);
    void requestMoveNode(KisNodeSP node, KisNodeSP parent, int index);

private slots:

    void beginInsertNodes(KisNode * parent, int index);
    void endInsertNodes(KisNode * parent, int index);
    void beginRemoveNodes(KisNode * parent, int index);
    void endRemoveNodes(KisNode * parent, int index);
    void updateSettings();
    void progressPercentageChanged(int, const KisNodeSP);
    void layersChanged();
private:

    class Private;
    Private * const m_d;

};

#endif
