/*
 *  Copyright (c) 2015 Dmitry Kazakov <dimula73@gmail.com>
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

#ifndef __TIMELINE_FRAMES_MODEL_BASE_H
#define __TIMELINE_FRAMES_MODEL_BASE_H

#include <QAbstractTableModel>

#include <QScopedPointer>
#include <QIcon>

#include "kritaanimationdocker_export.h"
#include "KisDocumentSectionModel.h"


class KRITAANIMATIONDOCKER_EXPORT TimelineFramesModelBase : public QAbstractTableModel
{
    Q_OBJECT
public:
    TimelineFramesModelBase(QObject *parent);
    ~TimelineFramesModelBase();

    bool canDropFrameData(const QMimeData *data, const QModelIndex &index);
    bool insertOtherLayer(int index, int dstRow);
    bool hideLayer(int row);

    bool createFrame(const QModelIndex &dstIndex);
    bool copyFrame(const QModelIndex &dstIndex);
    bool removeFrame(const QModelIndex &dstIndex);

    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    int columnCount(const QModelIndex &parent = QModelIndex()) const;
    QVariant data(const QModelIndex &index, int role) const;
    bool setData(const QModelIndex &index, const QVariant &value, int role);
    QVariant headerData(int section, Qt::Orientation orientation, int role) const;
    bool setHeaderData(int section, Qt::Orientation orientation, const QVariant &value, int role);

    Qt::DropActions supportedDragActions() const;
    Qt::DropActions supportedDropActions() const;
    QStringList mimeTypes() const;
    QMimeData * mimeData(const QModelIndexList &indexes) const;
    bool dropMimeData(const QMimeData * data, Qt::DropAction action, int row, int column, const QModelIndex & parent);
    Qt::ItemFlags flags(const QModelIndex &index) const;

    bool insertRows(int row, int count, const QModelIndex &parent);
    bool removeRows(int row, int count, const QModelIndex &parent);

    enum ItemDataRole
    {
        ActiveLayerRole = Qt::UserRole + 1,
        ActiveFrameRole,
        FrameCachedRole,
        PropertiesRole,
        OtherLayersRole,
        FrameEditableRole
    };

    // metatype is added by the original implementation
    typedef KisDocumentSectionModel::Property Property;
    typedef KisDocumentSectionModel::PropertyList PropertyList;

    struct OtherLayer {
        OtherLayer(const QString &_name) : name(_name) {}
        QString name;
    };

    typedef QList<OtherLayer> OtherLayersList;

private:
    struct Private;
    const QScopedPointer<Private> m_d;
};

Q_DECLARE_METATYPE( TimelineFramesModelBase::OtherLayersList )

#endif /* __TIMELINE_FRAMES_MODEL_BASE_H */
