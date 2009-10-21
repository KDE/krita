/*
 *  Copyright (c) 2009 Cyrille Berger <cberger@cberger.net>
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

#ifndef _KIS_COMPOSITE_OPS_MODEL_H_
#define _KIS_COMPOSITE_OPS_MODEL_H_

#include <QAbstractListModel>

class KoCompositeOp;

/**
 * This model can be use to show a list of visible composite op in a list view.
 */
class KisCompositeOpsModel : public QAbstractListModel
{
public:
    enum AdditionalRoles {
        CompositeOpSortRole = 0x1FDFDA
    };
public:
    KisCompositeOpsModel(const QList<KoCompositeOp*>& list);
    ~KisCompositeOpsModel();
    int rowCount(const QModelIndex & parent = QModelIndex()) const;
    QVariant data(const QModelIndex & index, int role = Qt::DisplayRole) const;
    const QString& itemAt(const QModelIndex & index) const;
    QModelIndex indexOf(const KoCompositeOp*) const;
    /**
     * @return the index for the given composite op id
     */
    QModelIndex indexOf(const QString&) const;
private:
    struct CompositeOpInfo {
        CompositeOpInfo(QString _id, QString _description, QString _category) : id(_id), description(_description), category(_category) {}
        QString id;
        QString description;
        QString category;
    };
    QList< CompositeOpInfo > m_list;
};

#endif
