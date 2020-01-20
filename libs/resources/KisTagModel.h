/*
 * Copyright (c) 2018 boud <boud@valdyas.org>
 * Copyright (c) 2020 Agata Cacko <cacko.azh@gmail.com>
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

#include <KisTag.h>
#include <KoResource.h>

#include "kritaresources_export.h"


class KRITARESOURCES_EXPORT KisTagModel : public QAbstractTableModel
{
    Q_OBJECT

private:

    KisTagModel(const QString &resourceType, QObject *parent = 0);

public:

    enum Columns {
        Id = 0,
        Url,
        Name,
        Comment,
        ResourceType,
        Active,
        KisTagRole,
    };


    ~KisTagModel() override;

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role) const override;

// KisTagModel API

    KisTagSP tagForIndex(QModelIndex index = QModelIndex()) const;
    QList<KisTagSP> allTags() const;

    bool addEmptyTag(const QString &tagName, QVector<KoResourceSP> taggedResouces);
    bool addEmptyTag(const KisTagSP tag, QVector<KoResourceSP> taggedResouces);
    bool addTag(const KisTagSP tag, QVector<KoResourceSP> taggedResouces = QVector<KoResourceSP>());
    bool removeTag(const KisTagSP tag);
    bool tagResource(const KisTagSP tag, const KoResourceSP resource);
    bool untagResource(const KisTagSP tag, const KoResourceSP resource);
    bool renameTag(const KisTagSP tag, const QString &name);
    bool changeTagActive(const KisTagSP tag, bool active);
    QVector<KisTagSP> tagsForResource(int resourceId) const;


private:

    friend class DlgDbExplorer;
    friend class KisTagModelProvider;
    friend class TestTagModel;

    void setResourceType(const QString &resourceType);


    bool prepareQuery();

    struct Private;
    Private* const d;

};

typedef QSharedPointer<KisTagModel> KisTagModelSP;

#endif // KISTAGMODEL_H
