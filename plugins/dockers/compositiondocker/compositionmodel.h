/*
 *  Copyright (c) 2012 Sven Langkamp <sven.langkamp@gmail.com>
 *
 *  This library is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef COMPOSITIONMODEL_H
#define COMPOSITIONMODEL_H

#include <QModelIndex>

#include <kis_types.h>
#include <kis_layer_composition.h>

class CompositionModel : public QAbstractTableModel
{
    Q_OBJECT
public:
    CompositionModel(QObject* parent = 0);
    ~CompositionModel() override;
    
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    bool setData ( const QModelIndex& index, const QVariant& value, int role = Qt::EditRole ) override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    int columnCount(const QModelIndex& parent = QModelIndex()) const override;
    Qt::ItemFlags flags(const QModelIndex& index) const override;

    KisLayerCompositionSP compositionFromIndex(const QModelIndex& index);
    void setCompositions(QList<KisLayerCompositionSP> compositions);
    
// public Q_SLOTS:
//     void clear();
private:
    QList<KisLayerCompositionSP> m_compositions;
};

#endif // TASKSETMODEL_H
