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

#include <kis_types.h>
/**
 * KisLayerModel offers a Qt model-view compatible view on the layer
 * hierarchy.
 */
class KisLayerModel : KoDocumentSectionModel
{

    Q_OBJECT

public: // from QAbstractItemModel

    KisLayerModel(QObject * parent );
    ~KisLayerModel();

    void setRoot( KisGroupLayerSP layer );

    QModelIndex indexFromLayer(KisLayer *layer) const;
    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    int columnCount(const QModelIndex &parent = QModelIndex()) const;
    QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const;
    QModelIndex parent(const QModelIndex &index) const;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
    Qt::ItemFlags flags(const QModelIndex &index) const;
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole);

private:

    bool isGroupLayer( KisLayer * layer );
    bool isPaintLayer( KisLayer * layer );
    bool isAdjustmentLayer( KisLayer * layer );

    class Private;
    Private * const m_d;

};

#endif
