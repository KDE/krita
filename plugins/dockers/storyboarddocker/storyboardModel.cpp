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

#include "storyboardModel.h"
#include "storyboardView.h"
#include <kis_image_animation_interface.h>
#include <kis_image.h>
#include <QDebug>
#include <QMimeData>

#include <kis_icon.h>
#include <kis_image.h>
#include <KoColorSpaceRegistry.h>
#include <kis_layer_utils.h>
#include <kis_group_layer.h>

StoryboardModel::StoryboardModel(QObject *parent)
        : QAbstractItemModel(parent)
        , m_locked(false)
{
    connect(this, SIGNAL(rowsInserted(const QModelIndex, int, int)),
                this, SLOT(slotInsertChildRows(const QModelIndex, int, int)));
    //TODO: populate model with already existing item's thumbnails
}

QModelIndex StoryboardModel::index(int row, int column, const QModelIndex &parent) const
{
    if (!hasIndex(row, column, parent)) {
        return QModelIndex();
    }
    if (row < 0 || row >= rowCount(parent)) {
        return QModelIndex();
    }
    if (column !=0) {
        return QModelIndex();
    }
    //1st level node has invalid parent
    if (!parent.isValid()) {
        return createIndex(row, column, m_items.at(row));
    }
    else if (!parent.parent().isValid()) {
        StoryboardItem *parentItem = m_items.at(parent.row());
        StoryboardChild *childItem = parentItem->child(row);
        if (childItem) {
            return createIndex(row, column, childItem);
        }
    }
    return QModelIndex();
}

QModelIndex StoryboardModel::parent(const QModelIndex &index) const
{
    if (!index.isValid()) {
        return QModelIndex();
    }

    //no parent for 1st level node
    StoryboardItem *childItemFirstLevel = static_cast<StoryboardItem*>(index.internalPointer());
    if (m_items.contains(childItemFirstLevel)) {
        return QModelIndex();
    }

    //return parent only for 2nd level nodes
    StoryboardChild *childItem = static_cast<StoryboardChild*>(index.internalPointer());
    StoryboardItem *parentItem = childItem->parent();
    int indexOfParent = m_items.indexOf(const_cast<StoryboardItem*>(parentItem));
    return createIndex(indexOfParent, 0, parentItem);
}

int StoryboardModel::rowCount(const QModelIndex &parent) const
{
    if (!parent.isValid()) {
        return m_items.count();
    }
    else if (!parent.parent().isValid()) {
        StoryboardItem *parentItem = m_items.at(parent.row());
        return parentItem->childCount();
    }
    return 0;   //2nd level nodes have no child
}

int StoryboardModel::columnCount(const QModelIndex &parent) const
{
   if (!parent.isValid()) {
       return 1;
   }
   //1st level nodes have 1 column
   if (!parent.parent().isValid()) {
       return 1;
   }
   //end level nodes have no child
   return 0;
}

QVariant StoryboardModel::data(const QModelIndex &index, int role) const
{

    if (!index.isValid()) {
        return QVariant();
    }
    //return data only for the storyboardChild i.e. 2nd level nodes
    if (!index.parent().isValid()) {
        return QVariant();
    }

    if (role == Qt::DisplayRole || role == Qt::EditRole || role == Qt::UserRole) {
        StoryboardChild *child = m_items.at(index.parent().row())->child(index.row());
        if (index.row() == StoryboardModel::FrameNumber) {
            ThumbnailData thumbnailData = qvariant_cast<ThumbnailData>(child->data());
            if (role == Qt::UserRole) {
                return thumbnailData.pixmap;
            }
            else {
                return thumbnailData.frameNum;
            }
        }
        else if (index.row() >= StoryboardModel::Comments) {
            CommentBox commentBox = qvariant_cast<CommentBox>(child->data());
            if (role == Qt::UserRole) {         //scroll bar position
                return commentBox.scrollValue;
            }
            else {
                return commentBox.content;
            }
        }
        return child->data();
    }
    return QVariant();
}

bool StoryboardModel::setData(const QModelIndex & index, const QVariant & value, int role)
{
    if (index.isValid() && (role == Qt::EditRole || role == Qt::DisplayRole)) {
        if (!index.parent().isValid()) {
            return false;
        }

        StoryboardChild *child = m_items.at(index.parent().row())->child(index.row());
        if (child) {
            int fps = m_image->animationInterface()->framerate();      //TODO: update all items on framerate change
            if ((index.row() == StoryboardModel::DurationFrame  || index.row() == StoryboardModel::DurationSecond) && value.toInt() < 0) {
                return false;
            }
            if (index.row() == StoryboardModel::DurationFrame && value.toInt() >= fps) {
                QModelIndex secondIndex = index.siblingAtRow(2);
                setData(secondIndex, secondIndex.data().toInt() + value.toInt() / fps, role);
                child->setData(value.toInt() % fps);
            }
            else if (index.row() == StoryboardModel::FrameNumber) {
                ThumbnailData thumbnailData = qvariant_cast<ThumbnailData>(child->data());
                thumbnailData.frameNum = value.toInt();
                child->setData(QVariant::fromValue<ThumbnailData>(thumbnailData));
            }
            else if (index.row() >= StoryboardModel::Comments) {
                CommentBox commentBox = qvariant_cast<CommentBox>(child->data());
                commentBox.content = value.toString();
                child->setData(QVariant::fromValue<CommentBox>(commentBox));
            }
            else {
                child->setData(value);
            }
            emit dataChanged(index, index);
            return true;
        }
    }
    return false;
}

bool StoryboardModel::setCommentScrollData(const QModelIndex & index, const QVariant & value)
{
    StoryboardChild *child = m_items.at(index.parent().row())->child(index.row());
    if (child) {
        CommentBox commentBox = qvariant_cast<CommentBox>(child->data());
        commentBox.scrollValue = value.toInt();
        child->setData(QVariant::fromValue<CommentBox>(commentBox));
        emit dataChanged(index, index);
        return true;
    }
    return false;
}

bool StoryboardModel::setThumbnailPixmapData(const QModelIndex & index, const QVariant & value)
{
    StoryboardChild *child = m_items.at(index.parent().row())->child(index.row());
    if (child) {
        ThumbnailData thumbnailData = qvariant_cast<ThumbnailData>(child->data());
        thumbnailData.pixmap = value;
        child->setData(QVariant::fromValue<ThumbnailData>(thumbnailData));
        emit dataChanged(index, index);
        return true;
    }
    return false;
}

Qt::ItemFlags StoryboardModel::flags(const QModelIndex & index) const
{
    if(!index.isValid()) {
        return Qt::ItemIsDropEnabled;
    }

    //1st level nodes
    if (!index.parent().isValid()) {
        return Qt::ItemIsDragEnabled | Qt::ItemIsSelectable | Qt::ItemIsEnabled;
    }

    //2nd level nodes
    return Qt::ItemIsSelectable | Qt::ItemIsEditable | Qt::ItemIsEnabled | Qt::ItemNeverHasChildren;
}

bool StoryboardModel::insertRows(int position, int rows, const QModelIndex &parent)
{
    if (!parent.isValid()) {
        if (position < 0 || position > m_items.count()) {
            return false;
        }
        beginInsertRows(QModelIndex(), position, position+rows-1);
        for (int row = 0; row < rows; ++row) {
            StoryboardItem *newItem = new StoryboardItem();
            m_items.insert(position, newItem);
        }
        endInsertRows();
        return true;
    }
    else if (!parent.parent().isValid()) {              //insert 2nd level nodes
        StoryboardItem *item = m_items.at(parent.row());

        if (position < 0 || position > item->childCount()) {
            return false;
        }
        beginInsertRows(parent, position, position+rows-1);
        for (int row = 0; row < rows; ++row) {
            item->insertChild(position, QVariant());
        }
        endInsertRows();
        return true;
    }
    //we can't insert to 2nd level nodes as they are leaf nodes
    return false;
}

bool StoryboardModel::removeRows(int position, int rows, const QModelIndex &parent)
{
    //remove 1st level nodes
    if (!parent.isValid()) {

        if (position < 0 || position >= m_items.count()) {
            return false;
        }
        beginRemoveRows(QModelIndex(), position, position+rows-1);
        for (int row = position + rows - 1; row >= position; row--) {
            delete m_items.at(row);
            m_items.removeAt(row);
        }
        endRemoveRows();
        return true;
    }
    else if (!parent.parent().isValid()) {                     //remove 2nd level nodes
        StoryboardItem *item = m_items.at(parent.row());

        if (position < 0 || position >= item->childCount()) {
            return false;
        }
        if (m_items.contains(item)) {
            beginRemoveRows(parent, position, position+rows-1);
            for (int row = 0; row < rows; ++row) {
                item->removeChild(position);
            }
            endRemoveRows();
            return true;
        }
    }
    //2nd level node has no child
    return false;
}

bool StoryboardModel::moveRows(const QModelIndex &sourceParent, int sourceRow, int count,
                                const QModelIndex &destinationParent, int destinationChild)
{
    if (sourceParent != destinationParent) {
        return false;
    }
    if (destinationChild == sourceRow || destinationChild == sourceRow + 1) {
        return false;
    }
    if (destinationChild > sourceRow + count - 1) {
        //we adjust for the upward shift, see qt doc for why this is needed
        beginMoveRows(sourceParent, sourceRow, sourceRow + count - 1, destinationParent, destinationChild + count - 1);
        destinationChild = destinationChild - count;
    }
    else {
        beginMoveRows(sourceParent, sourceRow, sourceRow + count - 1, destinationParent, destinationChild);
    }
    //for moves within the 1st level nodes for comment nodes
    if (sourceParent == destinationParent && sourceParent.isValid() && !sourceParent.parent().isValid()) {
        const QModelIndex parent = sourceParent;
        for (int row = 0; row < count; row++) {
            if (sourceRow < StoryboardModel::Comments || sourceRow >= rowCount(parent)) {
                return false;
            }
            if (destinationChild + row < StoryboardModel::Comments || destinationChild + row >= rowCount(parent)) {
                return false;
            }

            StoryboardItem *item = m_items.at(parent.row());
            item->moveChild(sourceRow, destinationChild + row);
        }
        endMoveRows();
        return true;
    }
    else if (!sourceParent.isValid()) {                  //for moves of 1st level nodes
        for (int row = 0; row < count; row++) {
            if (sourceRow < 0 || sourceRow >= rowCount()) {
                return false;
            }
            if (destinationChild + row < 0 || destinationChild + row >= rowCount()) {
                return false;
            }

            m_items.move(sourceRow, destinationChild + row);
        }
        endMoveRows();
        return true;
    }
    else {
        return false;
    }
}

QMimeData *StoryboardModel::mimeData(const QModelIndexList &indexes) const
{
    QMimeData *mimeData = new QMimeData();
    QByteArray encodeData;

    QDataStream stream(&encodeData, QIODevice::WriteOnly);

    //take the row number of the index where drag started
    foreach (QModelIndex index, indexes) {
        if (index.isValid()) {
            int row = index.row();
            stream << row;
        }
    }

    mimeData->setData("application/x-qabstractitemmodeldatalist", encodeData); //default mimetype
    return mimeData;
}

bool StoryboardModel::dropMimeData(const QMimeData *data, Qt::DropAction action,
                                int row, int column, const QModelIndex &parent)
{
    Q_UNUSED(column);
    if (action == Qt::IgnoreAction) {
        return false;
    }

    if (action == Qt::MoveAction && data->hasFormat("application/x-qabstractitemmodeldatalist")) {
        QByteArray bytes = data->data("application/x-qabstractitemmodeldatalist");
        QDataStream stream(&bytes, QIODevice::ReadOnly);

        if (parent.isValid()) {
            return false;
        }
        int sourceRow;
        QModelIndexList moveRowIndexes;
        while (!stream.atEnd()) {
            stream >> sourceRow;
            QModelIndex index = this->index(sourceRow, 0);
            moveRowIndexes.append(index);
        }
        moveRows(QModelIndex(), moveRowIndexes.at(0).row(), moveRowIndexes.count(), parent, row);

        //returning true deletes the source row
        return false;
    }
    return false;
}

Qt::DropActions StoryboardModel::supportedDropActions() const
{
    return Qt::CopyAction | Qt::MoveAction;
}

Qt::DropActions StoryboardModel::supportedDragActions() const
{
    return Qt::CopyAction | Qt::MoveAction;
}

int StoryboardModel::visibleCommentCount() const
{
    int visibleComments = 0;
    foreach(Comment comment, m_commentList) {
        if (comment.visibility) {
            visibleComments++;
        }
    }
    return visibleComments;
}

int StoryboardModel::visibleCommentsUpto(QModelIndex index) const
{
    int commentRow = index.row() - 4;
    int visibleComments = 0;
    for (int row = 0; row < commentRow; row++) {
        if (m_commentList.at(row).visibility) {
            visibleComments++;
        }
    }
    return visibleComments;
}

void StoryboardModel::setCommentModel(CommentModel *commentModel)
{
    m_commentModel = commentModel;
    connect(m_commentModel, SIGNAL(dataChanged(const QModelIndex ,const QModelIndex)),
                this, SLOT(slotCommentDataChanged()));
    connect(m_commentModel, SIGNAL(rowsRemoved(const QModelIndex ,int, int)),
                this, SLOT(slotCommentRowRemoved(const QModelIndex ,int, int)));
    connect(m_commentModel, SIGNAL(rowsInserted(const QModelIndex, int, int)),
                this, SLOT(slotCommentRowInserted(const QModelIndex, int, int)));
    connect(m_commentModel, SIGNAL(rowsMoved(const QModelIndex, int, int, const QModelIndex, int)),
                this, SLOT(slotCommentRowMoved(const QModelIndex, int, int, const QModelIndex, int)));
}

Comment StoryboardModel::getComment(int row) const
{
    return m_commentList.at(row);
}

void StoryboardModel::setLocked(bool value)
{
    m_locked = value;
}

bool StoryboardModel::isLocked() const
{
    return m_locked;
}

void StoryboardModel::setView(StoryboardView *view)
{
    m_view = view;
}

void StoryboardModel::setImage(KisImageWSP image)
{
    m_image = image;
}

QModelIndex StoryboardModel::indexFromFrame(int frame) const
{
    int end = rowCount(), begin = 0;
    while (end >= begin) {
        int row = begin + (end - begin) / 2;
        QModelIndex parentIndex = index(row, 0);
        QModelIndex childIndex = index(0, 0, parentIndex);
        if (childIndex.data().toInt() == frame) {
            return parentIndex;
        }
        else if (childIndex.data().toInt() > frame) {
            end = row - 1;
        }
        else if (childIndex.data().toInt() < frame) {
            begin = row + 1;
        }
    }
    return QModelIndex();
}

QModelIndex StoryboardModel::lastIndexBeforeFrame(int frame) const
{
    int end = rowCount(), begin = 0;
    QModelIndex retIndex;
    while (end >= begin) {
        int row = begin + (end - begin) / 2;
        QModelIndex parentIndex = index(row, 0);
        QModelIndex childIndex = index(0, 0, parentIndex);
        if (childIndex.data().toInt() >= frame) {
            end = row - 1;
        }
        else if (childIndex.data().toInt() < frame) {
            retIndex = parentIndex.row() > retIndex.row() ? parentIndex : retIndex;
            begin = row + 1;
        }
    }
    return retIndex;
}

void StoryboardModel::slotKeyframeAdded(KisKeyframeSP keyframe)
{
    if (!indexFromFrame(keyframe->time()).isValid() && !isLocked()) {
        int frame = keyframe->time();
        int prevItemRow = lastIndexBeforeFrame(frame).row();
        insertRows(prevItemRow + 1, 1);
        setData (index (0, 0, index(prevItemRow + 1, 0)), frame);
        m_view->setCurrentItem(frame);
        slotUpdateCurrentThumbnail();
    }
}

void StoryboardModel::slotKeyframeRemoved(KisKeyframeSP keyframe)
{
    QModelIndex itemIndex = indexFromFrame(keyframe->time());
    if (itemIndex.isValid()) {
        bool onlyKeyframe = true;
        KisNodeSP node = keyframe->channel()->node()->image()->rootLayer();
        while (node) {
            KisLayerUtils::recursiveApplyNodes(node,
                                                [keyframe, &onlyKeyframe] (KisNodeSP node) {
                                                if (node->isAnimated()) {
                                                    auto keyframeMap = node->keyframeChannels();
                                                    for (auto elem: keyframeMap) {
                                                        KisKeyframeChannel *keyframeChannel = elem;
                                                        bool keyframeAbsent = keyframeChannel->keyframeAt(keyframe->time()).isNull();
                                                        if (node != KisNodeSP(keyframe->channel()->node())) {
                                                            onlyKeyframe &= keyframeAbsent;
                                                        }
                                                    }
                                                }
                                                });
            node = node->nextSibling();
        }
        if (onlyKeyframe) {
            removeRows(itemIndex.row(), 1);
        }
    }
}

void StoryboardModel::slotKeyframeMoved(KisKeyframeSP keyframe, int from)
{
    QModelIndex fromIndex = indexFromFrame(from);
    if (fromIndex.isValid()) {
        //check whether there are keyframes at the "from" time in other nodes
        bool onlyKeyframe = true;
        KisNodeSP node = keyframe->channel()->node()->image()->rootLayer();
        while (node) {
            KisLayerUtils::recursiveApplyNodes(node,
                                                [from, keyframe, &onlyKeyframe] (KisNodeSP node) {
                                                if (node->isAnimated()) {
                                                    auto keyframeMap = node->keyframeChannels();
                                                    for (auto elem: keyframeMap) {
                                                        KisKeyframeChannel *keyframeChannel = elem;
                                                        bool keyframeAbsent = keyframeChannel->keyframeAt(from).isNull();
                                                        if (node != KisNodeSP(keyframe->channel()->node())) {
                                                            onlyKeyframe &= keyframeAbsent;
                                                        }
                                                    }
                                                }
                                                });
            node = node->nextSibling();
        }
        int toItemRow = lastIndexBeforeFrame(keyframe->time()).row();
        QModelIndex destinationIndex = indexFromFrame(keyframe->time());

        if (onlyKeyframe && !destinationIndex.isValid()) {
            setData(index(0, 0, fromIndex), keyframe->time());
            moveRows(QModelIndex(), fromIndex.row(), 1, QModelIndex(), toItemRow + 1);
        }
        else if (onlyKeyframe && destinationIndex.isValid()) {
            removeRows(fromIndex.row(), 1);
        }
        else if (!destinationIndex.isValid()) {
            insertRows(toItemRow + 1, 1);
            destinationIndex = index(toItemRow + 1, 0);
            for (int i=1; i < rowCount(destinationIndex); i++) {
                setData(index(i, 0, destinationIndex), index(i, 0, fromIndex).data());
            }
            setData(index(0, 0, destinationIndex), keyframe->time());
        }
        m_view->setCurrentItem(keyframe->time());
        slotUpdateCurrentThumbnail();
    }
}

void StoryboardModel::slotUpdateCurrentThumbnail()
{
    QModelIndex currIndex = indexFromFrame(m_image->animationInterface()->currentUITime());
    QModelIndex currFrameIndex = index(0, 0, currIndex);

    if (!currIndex.isValid()) {
        return;
    }

    QRect thumbnailRect = m_view->visualRect(currIndex);
    float scale = qMin(thumbnailRect.height() / (float)m_image->height(), (float)thumbnailRect.width() / m_image->width());
    KisPaintDeviceSP thumbDev = m_image->projection();

    if (thumbDev) {
        QImage image = thumbDev->convertToQImage(KoColorSpaceRegistry::instance()->rgb8()->profile());
        QPixmap pxmap = QPixmap::fromImage(image);
        pxmap = pxmap.scaled((1.5)*scale*m_image->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
        setThumbnailPixmapData(currFrameIndex, pxmap);
    }

    //TODO: check if other thumbnails need updating and update only those that need to be
}

void StoryboardModel::slotCommentDataChanged()
{
    m_commentList = m_commentModel->m_commentList;
    emit(layoutChanged());
}

void StoryboardModel::slotCommentRowInserted(const QModelIndex parent, int first, int last)
{
    Q_UNUSED(parent);
    int numItems = rowCount();
    for(int row = 0; row < numItems; row++) {
        QModelIndex parentIndex = index(row, 0);
        insertRows(4 + first, last - first + 1, parentIndex);       //four indices are already there
    }
    slotCommentDataChanged();
}

void StoryboardModel::slotCommentRowRemoved(const QModelIndex parent, int first, int last)
{
    Q_UNUSED(parent);
    int numItems = rowCount();
    for(int row = 0; row < numItems; row++) {
        QModelIndex parentIndex = index(row, 0);
        removeRows(4 + first, last - first + 1, parentIndex);
    }
    slotCommentDataChanged();
}

void StoryboardModel::slotCommentRowMoved(const QModelIndex &sourceParent, int start, int end,
                            const QModelIndex &destinationParent, int destinationRow)
{
    Q_UNUSED(sourceParent);
    Q_UNUSED(destinationParent);
    int numItems = rowCount();
    for(int row = 0; row < numItems; row++) {
        QModelIndex parentIndex = index(row, 0);
        moveRows(parentIndex, start + 4, end - start + 1, parentIndex, destinationRow + 4);
    }
    slotCommentDataChanged();
}

void StoryboardModel::slotInsertChildRows(const QModelIndex parent, int first, int last)
{
    if (!parent.isValid()) {
        int rows = last - first + 1;
        for (int row = 0; row < rows; ++row) {
            QModelIndex parentIndex = index(first + row, 0);
            insertRows(0, 4 + m_commentList.count(), parentIndex);

            QString sceneName = i18nc("default name for storyboard item", "scene ") + QString::number(m_lastScene);
            setData (index (1, 0, parentIndex), sceneName);
            m_lastScene++;

            setData (index (2, 0, parentIndex), 20);
            setData (index (3, 0, parentIndex), 20);
        }
    }
}
