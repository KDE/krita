/*
 *  Copyright (c) 2007 Boudewijn Rempt <boud@valdyas.org>
 *
 *  this program is free software; you can redistribute it and/or modify
 *  it under the terms of the gnu general public license as published by
 *  the free software foundation; either version 2 of the license, or
 *  (at your option) any later version.
 *
 *  this program is distributed in the hope that it will be useful,
 *  but without any warranty; without even the implied warranty of
 *  merchantability or fitness for a particular purpose.  see the
 *  gnu general public license for more details.
 *
 *  you should have received a copy of the gnu general public license
 *  along with this program; if not, write to the free software
 *  foundation, inc., 675 mass ave, cambridge, ma 02139, usa.
 */
#ifndef KIS_LAYER_MODEL
#define KIS_LAYER_MODEL

#include <KoDocumentSectionModel.h>
#include "krita_export.h"
#include <kis_types.h>
/**
 * KisLayerModel offers a Qt model-view compatible view on the layer
 * hierarchy.
 *
 *
 * Note that there's a discrepancy between the krita layer tree model
 * and the model Qt wants to see: we hide the root layer from Qt.
 */
class KRITAUI_EXPORT KisLayerModel : public KoDocumentSectionModel
{

    Q_OBJECT

public: // from QAbstractItemModel

    KisLayerModel(QObject * parent );
    ~KisLayerModel();

    void setImage( KisImageSP image );


    KisLayerSP layerFromIndex(const QModelIndex &index);
    vKisLayerSP layersFromIndexes(const QModelIndexList &list);
    QModelIndex indexFromLayer(const KisLayer *layer) const;
    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    int columnCount(const QModelIndex &parent = QModelIndex()) const;
    QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const;
    QModelIndex parent(const QModelIndex &index) const;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
    Qt::ItemFlags flags(const QModelIndex &index) const;
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole);

public slots:

    void beginInsertLayers( KisGroupLayer * parent, int index );
    void endInsertLayers( KisGroupLayer * parent, int index );
    void beginRemoveLayers( KisGroupLayer * parent, int index );
    void endRemoveLayers( KisGroupLayer * parent, int index );

private:

    class Private;
    Private * const m_d;

};

#endif
