/*
 * Copyright (c) 2018 boud <boud@valdyas.org>
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
#ifndef KISTAGMODEL_H
#define KISTAGMODEL_H

#include <QObject>
#include <QAbstractTableModel>

#include "kritaresources_export.h"

class KRITARESOURCES_EXPORT KisTagModel : public QAbstractTableModel
{
    Q_OBJECT
public:

    enum Columns {
        Id = 0,
        Url,
        Name,
        Comment,
        ResourceType
    };

    KisTagModel(const QString &resourceType, QObject *parent = 0);
    ~KisTagModel() override;

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role) const override;

// KisTagModel API

    void setResourceType(const QString &resourceType);

private:

    bool prepareQuery();

    struct Private;
    Private* const d;

};

#endif // KOTAGMODEL_H
