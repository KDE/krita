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
#include <kis_pointer_utils.h>
#include <kis_group_layer.h>
#include "kis_time_span.h"
#include "kis_raster_keyframe_channel.h"
#include "KisStoryboardThumbnailRenderScheduler.h"

StoryboardModel::StoryboardModel(QObject *parent)
        : QAbstractItemModel(parent)
        , m_locked(false)
        , m_imageIdleWatcher(10)
        , m_renderScheduler(new KisStoryboardThumbnailRenderScheduler(this))
        , m_renderSchedulingCompressor(1000,KisSignalCompressor::FIRST_ACTIVE)
{
    connect(this, SIGNAL(rowsInserted(const QModelIndex, int, int)),
                this, SLOT(slotInsertChildRows(const QModelIndex, int, int)));

    connect(m_renderScheduler, SIGNAL(sigFrameCompleted(int, KisPaintDeviceSP)), this, SLOT(slotFrameRenderCompleted(int, KisPaintDeviceSP)));
    connect(m_renderScheduler, SIGNAL(sigFrameCancelled(int)), this, SLOT(slotFrameRenderCancelled(int)));
    connect(&m_renderSchedulingCompressor, SIGNAL(timeout()), this, SLOT(slotUpdateThumbnails()));
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
        return createIndex(row, column, m_items.at(row).data());
    }
    else if (!parent.parent().isValid()) {
        StoryboardItemSP parentItem = m_items.at(parent.row());
        QSharedPointer<StoryboardChild> childItem = parentItem->child(row);
        if (childItem) {
            return createIndex(row, column, childItem.data());
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

    Q_FOREACH( StoryboardItemSP item, m_items) {
        if (item.data() == childItemFirstLevel) {
            return QModelIndex();
        }
    }

    //return parent only for 2nd level nodes
    StoryboardChild *childItem = static_cast<StoryboardChild*>(index.internalPointer());
    QSharedPointer<StoryboardItem> parentItem = childItem->parent();
    int indexOfParent = m_items.indexOf(parentItem);
    return createIndex(indexOfParent, 0, parentItem.data());
}

int StoryboardModel::rowCount(const QModelIndex &parent) const
{
    if (!parent.isValid()) {
        return m_items.count();
    }
    else if (!parent.parent().isValid()) {
        QSharedPointer<StoryboardItem> parentItem = m_items.at(parent.row());
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
        QSharedPointer<StoryboardChild> child = m_items.at(index.parent().row())->child(index.row());
        if (index.row() == StoryboardItem::FrameNumber) {
            ThumbnailData thumbnailData = qvariant_cast<ThumbnailData>(child->data());
            if (role == Qt::UserRole) {
                return thumbnailData.pixmap;
            }
            else {
                return thumbnailData.frameNum;
            }
        }
        else if (index.row() >= StoryboardItem::Comments) {
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

        QSharedPointer<StoryboardChild> child = m_items.at(index.parent().row())->child(index.row());
        if (child) {
            int fps = m_image.isValid() ? m_image->animationInterface()->framerate() : 24;      //TODO: update all items on framerate change

            if (index.row() == StoryboardItem::FrameNumber) {
                if (value.toInt() < 0) {
                    return false;
                }
                ThumbnailData thumbnailData = qvariant_cast<ThumbnailData>(child->data());
                thumbnailData.frameNum = value.toInt();
                child->setData(QVariant::fromValue<ThumbnailData>(thumbnailData));
            }
            else if (index.row() == StoryboardItem::DurationSecond) {
                if (value.toInt() < 0) {
                    return false;
                }
                child->setData(value);
            }
            else if (index.row() == StoryboardItem::DurationFrame) {
                if (value.toInt() < 0) {
                    return false;
                }
                QModelIndex secondIndex = index.siblingAtRow(StoryboardItem::DurationSecond);

                if (secondIndex.data().toInt() == 0 && value.toInt() == 0) {
                    return false;
                }
                setData(secondIndex, secondIndex.data().toInt() + value.toInt() / fps, role);
                child->setData(value.toInt() % fps);
            }
            else if (index.row() >= StoryboardItem::Comments) {
                CommentBox commentBox = qvariant_cast<CommentBox>(child->data());
                commentBox.content = value.toString();
                child->setData(QVariant::fromValue<CommentBox>(commentBox));
            }
            else {
                child->setData(value);
            }
            emit dataChanged(index, index);
            emit(sigStoryboardItemListChanged());
            return true;
        }
    }
    return false;
}

bool StoryboardModel::setCommentScrollData(const QModelIndex & index, const QVariant & value)
{
    QSharedPointer<StoryboardChild> child = m_items.at(index.parent().row())->child(index.row());
    if (child) {
        CommentBox commentBox = qvariant_cast<CommentBox>(child->data());
        commentBox.scrollValue = value.toInt();
        child->setData(QVariant::fromValue<CommentBox>(commentBox));
        emit(sigStoryboardItemListChanged());
        return true;
    }
    return false;
}

bool StoryboardModel::setThumbnailPixmapData(const QModelIndex & parentIndex, const KisPaintDeviceSP & dev)
{

    QModelIndex index = this->index(0, 0, parentIndex);
    QRect thumbnailRect = m_view->visualRect(parentIndex);
    float scale = qMin(thumbnailRect.height() / (float)m_image->height(), (float)thumbnailRect.width() / m_image->width());

    QImage image = dev->convertToQImage(KoColorSpaceRegistry::instance()->rgb8()->profile(), m_image->bounds());
    QPixmap pxmap = QPixmap::fromImage(image);
    pxmap = pxmap.scaled((1.5)*scale*m_image->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation);

    if (!index.parent().isValid())
        return false;

    QSharedPointer<StoryboardChild> child = m_items.at(index.parent().row())->child(index.row());
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

    int currentKeyframeTime = data(index(StoryboardItem::FrameNumber, 0, parentIndex)).toInt();
    int nextKeyframeTime = nextKeyframeGlobal(currentKeyframeTime);

    if (nextKeyframeTime != INT_MAX) {
        int timeInFrame = nextKeyframeTime - currentKeyframeTime;

        int fps = m_image->animationInterface()->framerate();

        if (index (StoryboardItem::DurationSecond, 0, parentIndex).data().toInt() != timeInFrame / fps) {
            setData (index (StoryboardItem::DurationSecond, 0, parentIndex), timeInFrame / fps);
        }
        if (index (StoryboardItem::DurationFrame, 0, parentIndex).data().toInt() != timeInFrame % fps) {
            setData (index (StoryboardItem::DurationFrame, 0, parentIndex), timeInFrame % fps);
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
            StoryboardItemSP newItem = toQShared(new StoryboardItem());
            m_items.insert(position, newItem);
        }
        endInsertRows();
        emit(sigStoryboardItemListChanged());
        return true;
    }
    else if (!parent.parent().isValid()) {              //insert 2nd level nodes
        StoryboardItemSP item = m_items.at(parent.row());

        if (position < 0 || position > item->childCount()) {
            return false;
        }
        beginInsertRows(parent, position, position+rows-1);
        for (int row = 0; row < rows; ++row) {
            item->insertChild(position, QVariant());
        }
        endInsertRows();
        emit(sigStoryboardItemListChanged());
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
            m_items.removeAt(row);
        }
        endRemoveRows();
        emit(sigStoryboardItemListChanged());
        return true;
    }
    else if (!parent.parent().isValid()) {                     //remove 2nd level nodes
        StoryboardItemSP item = m_items.at(parent.row());

        if (position < 0 || position >= item->childCount()) {
            return false;
        }
        if (m_items.contains(item)) {
            beginRemoveRows(parent, position, position+rows-1);
            for (int row = 0; row < rows; ++row) {
                item->removeChild(position);
            }
            endRemoveRows();
            emit(sigStoryboardItemListChanged());
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
            if (sourceRow < StoryboardItem::Comments || sourceRow >= rowCount(parent)) {
                return false;
            }
            if (destinationChild + row < StoryboardItem::Comments || destinationChild + row >= rowCount(parent)) {
                return false;
            }

            StoryboardItemSP item = m_items.at(parent.row());
            item->moveChild(sourceRow, destinationChild + row);
        }
        endMoveRows();
        emit(sigStoryboardItemListChanged());
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
        emit(sigStoryboardItemListChanged());
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
    if (m_image) {
        m_image->disconnect(this);
        m_image->animationInterface()->disconnect(this);
    }
    m_image = image;
    m_renderScheduler->setImage(m_image);
    m_imageIdleWatcher.setTrackedImage(m_image);

    if (!image) {
        return;
    }

    //setting image to a different image stops rendering of all frames previously scheduled.
    //resetData() must be called before setImage(KisImageWSP) so that we can schedule rendering for the items in the new KisDocument
    foreach (StoryboardItemSP item, m_items) {
        int frame = qvariant_cast<ThumbnailData>(item->child(StoryboardItem::FrameNumber)->data()).frameNum.toInt();
        m_renderScheduler->scheduleFrameForRegeneration(frame,true);
    }

    m_imageIdleWatcher.startCountdown();
    connect(&m_imageIdleWatcher, SIGNAL(startedIdleMode()), m_renderScheduler, SLOT(slotStartFrameRendering()));

    connect(m_image, SIGNAL(sigImageUpdated(const QRect &)), &m_renderSchedulingCompressor, SLOT(start()));

    connect(m_image, SIGNAL(sigRemoveNodeAsync(KisNodeSP)), this, SLOT(slotNodeRemoved(KisNodeSP)));

    //for add, remove and move
    connect(m_image->animationInterface(), SIGNAL(sigKeyframeAdded(const KisKeyframeChannel*,int)),
            this, SLOT(slotKeyframeAdded(const KisKeyframeChannel*,int)), Qt::UniqueConnection);
    connect(m_image->animationInterface(), SIGNAL(sigKeyframeRemoved(const KisKeyframeChannel*,int)),
            this, SLOT(slotKeyframeRemoved(const KisKeyframeChannel*,int)), Qt::UniqueConnection);
    connect(m_image->animationInterface(), SIGNAL(sigKeyframeMoved(const KisKeyframeChannel*, int, int)),
            this, SLOT(slotKeyframeMoved(const KisKeyframeChannel*, int, int)), Qt::UniqueConnection);

    //for selection sync with timeline
    slotCurrentFrameChanged(m_image->animationInterface()->currentUITime());
    connect(m_image->animationInterface(), SIGNAL(sigUiTimeChanged(int)), this, SLOT(slotCurrentFrameChanged(int)), Qt::UniqueConnection);
    connect(m_view->selectionModel(), SIGNAL(selectionChanged(QItemSelection, QItemSelection)),
            this, SLOT(slotChangeFrameGlobal(QItemSelection, QItemSelection)), Qt::UniqueConnection);
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

QModelIndexList StoryboardModel::affectedIndexes(KisTimeSpan range) const
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

bool StoryboardModel::isOnlyKeyframe(KisNodeSP keyframeNode, int time) const
{
    bool onlyKeyframe = true;
    KisNodeSP node = m_image->rootLayer();
    while (node) {
        KisLayerUtils::recursiveApplyNodes(node,
                                            [keyframeNode, &onlyKeyframe, time] (KisNodeSP node) {
                                            if (node->isAnimated()) {
                                                auto keyframeMap = node->keyframeChannels();
                                                for (auto elem: keyframeMap) {
                                                    KisKeyframeChannel *keyframeChannel = elem;
                                                    bool keyframeAbsent = keyframeChannel->keyframeAt(time).isNull();
                                                    if (node != keyframeNode) {
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

            int nextKeyframeTimeQuery = keyframeChannel->nextKeyframeTime(keyframeTime);
            if (keyframeChannel->keyframeAt(nextKeyframeTimeQuery)) {
                if (nextKeyframeTime == INT_MAX) {
                    nextKeyframeTime = nextKeyframeTimeQuery;
                } else {
                    nextKeyframeTime = qMin(nextKeyframeTime, nextKeyframeTimeQuery);
                }
            }
        }
    });
    }

    return nextKeyframeTime;
}

bool StoryboardModel::insertHoldFramesAfter(int newDuration, int oldDuration, QModelIndex index)
{
    if (!index.isValid()) {
        return false;
    }
    int frame = index.siblingAtRow(StoryboardItem::FrameNumber).data().toInt();
    int fps = m_image.isValid() ? m_image->animationInterface()->framerate(): 24;

    if (newDuration < 0) {
        if (index.row() == StoryboardItem::DurationFrame
        && index.siblingAtRow(StoryboardItem::DurationSecond).data().toInt() > 0) {

            int durationSecond = index.siblingAtRow(StoryboardItem::DurationSecond).data().toInt();
            insertHoldFramesAfter(fps + newDuration, 0, index);
            insertHoldFramesAfter(durationSecond - 1, durationSecond, index.siblingAtRow(StoryboardItem::DurationSecond));
        }
        else {
            return false;
        }
    }

    //if the current keyframe is last keyframe globally, only set data in model
    if (nextKeyframeGlobal(frame) == INT_MAX) {
        setData(index, newDuration);
        return true;
    }

    if (index.row() == StoryboardItem::DurationSecond) {
        newDuration *= fps;
        oldDuration *= fps;
    }
    int durationChange = newDuration - oldDuration;
    if (durationChange == 0) {
        return false;
    }
    //minimum duration is 0s 1f
    if (index.row() ==StoryboardItem::DurationFrame && newDuration < 1 
        && index.siblingAtRow(StoryboardItem::DurationSecond).data().toInt() == 0) {
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
                    int timeIteration = keyframeChannel->lastKeyframeTime();
                    while (keyframeChannel->keyframeAt(timeIteration) && keyframeChannel->keyframeAt(timeIteration) != keyframeChannel->activeKeyframeAt(frame)) {
                        keyframeChannel->moveKeyframe(timeIteration, timeIteration + durationChange);
                        timeIteration = keyframeChannel->previousKeyframeTime(timeIteration);
                    }
                }
                else if (durationChange < 0) {
                    int timeIteration = keyframeChannel->nextKeyframeTime(frame);
                    const int minTime = frame + 1;
                    while (keyframeChannel->keyframeAt(timeIteration)) {
                        const int newTimeValue = qMax(minTime, timeIteration + durationChange);
                        keyframeChannel->moveKeyframe(timeIteration, newTimeValue);
                        timeIteration = keyframeChannel->nextKeyframeTime(newTimeValue);
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

    //disable for vector layers
    if (!m_activeNode->paintDevice()) {
        return false;
    }

    KisKeyframeChannel* keyframeChannel = m_activeNode->paintDevice()->keyframeChannel();
    if (!index.isValid()) {

        int lastKeyframeTime = 0;
        KisNodeSP node = m_image->rootLayer();
        if (node) {
        KisLayerUtils::recursiveApplyNodes (node, [&lastKeyframeTime] (KisNodeSP node)
        {
            if (node->isAnimated()) {
                lastKeyframeTime = qMax(lastKeyframeTime, node->paintDevice()->keyframeChannel()->lastKeyframeTime());
            }
        });
        }

        QModelIndex lastIndex = this->index(rowCount() - 1, 0);
        if (!keyframeChannel) {
            keyframeChannel = m_activeNode->getKeyframeChannel(KisKeyframeChannel::Raster.id(), true);
            slotKeyframeAdded(keyframeChannel, 0);
        }

        //insert keyframe after the last storyboard item
        if (lastIndex.isValid()) {
            insertItem(lastIndex, true);
        }
    }
    else {
        if (!keyframeChannel) {
            keyframeChannel = m_activeNode->getKeyframeChannel(KisKeyframeChannel::Raster.id(), true);
            slotKeyframeAdded(keyframeChannel, 0);
        }

        int frame = this->index(StoryboardItem::FrameNumber, 0, index).data().toInt();
        QModelIndex frameIndex = this->index(StoryboardItem::DurationFrame, 0, index);

        if (after) {
            int fps = m_image->animationInterface()->framerate();
            int durationInFrame = frameIndex.data().toInt() + fps * frameIndex.siblingAtRow(StoryboardItem::DurationSecond).data().toInt();

            //if this is the last keyframe globally don't insert hold frames
            if (nextKeyframeGlobal(frame) == INT_MAX) {
                keyframeChannel->addKeyframe(frame + qMax(1, durationInFrame));
            }
            else {
            //move keyframes to right by 1 and insert keyframe
                insertHoldFramesAfter(frameIndex.data().toInt() + 1, frameIndex.data().toInt(), frameIndex);
                keyframeChannel->addKeyframe(frame + qMax(1, durationInFrame));
            }
        }
        else {
            insertHoldFramesAfter(frameIndex.data().toInt() + 1, frameIndex.data().toInt(), this->index(StoryboardItem::DurationFrame, 0, index.siblingAtRow(index.row() - 1)));
            keyframeChannel->addKeyframe(frame);
        }
    }
    return true;
}

void StoryboardModel::resetData(StoryboardItemList list)
{
    beginResetModel();
    m_items = list;
    endResetModel();
}

StoryboardItemList StoryboardModel::getData()
{
    return m_items;
}

void StoryboardModel::slotCurrentFrameChanged(int frameId)
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

void StoryboardModel::slotKeyframeAdded(const KisKeyframeChannel* channel, int time)
{
    if (!indexFromFrame(time).isValid() && !isLocked()) {
        int frame = time;
        int prevItemRow = lastIndexBeforeFrame(frame).row();

        insertRows(prevItemRow + 1, 1);
        setData (index (StoryboardItem::FrameNumber, 0, index(prevItemRow + 1, 0)), frame);

        //default value for item corresponding to last keyframe is 0s 1f
        if (nextKeyframeGlobal(frame) == INT_MAX) {
            setData (index (StoryboardItem::DurationSecond, 0, index(prevItemRow + 1, 0)), 0);
            setData (index (StoryboardItem::DurationFrame, 0, index(prevItemRow + 1, 0)), 1);
        }
        updateDurationData(index(prevItemRow + 1, 0));
        updateDurationData(index(prevItemRow, 0));
        m_view->setCurrentItem(frame);
    }

    slotUpdateThumbnailForFrame(time, false);
}

void StoryboardModel::slotKeyframeRemoved(const KisKeyframeChannel *channel, int time)
{
    QModelIndex itemIndex = indexFromFrame(time);
    if (itemIndex.isValid()) {
        if (isOnlyKeyframe(channel->node().toStrongRef(), time)) {
            removeRows(itemIndex.row(), 1);
            m_renderScheduler->cancelFrameRendering(itemIndex.row());
            updateDurationData(lastIndexBeforeFrame(time));
        }
    }
}

void StoryboardModel::slotKeyframeMoved(const KisKeyframeChannel* channel, int from, int to)
{
    KisKeyframeSP keyframe = channel->keyframeAt(to);
    KIS_ASSERT(keyframe);

    QModelIndex fromIndex = indexFromFrame(from);
    if (fromIndex.isValid()) {
        //check whether there are keyframes at the "from" time in other nodes
        bool onlyKeyframe = isOnlyKeyframe(channel->node().toStrongRef(), from);

        int toItemRow = lastIndexBeforeFrame(to).row();
        QModelIndex destinationIndex = indexFromFrame(to);

        if (onlyKeyframe && !destinationIndex.isValid()) {
            setData(index(StoryboardItem::FrameNumber, 0, fromIndex), to);
            moveRows(QModelIndex(), fromIndex.row(), 1, QModelIndex(), toItemRow + 1);

            updateDurationData(indexFromFrame(to));
            updateDurationData(lastIndexBeforeFrame(to));

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
            setData(index(StoryboardItem::FrameNumber, 0, destinationIndex), to);

            QModelIndex fromIndex = indexFromFrame(from);
            for (int i=1; i < rowCount(destinationIndex); i++) {
                setData(index(i, 0, destinationIndex), index(i, 0, fromIndex).data());
            }

            updateDurationData(indexFromFrame(to));
            updateDurationData(lastIndexBeforeFrame(to));
        }
        slotUpdateThumbnailForFrame(to);
    }
}

void StoryboardModel::slotNodeRemoved(KisNodeSP node)
{ 
    if (node->isAnimated() && node->paintDevice()) {
        KisKeyframeChannel *channel = node->paintDevice()->keyframeChannel();
        int keyframeTime = channel->firstKeyframeTime();
        while (channel->keyframeAt(keyframeTime)) {
            //sigKeyframeRemoved is not emitted when parent node is deleted so calling explicitly
            slotKeyframeRemoved(channel, keyframeTime);
            keyframeTime = channel->nextKeyframeTime(keyframeTime);
        }
    }
}

void StoryboardModel::slotUpdateThumbnailForFrame(int frame, bool delay)
{
    if (!m_image) {
        return;
    }

    QModelIndex index = indexFromFrame(frame);
    bool affected = true;
    if (index.isValid()) {
        if (frame == m_image->animationInterface()->currentUITime()) {
            if(!delay) {
                setThumbnailPixmapData(index, m_image->projection());
                return;
            }
            else {
                affected = false;
            }
        }
        m_renderScheduler->scheduleFrameForRegeneration(frame, affected);
    }
}

void StoryboardModel::slotUpdateThumbnails()
{
    if (!m_image) {
        return;
    }

    int currentTime = m_image->animationInterface()->currentUITime();
    slotUpdateThumbnailForFrame(currentTime);

    KisTimeSpan affectedRange;
    if (m_activeNode && m_activeNode->paintDevice()) {
        KisRasterKeyframeChannel *currentChannel = m_activeNode->paintDevice()->keyframeChannel();
        if (currentChannel) {
            affectedRange = currentChannel->affectedFrames(currentTime);
            if (affectedRange.isInfinite()) {
                int end = index(StoryboardItem::FrameNumber, 0, index(rowCount() - 1, 0)).data().toInt();
                affectedRange = KisTimeSpan::fromTimeToTime(affectedRange.start(), end);
            }
            QModelIndexList dirtyIndexes = affectedIndexes(affectedRange);
            foreach(QModelIndex index, dirtyIndexes) {
                int frame = this->index(StoryboardItem::FrameNumber, 0, index).data().toInt();
                slotUpdateThumbnailForFrame(frame);
            }
        }
        else {
            affectedRange = KisTimeSpan::infinite(0);
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
            const int currentKeyframeTime = m_activeNode->paintDevice()->keyframeChannel()->activeKeyframeTime(data(index(StoryboardItem::FrameNumber, 0, parentIndex)).toInt());
            int nextKeyframeTime = nextKeyframeGlobal(currentKeyframeTime);

            if (nextKeyframeTime == INT_MAX) {
                setData (index (2, 0, parentIndex), 0);
                setData (index (3, 0, parentIndex), 0);
            }
            else {
                int timeInFrame = nextKeyframeTime - currentKeyframeTime - 1;

                int fps = m_image->animationInterface()->framerate();
                setData (index (StoryboardItem::DurationSecond, 0, parentIndex), timeInFrame / fps);
                setData (index (StoryboardItem::DurationFrame, 0, parentIndex), timeInFrame % fps);
            }
        }
    }
}
