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

#include "StoryboardModel.h"
#include "StoryboardView.h"
#include <kis_image_animation_interface.h>

#include <QDebug>
#include <QMimeData>

#include <kis_icon.h>
#include <KoColorSpaceRegistry.h>
#include <kis_layer_utils.h>
#include <kis_group_layer.h>
#include "kis_time_range.h"
#include "kis_raster_keyframe_channel.h"
#include "KisStoryboardThumbnailRenderScheduler.h"

StoryboardModel::StoryboardModel(QObject *parent)
        : QAbstractItemModel(parent)
        , m_locked(false)
        , m_imageIdleWatcher(10)
        , m_renderScheduler(new KisStoryboardThumbnailRenderScheduler(this))
{
    connect(this, SIGNAL(rowsInserted(const QModelIndex, int, int)),
                this, SLOT(slotInsertChildRows(const QModelIndex, int, int)));

    connect(m_renderScheduler, SIGNAL(sigFrameCompleted(int, KisPaintDeviceSP)), this, SLOT(slotFrameRenderCompleted(int, KisPaintDeviceSP)));
    connect(m_renderScheduler, SIGNAL(sigFrameCancelled(int)), this, SLOT(slotFrameRenderCancelled(int)));
    //TODO: populate model with already existing item's thumbnails
}

StoryboardModel::~StoryboardModel()
{
    delete m_renderScheduler;
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
            int fps = m_image.isValid() ? m_image->animationInterface()->framerate() : 24;      //TODO: update all items on framerate change

            if (index.row() == FrameNumber) {
                if (value.toInt() < 0) {
                    return false;
                }
                ThumbnailData thumbnailData = qvariant_cast<ThumbnailData>(child->data());
                thumbnailData.frameNum = value.toInt();
                child->setData(QVariant::fromValue<ThumbnailData>(thumbnailData));
            }
            else if (index.row() == DurationSecond) {
                if (value.toInt() < 0) {
                    return false;
                }
                child->setData(value);
            }
            else if (index.row() == DurationFrame) {
                if (value.toInt() < 0) {
                    return false;
                }
                QModelIndex secondIndex = index.siblingAtRow(DurationSecond);
                setData(secondIndex, secondIndex.data().toInt() + value.toInt() / fps, role);
                child->setData(value.toInt() % fps);
            }
            else if (index.row() >= Comments) {
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

bool StoryboardModel::setThumbnailPixmapData(const QModelIndex & parentIndex, const KisPaintDeviceSP & dev)
{

    QModelIndex index = this->index(0, 0, parentIndex);
    QRect thumbnailRect = m_view->visualRect(parentIndex);
    float scale = qMin(thumbnailRect.height() / (float)m_image->height(), (float)thumbnailRect.width() / m_image->width());

    QImage image = dev->convertToQImage(KoColorSpaceRegistry::instance()->rgb8()->profile());
    QPixmap pxmap = QPixmap::fromImage(image);
    pxmap = pxmap.scaled((1.5)*scale*m_image->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation);

    StoryboardChild *child = m_items.at(index.parent().row())->child(index.row());
    if (child) {
        ThumbnailData thumbnailData = qvariant_cast<ThumbnailData>(child->data());
        thumbnailData.pixmap = pxmap;
        child->setData(QVariant::fromValue<ThumbnailData>(thumbnailData));
        emit dataChanged(index, index);
        return true;
    }
    return false;
}

bool StoryboardModel::updateDurationData(const QModelIndex & parentIndex)
{
    if (!parentIndex.isValid()) {
        return false;
    }

    int currentKeyframeTime = data(index(FrameNumber, 0, parentIndex)).toInt();
    int nextKeyframeTime = nextKeyframeGlobal(currentKeyframeTime);

    if (nextKeyframeTime == INT_MAX) {
        setData (index (DurationSecond, 0, parentIndex), 0);
        setData (index (DurationFrame, 0, parentIndex), 0);
    }
    else {
        int timeInFrame = nextKeyframeTime - currentKeyframeTime - 1;

        int fps = m_image->animationInterface()->framerate();

        if (index (DurationSecond, 0, parentIndex).data().toInt() != timeInFrame / fps) {
            setData (index (DurationSecond, 0, parentIndex), timeInFrame / fps);
        }
        if (index (DurationFrame, 0, parentIndex).data().toInt() != timeInFrame % fps) {
            setData (index (DurationFrame, 0, parentIndex), timeInFrame % fps);
        }
    }

    return true;
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

    m_imageIdleWatcher.setTrackedImage(image);
    m_imageIdleWatcher.startCountdown();
    connect(&m_imageIdleWatcher, SIGNAL(startedIdleMode()), this, SLOT(slotUpdateThumbnails()));

    m_renderScheduler->setImage(m_image);

    //for add, remove and move
    connect(m_image->animationInterface(), SIGNAL(sigKeyframeAdded(KisKeyframeSP)),
            this, SLOT(slotKeyframeAdded(KisKeyframeSP)));
    connect(m_image->animationInterface(), SIGNAL(sigKeyframeRemoved(KisKeyframeSP)),
            this, SLOT(slotKeyframeRemoved(KisKeyframeSP)));
    connect(m_image->animationInterface(), SIGNAL(sigKeyframeMoved(KisKeyframeSP, int)),
            this, SLOT(slotKeyframeMoved(KisKeyframeSP, int)));

    //for selection sync with timeline
    connect(m_image->animationInterface(), SIGNAL(sigUiTimeChanged(int)), this, SLOT(slotFrameChanged(int)));
    connect(m_view->selectionModel(), SIGNAL(selectionChanged(QItemSelection, QItemSelection)),
            this, SLOT(slotChangeFrameGlobal(QItemSelection, QItemSelection)));
}

void StoryboardModel::slotSetActiveNode(KisNodeSP node)
{
    m_activeNode = node;
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

QModelIndexList StoryboardModel::affectedIndexes(KisTimeRange range) const
{
    QModelIndex firstIndex = indexFromFrame(range.start());
    if (firstIndex.isValid()) {
        firstIndex = firstIndex.siblingAtRow(firstIndex.row() + 1);
    }
    else {
        firstIndex = lastIndexBeforeFrame(range.start());
        firstIndex = firstIndex.siblingAtRow(firstIndex.row() + 1);
    }

    QModelIndex lastIndex = indexFromFrame(range.end());
    if (!lastIndex.isValid()) {
        lastIndex = lastIndexBeforeFrame(range.end());
    }

    QModelIndexList list;
    if (!firstIndex.isValid()) {
        return list;
    }
    for (int i = firstIndex.row(); i <= lastIndex.row(); i++) {
        list.append(index(i, 0));
    }
    return list;
}

bool StoryboardModel::isOnlyKeyframe(KisKeyframeSP keyframe, int time) const
{
    bool onlyKeyframe = true;
    KisNodeSP node = m_image->rootLayer();
    while (node) {
        KisLayerUtils::recursiveApplyNodes(node,
                                            [keyframe, &onlyKeyframe, time] (KisNodeSP node) {
                                            if (node->isAnimated()) {
                                                auto keyframeMap = node->keyframeChannels();
                                                for (auto elem: keyframeMap) {
                                                    KisKeyframeChannel *keyframeChannel = elem;
                                                    bool keyframeAbsent = keyframeChannel->keyframeAt(time).isNull();
                                                    if (node != keyframe->channel()->node().toStrongRef()) {
                                                        onlyKeyframe &= keyframeAbsent;
                                                    }
                                                }
                                            }
                                            });
        node = node->nextSibling();
    }
    return onlyKeyframe;
}

int StoryboardModel::nextKeyframeGlobal(int keyframeTime) const
{
    KisNodeSP node = m_image->rootLayer();
    int nextKeyframeTime = INT_MAX;
    if (node) {
    KisLayerUtils::recursiveApplyNodes (node, [keyframeTime, &nextKeyframeTime] (KisNodeSP node)
    {
        if (node->isAnimated()) {
            KisKeyframeChannel *keyframeChannel = node->paintDevice()->keyframeChannel();

            //check for keyframe at the next frame, Kf == keyframe
            KisKeyframeSP nextFrameKf = keyframeChannel->keyframeAt(keyframeTime + 1);
            if (nextFrameKf.isNull()) {
                KisKeyframeSP activeKfNextFrame = keyframeChannel->nextKeyframe(keyframeChannel->activeKeyframeAt(keyframeTime));
                if (activeKfNextFrame && nextKeyframeTime == INT_MAX) {
                    nextKeyframeTime = activeKfNextFrame->time();
                }
                else if (activeKfNextFrame && nextKeyframeTime != INT_MAX){
                    nextKeyframeTime = qMin(nextKeyframeTime, activeKfNextFrame->time());
                }
            }
            else {
                nextKeyframeTime = nextFrameKf->time();
            }
        }
    });
    }

    return nextKeyframeTime;
}

//durations can be in frames or second, just provide right index
bool StoryboardModel::insertHoldFramesAfter(int newDuration, int oldDuration, QModelIndex index)
{
    if (!index.isValid() || newDuration < 0) {
        return false;
    }
    int frame = index.siblingAtRow(FrameNumber).data().toInt();
    int fps = m_image.isValid() ? m_image->animationInterface()->framerate(): 24;
    if (index.row() == DurationSecond) {
        newDuration *= fps;
        oldDuration *= fps;
    }
    int durationChange = newDuration - oldDuration;
    if (durationChange == 0) {
        return false;
    }
    KisNodeSP node = m_image->rootLayer();
    if (node) {
    KisLayerUtils::recursiveApplyNodes (node, [frame, durationChange] (KisNodeSP node)
    {
        if (node->isAnimated()) {
            KisKeyframeChannel *keyframeChannel = node->paintDevice()->keyframeChannel();
            if (keyframeChannel){
                if (durationChange > 0) {
                    KisKeyframeSP keyframe = keyframeChannel->lastKeyframe();
                    while (keyframe && keyframe != keyframeChannel->activeKeyframeAt(frame)) {
                        keyframeChannel->moveKeyframe(keyframe, keyframe->time() + durationChange);
                        keyframe = keyframeChannel->previousKeyframe(keyframe);
                    }
                }
                else if (durationChange < 0) {
                    KisKeyframeSP keyframe = keyframeChannel->activeKeyframeAt(frame + 1);
                    if (keyframe->time() != frame + 1) {
                        keyframe = keyframeChannel->nextKeyframe(keyframe);
                    }
                    while (keyframe) {
                        keyframeChannel->moveKeyframe(keyframe, keyframe->time() + durationChange);
                        keyframe = keyframeChannel->nextKeyframe(keyframe);
                    }
                }
            }
        }
    });
    }
    slotChangeFrameGlobal(m_view->selectionModel()->selection(), QItemSelection());

    return true;
}

bool StoryboardModel::insertItem(QModelIndex index, bool after)
{
    //index is the index at which context menu was created, or the + button belongs to
    //after is whether we want the item before or after index

    KisKeyframeChannel* keyframeChannel = m_activeNode->paintDevice()->keyframeChannel();
    if (!index.isValid()) {
        if (keyframeChannel) {
            int lastKeyframeTime = 0;
            KisNodeSP node = m_image->rootLayer();
            if (node) {
            KisLayerUtils::recursiveApplyNodes (node, [&lastKeyframeTime] (KisNodeSP node)
            {
                if (node->isAnimated()) {
                    lastKeyframeTime = qMax(lastKeyframeTime, node->paintDevice()->keyframeChannel()->lastKeyframe()->time());
                }
            });
            }

            keyframeChannel->addKeyframe(lastKeyframeTime + 1);
        }
        else {
            m_activeNode->getKeyframeChannel(KisKeyframeChannel::Content.id(), true);
        }
    }
    else {
        if (!keyframeChannel) {
            keyframeChannel = m_activeNode->getKeyframeChannel(KisKeyframeChannel::Content.id(), true);
        }
        int frame = this->index(FrameNumber, 0, index).data().toInt();
        if (after) {
            insertHoldFramesAfter(1, 0, this->index(DurationFrame, 0, index));
            int fps = m_image->animationInterface()->framerate();
            int durationInFrame = this->index(DurationFrame, 0, index).data().toInt() + fps * this->index(DurationSecond, 0, index).data().toInt();
            keyframeChannel->addKeyframe(frame + qMax(1, durationInFrame));
        }
        else {
            insertHoldFramesAfter(1, 0, this->index(DurationFrame, 0, index.siblingAtRow(index.row() - 1)));
            keyframeChannel->addKeyframe(frame);
        }
    }
    return true;
}

void StoryboardModel::slotFrameChanged(int frameId)
{
    m_view->setCurrentItem(frameId);
}

void StoryboardModel::slotChangeFrameGlobal(QItemSelection selected, QItemSelection deselected)
{
    Q_UNUSED(deselected);
    if (!selected.indexes().isEmpty()) {
        int frameId = data(index(0, 0, selected.indexes().at(0))).toInt();
        m_image->animationInterface()->switchCurrentTimeAsync(frameId);
    }
}

void StoryboardModel::slotKeyframeAdded(KisKeyframeSP keyframe)
{
    if (!indexFromFrame(keyframe->time()).isValid() && !isLocked()) {
        int frame = keyframe->time();
        int prevItemRow = lastIndexBeforeFrame(frame).row();
        insertRows(prevItemRow + 1, 1);
        setData (index (FrameNumber, 0, index(prevItemRow + 1, 0)), frame);
        updateDurationData(index(prevItemRow + 1, 0));
        updateDurationData(index(prevItemRow, 0));
        m_view->setCurrentItem(frame);
    }

    slotUpdateThumbnailForFrame(keyframe->time());
}

void StoryboardModel::slotKeyframeRemoved(KisKeyframeSP keyframe)
{
    QModelIndex itemIndex = indexFromFrame(keyframe->time());
    if (itemIndex.isValid()) {
        if (isOnlyKeyframe(keyframe, keyframe->time())) {
            removeRows(itemIndex.row(), 1);
        }
    }
}

void StoryboardModel::slotKeyframeMoved(KisKeyframeSP keyframe, int from)
{
    QModelIndex fromIndex = indexFromFrame(from);
    if (fromIndex.isValid()) {
        //check whether there are keyframes at the "from" time in other nodes
        bool onlyKeyframe = isOnlyKeyframe(keyframe, from);

        int toItemRow = lastIndexBeforeFrame(keyframe->time()).row();
        QModelIndex destinationIndex = indexFromFrame(keyframe->time());

        if (onlyKeyframe && !destinationIndex.isValid()) {
            setData(index(0, 0, fromIndex), keyframe->time());
            moveRows(QModelIndex(), fromIndex.row(), 1, QModelIndex(), toItemRow + 1);

            updateDurationData(indexFromFrame(keyframe->time()));
            updateDurationData(lastIndexBeforeFrame(keyframe->time()));

            QModelIndex newFromIndex = lastIndexBeforeFrame(from);
            updateDurationData(newFromIndex);
        }
        else if (onlyKeyframe && destinationIndex.isValid()) {
            removeRows(fromIndex.row(), 1);

            QModelIndex beforeFromIndex = lastIndexBeforeFrame(from);
            updateDurationData(beforeFromIndex);
        }
        else if (!destinationIndex.isValid()) {
            insertRows(toItemRow + 1, 1);
            destinationIndex = index(toItemRow + 1, 0);
            for (int i=1; i < rowCount(destinationIndex); i++) {
                setData(index(i, 0, destinationIndex), index(i, 0, fromIndex).data());
            }
            setData(index(0, 0, destinationIndex), keyframe->time());

            updateDurationData(indexFromFrame(keyframe->time()));
            updateDurationData(lastIndexBeforeFrame(keyframe->time()));
        }
    }
}

void StoryboardModel::slotUpdateThumbnailForFrame(int frame)
{
    QModelIndex index = indexFromFrame(frame);
    bool affected = true;
    if (index.isValid()) {
        if (frame == m_image->animationInterface()->currentUITime()) {
            setThumbnailPixmapData(index, m_image->projection());
            affected = false;
        }
        m_renderScheduler->scheduleFrameForRegeneration(frame, affected);
    }
}

void StoryboardModel::slotUpdateThumbnails()
{
    int currentTime = m_image->animationInterface()->currentUITime();
    slotUpdateThumbnailForFrame(currentTime);

    KisTimeRange affectedRange;
    if (m_activeNode) {
        KisRasterKeyframeChannel *currentChannel = m_activeNode->paintDevice()->keyframeChannel();
        if (currentChannel) {
            affectedRange = currentChannel->affectedFrames(currentTime);
            if (affectedRange.isInfinite()) {
                int end = index(FrameNumber, 0, index(rowCount() - 1, 0)).data().toInt();
                affectedRange = KisTimeRange(affectedRange.start(), end, true);
            }
            QModelIndexList dirtyIndexes = affectedIndexes(affectedRange);
            foreach(QModelIndex index, dirtyIndexes) {
                int frame = this->index(FrameNumber, 0, index).data().toInt();
                slotUpdateThumbnailForFrame(frame);
            }
        }
        else {
            affectedRange = KisTimeRange::infinite(0);
            //update all
        }
    }
    else {
        return;
    }
}

void StoryboardModel::slotFrameRenderCompleted(int frame, KisPaintDeviceSP dev)
{
    QModelIndex index = indexFromFrame(frame);
    if (index.isValid()) {
        setThumbnailPixmapData(index, dev);
    }
}

void StoryboardModel::slotFrameRenderCancelled(int frame)
{
    qDebug()<<"frame render for "<<frame<<" cancelled";
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

            //get the next keyframe and set duration to the num of frames in between
            KisKeyframeSP currentKeyframe = m_activeNode->paintDevice()->keyframeChannel()->keyframeAt(data(index(FrameNumber, 0, parentIndex)).toInt());
            int nextKeyframeTime = nextKeyframeGlobal(currentKeyframe->time());

            if (nextKeyframeTime == INT_MAX) {
                setData (index (2, 0, parentIndex), 0);
                setData (index (3, 0, parentIndex), 0);
            }
            else {
                int timeInFrame = nextKeyframeTime - currentKeyframe->time() - 1;

                int fps = m_image->animationInterface()->framerate();
                setData (index (DurationSecond, 0, parentIndex), timeInFrame / fps);
                setData (index (DurationFrame, 0, parentIndex), timeInFrame % fps);
            }
        }
    }
}
