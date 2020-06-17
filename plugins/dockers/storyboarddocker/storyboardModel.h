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
#ifndef STORYBOARD_MODEL
#define STORYBOARD_MODEL

#include <QAbstractListModel>
#include <QStringList>

#include "storyboardItem.h"
#include "commentModel.h"

/*
    The main storyboard model. 
*/

class CommentBox
{
public:
    CommentBox()
    : content("")
    , scrollValue(0)
    {}
    CommentBox(const CommentBox& other)
    : content(other.content)
    , scrollValue(other.scrollValue)
    {}
    ~CommentBox()
    {}
    QVariant content;
    QVariant scrollValue;
};
Q_DECLARE_METATYPE(CommentBox)

class StoryboardModel : public QAbstractItemModel
{
    Q_OBJECT

public:
//if we don't need this constructor change it
    StoryboardModel(QObject *parent = nullptr);

    QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex &index) const override;

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;
    bool setCommentScrollData(const QModelIndex & index, const QVariant & value);
    Qt::ItemFlags flags(const QModelIndex &index) const override;
    
    //for removing and inserting rows
    bool insertRows(int position, int rows, const QModelIndex &index = QModelIndex());
    bool removeRows(int position, int rows, const QModelIndex &index = QModelIndex());
    bool moveRows(const QModelIndex &sourceParent, int sourceRow, int count,
                    const QModelIndex &destinationParent, int destinationChild);

    //for drag and drop
    QMimeData *mimeData(const QModelIndexList &indexes) const;
    bool dropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent);
    Qt::DropActions supportedDropActions() const override;
    Qt::DropActions supportedDragActions() const override;

    //these function access the value from the comment model
    int visibleCommentCount() const;
    int visibleCommentsUpto(QModelIndex index) const;
    void setCommentModel(CommentModel *commentModel);
    Comment getComment(int row) const;

private Q_SLOTS:
    void slotCommentDataChanged();
    void slotCommentRowInserted(const QModelIndex, int, int);
    void slotCommentRowRemoved(const QModelIndex, int, int);
    void slotCommentRowMoved(const QModelIndex &sourceParent, int sourceRow, int count,
                            const QModelIndex &destinationParent, int destinationChild);

private:
    QVector<StoryboardItem*> m_items;
    int m_commentCount = 0;
    QVector<Comment> m_commentList;
    CommentModel *m_commentModel;
};

#endif