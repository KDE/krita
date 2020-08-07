/*
 *  Copyright (c) 2020 Saurabh Kumar <saurabhk660@gmail.com>
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
#ifndef COMMENT_MODEL
#define COMMENT_MODEL

#include "StoryboardItem.h"
#include <QAbstractListModel>
#include <kritastoryboarddocker_export.h>

class StoryboardModel;

/*
    This model manages the comment data of StoryboardModel
*/
class KRITASTORYBOARDDOCKER_EXPORT CommentModel : public QAbstractListModel
{

    Q_OBJECT

public:
    CommentModel(QObject *parent = 0);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

    Qt::ItemFlags flags(const QModelIndex &index) const override;
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;
    
    bool insertRows(int position, int rows, const QModelIndex &index = QModelIndex());
    bool removeRows(int position, int rows, const QModelIndex &index = QModelIndex());
    bool moveRows(const QModelIndex &sourceParent, int sourceRow, int count,
                    const QModelIndex &destinationParent, int destinationChild);
    QMimeData *mimeData(const QModelIndexList &indexes) const;
    bool dropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent);

    Qt::DropActions supportedDropActions() const override;
    Qt::DropActions supportedDragActions() const override;

    void resetData(QVector<Comment> list);
    QVector<Comment> getData();

Q_SIGNALS:
    /*
     * this signal is emitted whenever m_items is changed.
     * it is used to keep the StoryboardItemList in KisDocument
     * in sync with m_items
     */
    //TODO: Use a signal compressor to reduce frequency
    void sigCommentListChanged();

private:
    QVector<Comment> m_commentList;
    friend class StoryboardModel;
};

#endif
