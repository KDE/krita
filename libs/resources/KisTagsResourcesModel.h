/*
 * Copyright (c) 2019 Agata Cacko <cacko.azh@gmail.com>
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
#ifndef KIS_TAGS_RESOURCES_MODEL_H
#define KIS_TAGS_RESOURCES_MODEL_H

#include <QObject>

#include "kritaresources_export.h"

#include <QAbstractItemModel>
#include <KisTag.h>
#include <KoResource.h>


class KRITARESOURCES_EXPORT KisTagsResourcesModel : public QAbstractTableModel
{
    Q_OBJECT

private:

    KisTagsResourcesModel(const QString &resourceType, QObject *parent = 0);

public:

    enum Columns {
        Id = 0,
        ResourceId,
        TagId,
    };

    ~KisTagsResourcesModel() override;

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role) const override;

    void setResourceType(const QString &resourceType);

    bool tagResource(const KisTagSP tag, const KoResourceSP resource);
    bool untagResource(const KisTagSP tag, const KoResourceSP resource);

    QVector<KisTagSP> tagsForResource(int resourceId) const;

    bool resetQuery();


private:

    bool prepareQuery();

    friend class KisTagsResourcesModelProvider;

    struct Private;
    Private * d;
};

#endif // KIS_TAGS_RESOURCES_MODEL_H
