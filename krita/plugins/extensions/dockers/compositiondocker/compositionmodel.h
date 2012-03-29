/*
 *  Copyright (c) 2012 Sven Langkamp <sven.langkamp@gmail.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; version 2 of the License.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
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

class QAction;
class CompositionModel : public QAbstractTableModel
{
    Q_OBJECT
public:
    CompositionModel(QObject* parent = 0);
    virtual ~CompositionModel();
    
    virtual QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const;
    virtual QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;
    virtual int rowCount(const QModelIndex& parent = QModelIndex()) const;
    virtual int columnCount(const QModelIndex& parent = QModelIndex()) const;
    virtual Qt::ItemFlags flags(const QModelIndex& index) const;

    KisLayerComposition* compositionFromIndex(const QModelIndex& index);
    void setCompositions(QList<KisLayerComposition*> compositions);
    
// public slots:
//     void clear();
private:
    QList<KisLayerComposition*> m_compositions;
};

#endif // TASKSETMODEL_H
