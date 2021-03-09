/*
 *  SPDX-FileCopyrightText: 2020 Saurabh Kumar <saurabhk660@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
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
#include <kis_post_execution_undo_adapter.h>
#include "kis_time_span.h"
#include "commands_new/kis_switch_current_time_command.h"
#include "kis_raster_keyframe_channel.h"
#include "KisStoryboardThumbnailRenderScheduler.h"
#include "KisAddRemoveStoryboardCommand.h"

StoryboardModel::StoryboardModel(QObject *parent)
        : QAbstractItemModel(parent)
        , m_freezeKeyframePosition(false)
        , m_lockBoards(false)
        , m_reorderingKeyframes(false)
        , m_imageIdleWatcher(10)
        , m_renderScheduler(new KisStoryboardThumbnailRenderScheduler(this))
        , m_renderSchedulingCompressor(1000,KisSignalCompressor::FIRST_ACTIVE)
{
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
        if (role == TotalSceneDurationInFrames) {
            int duration = this->index(StoryboardItem::DurationFrame, 0, index).data().toInt()
                + this->index(StoryboardItem::DurationSecond, 0, index).data().toInt()
                * getFramesPerSecond();
            return duration;
        }
        else if (role == TotalSceneDurationInSeconds) {
            qreal duration = this->index(StoryboardItem::DurationSecond, 0, index).data().toInt()
                + this->index(StoryboardItem::DurationFrame, 0, index).data().toInt()
                / getFramesPerSecond();
            return duration;
        }
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
    if (index.isValid() && !isLocked() && (role == Qt::EditRole || role == Qt::DisplayRole)) {
        if (!index.parent().isValid()) {
            return false;
        }

        QSharedPointer<StoryboardChild> child = m_items.at(index.parent().row())->child(index.row());
        if (child) {
            if (index.row() == StoryboardItem::FrameNumber && !value.canConvert<ThumbnailData>()) {
                if (value.toInt() < 0) {
                    return false;
                }
                ThumbnailData thumbnailData = qvariant_cast<ThumbnailData>(child->data());
                thumbnailData.frameNum = value.toInt();
                child->setData(QVariant::fromValue<ThumbnailData>(thumbnailData));
            }
            else if (index.row() == StoryboardItem::DurationSecond ||
                     index.row() == StoryboardItem::DurationFrame) {
                ENTER_FUNCTION() << ppVar(index) << ppVar(value);
#if QT_VERSION >= QT_VERSION_CHECK(5,11,0)
                QModelIndex secondIndex = index.row() == StoryboardItem::DurationSecond ? index : index.siblingAtRow(StoryboardItem::DurationSecond);
#else
                QModelIndex secondIndex = index.row() == StoryboardItem::DurationSecond ? index : index.sibling(StoryboardItem::DurationSecond, 0);
#endif
                const int secondCount = index.row() == StoryboardItem::DurationSecond ? value.toInt() : secondIndex.data().toInt();
#if QT_VERSION >= QT_VERSION_CHECK(5,11,0)
                QModelIndex frameIndex = index.row() == StoryboardItem::DurationFrame ? index : index.siblingAtRow(StoryboardItem::DurationFrame);
#else
                QModelIndex frameIndex = index.row() == StoryboardItem::DurationFrame ? index : index.sibling(StoryboardItem::DurationFrame, 0);
#endif
                const int frameCount = index.row() == StoryboardItem::DurationFrame ? value.toInt() : frameIndex.data().toInt();
#if QT_VERSION >= QT_VERSION_CHECK(5,11,0)
                const int sceneStartFrame = index.siblingAtRow(StoryboardItem::FrameNumber).data().toInt();
#else
                const int sceneStartFrame = index.sibling(StoryboardItem::FrameNumber, 0).data().toInt();
#endif
                // Do not allow desired scene length to be shorter than keyframes within
                // the given scene. This prevents overwriting data that exists internal
                // to a scene.
                const int sceneDesiredDuration = frameCount + secondCount * getFramesPerSecond();
                const int implicitSceneDuration = qMax(
                                                  qMax( sceneDesiredDuration, lastKeyframeWithin(index.parent()) - sceneStartFrame + 1 ),
                                                  1 );

                if (value.toInt() < 0 && secondIndex.data().toInt() == 0) {
                    return false;
                }
                if (implicitSceneDuration == data(index.parent(), TotalSceneDurationInFrames).toInt()) {
                    return false;
                }
                const int fps = m_image.isValid() ? m_image->animationInterface()->framerate() : 24;

                QModelIndex lastScene = index.parent();
                QModelIndex nextScene = this->index(lastScene.row() + 1, 0);
                changeSceneHoldLength(implicitSceneDuration, index.parent());
                while (nextScene.isValid()) {
                    const int lastSceneStartFrame = this->index(StoryboardItem::FrameNumber, 0, lastScene).data().toInt();
                    const int lastSceneDuration = lastScene == index.parent() ? implicitSceneDuration
                                                                              : data(lastScene, TotalSceneDurationInFrames).toInt();
                    setData( this->index(StoryboardItem::FrameNumber, 0, nextScene), lastSceneStartFrame + lastSceneDuration);
                    lastScene = nextScene;
                    nextScene = this->index(lastScene.row() + 1, 0);
                }

                StoryboardItemSP scene = m_items.at(index.parent().row());
                QSharedPointer<StoryboardChild> durationSeconds = scene->child(secondIndex.row());
                QSharedPointer<StoryboardChild> durationFrames = scene->child(frameIndex.row());
                durationSeconds->setData(QVariant::fromValue<int>(implicitSceneDuration / fps));
                durationFrames->setData(QVariant::fromValue<int>(implicitSceneDuration % fps));

            }
            else if (index.row() >= StoryboardItem::Comments && !value.canConvert<CommentBox>()) {
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

bool StoryboardModel::updateDurationData(const QModelIndex& parentIndex)
{
    if (!parentIndex.isValid()) {
        return false;
    }

    QModelIndex currentScene = parentIndex;
    QModelIndex nextScene = index(currentScene.row() + 1, 0);
    if (nextScene.isValid()) {
        const int currentSceneFrame = index(StoryboardItem::FrameNumber, 0, currentScene).data().toInt();
        const int nextSceneFrame = index(StoryboardItem::FrameNumber, 0, nextScene).data().toInt();
        const int sceneDuration = nextSceneFrame - currentSceneFrame;
        const int fps = getFramesPerSecond();

        if (index(StoryboardItem::DurationSecond, 0, parentIndex).data().toInt() != sceneDuration / fps) {
            setData (index (StoryboardItem::DurationSecond, 0, parentIndex), sceneDuration / fps);
        }
        if (index(StoryboardItem::DurationFrame, 0, parentIndex).data().toInt() != sceneDuration % fps) {
            setData (index (StoryboardItem::DurationFrame, 0, parentIndex), sceneDuration % fps);
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

        if (isLocked()) {
            return false;
        }

        beginInsertRows(QModelIndex(), position, position+rows-1);
        for (int row = 0; row < rows; ++row) {
            StoryboardItemSP newItem = toQShared(new StoryboardItem());
            m_items.insert(position + row, newItem);
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

        if (isLocked()) {
            return false;
        }

        beginRemoveRows(QModelIndex(), position, position+rows-1);

        for (int row = position + rows - 1; row >= position; row--) {
            m_items.removeAt(row);
            if (m_items.count() == 0) {
                break;
            }
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

bool StoryboardModel::moveRows(const QModelIndex &sourceParent, int sourceRow, int count, const QModelIndex &destinationParent, int destinationChild)
{
    KisMoveStoryboardCommand *command = new KisMoveStoryboardCommand(sourceRow, count, destinationChild, this);

    const int sourceFrame = index(StoryboardItem::FrameNumber, 0, index(sourceRow, 0)).data().toInt();

    if (moveRowsImpl(sourceParent, sourceRow, count, destinationParent, destinationChild)) {
        const int actualIndex = sourceRow < destinationChild ? destinationChild - 1 : destinationChild;
        const int destinationFrame = index(StoryboardItem::FrameNumber, 0, index(actualIndex, 0)).data().toInt();

        if (m_image) {
            KisSwitchCurrentTimeCommand* switchCommand = new KisSwitchCurrentTimeCommand(m_image->animationInterface(), sourceFrame, destinationFrame, command);
            switchCommand->redo();
        }

        pushUndoCommand(command);
        return true;
    }
    return false;
}

QStringList StoryboardModel::mimeTypes() const
{
    QStringList types;
    types << QLatin1String("application/x-krita-storyboard");
    return types;
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

    mimeData->setData("application/x-krita-storyboard", encodeData); //default mimetype
    return mimeData;
}

bool StoryboardModel::dropMimeData(const QMimeData *data, Qt::DropAction action,
                                int row, int column, const QModelIndex &parent)
{
    Q_UNUSED(column);
    if (action == Qt::IgnoreAction) {
        return false;
    }

    if (action == Qt::MoveAction && data->hasFormat("application/x-krita-storyboard")) {
        QByteArray bytes = data->data("application/x-krita-storyboard");
        QDataStream stream(&bytes, QIODevice::ReadOnly);

        if (parent.isValid()) {
            return false;
        }

        if (isLocked()) {
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
    foreach(StoryboardComment comment, m_commentList) {
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

void StoryboardModel::setCommentModel(StoryboardCommentModel *commentModel)
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

StoryboardComment StoryboardModel::getComment(int row) const
{
    return m_commentList.at(row);
}

void StoryboardModel::setFreeze(bool value)
{
    m_freezeKeyframePosition = value;
}

bool StoryboardModel::isFrozen() const
{
    return m_freezeKeyframePosition;
}

void StoryboardModel::setLocked(bool value)
{
    m_lockBoards = value;
}

bool StoryboardModel::isLocked() const
{
    return m_lockBoards;
}

int StoryboardModel::getFramesPerSecond() const
{
    return m_image.isValid() ? m_image->animationInterface()->framerate() : 24;
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
    m_lastScene = m_items.size();

    m_imageIdleWatcher.startCountdown();
    connect(&m_imageIdleWatcher, SIGNAL(startedIdleMode()), m_renderScheduler, SLOT(slotStartFrameRendering()));

    connect(m_image, SIGNAL(sigImageUpdated(const QRect &)), &m_renderSchedulingCompressor, SLOT(start()));

    connect(m_image, SIGNAL(sigRemoveNodeAsync(KisNodeSP)), this, SLOT(slotNodeRemoved(KisNodeSP)));

    //for add, remove and move
    connect(m_image->animationInterface(), SIGNAL(sigKeyframeAdded(const KisKeyframeChannel*,int)),
            this, SLOT(slotKeyframeAdded(const KisKeyframeChannel*,int)), Qt::UniqueConnection);
    connect(m_image->animationInterface(), SIGNAL(sigKeyframeRemoved(const KisKeyframeChannel*,int)),
            this, SLOT(slotKeyframeRemoved(const KisKeyframeChannel*,int)), Qt::UniqueConnection);

    connect(m_image->animationInterface(), SIGNAL(sigFramerateChanged()), this, SLOT(slotFramerateChanged()), Qt::UniqueConnection);

    //for selection sync with timeline
    slotCurrentFrameChanged(m_image->animationInterface()->currentUITime());
    connect(m_image->animationInterface(), SIGNAL(sigUiTimeChanged(int)), this, SLOT(slotCurrentFrameChanged(int)), Qt::UniqueConnection);
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
        QModelIndex childIndex = index(StoryboardItem::FrameNumber, 0, parentIndex);
        if (childIndex.data().toInt() == frame) {
            return parentIndex;
        } else if (childIndex.data().toInt() > frame) {
            end = row - 1;
        } else if (childIndex.data().toInt() < frame) {
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
#if QT_VERSION >= QT_VERSION_CHECK(5,11,0)
        firstIndex = firstIndex.siblingAtRow(firstIndex.row() + 1);
#else
        firstIndex = firstIndex.sibling(firstIndex.row() + 1, 0);
#endif
    }
    else {
        firstIndex = lastIndexBeforeFrame(range.start());
#if QT_VERSION >= QT_VERSION_CHECK(5,11,0)
        firstIndex = firstIndex.sibling(firstIndex.row() + 1, 0);
#else
#endif
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

int StoryboardModel::lastKeyframeGlobal() const
{
    if (!m_image)
        return 0;

    KisNodeSP node = m_image->rootLayer();
    int lastKeyframeTime = 0;
    if (node) {
    KisLayerUtils::recursiveApplyNodes (node, [&lastKeyframeTime] (KisNodeSP node)
    {
        if (node->isAnimated()) {
            KisKeyframeChannel *keyframeChannel = node->paintDevice()->keyframeChannel();

            if (!keyframeChannel)
                return;

            lastKeyframeTime = qMax(keyframeChannel->lastKeyframeTime(), lastKeyframeTime);
        }
    });
    }

    return lastKeyframeTime;
}

int StoryboardModel::lastKeyframeWithin(QModelIndex sceneIndex)
{
    KIS_ASSERT(sceneIndex.isValid());
    const int sceneFrame = index(StoryboardItem::FrameNumber, 0, sceneIndex).data().toInt();

    if (!m_image)
        return sceneFrame;
    
    QModelIndex nextScene = index(sceneIndex.row() + 1, 0);
    int nextSceneFrame;
    if (nextScene.isValid()) {
        nextSceneFrame = data(index(StoryboardItem::FrameNumber, 0, nextScene)).toInt();
    }
    else {
        nextSceneFrame = sceneFrame + data(sceneIndex, TotalSceneDurationInFrames).toInt();
    }

    int lastFrameOfScene = sceneFrame;
    for (int frame = sceneFrame; frame < nextSceneFrame; frame = nextKeyframeGlobal(frame)) {
        lastFrameOfScene = frame;
    }

    return lastFrameOfScene;
}

void StoryboardModel::reorderKeyframes()
{
    //Get the earliest frame number in the storyboard list
    int earliestFrame = INT_MAX;

    typedef int AssociateFrameOffset;

    QMultiHash<QModelIndex, AssociateFrameOffset> frameAssociates;

    for (int i = 0; i < rowCount(); i++) {
        QModelIndex sceneIndex = index(i, 0);
        int sceneFrame = index(StoryboardItem::FrameNumber, 0, sceneIndex).data().toInt();
        earliestFrame = sceneFrame < earliestFrame ? sceneFrame : earliestFrame;
        frameAssociates.insert(sceneIndex, 0);

        const int lastFrameOfScene = index(StoryboardItem::FrameNumber, 0, sceneIndex).data().toInt()
                                     + data(sceneIndex, TotalSceneDurationInFrames).toInt();

        for( int i = sceneFrame; i < lastFrameOfScene; i++) {
            frameAssociates.insert(sceneIndex, i - sceneFrame);
        }
    }

    if (earliestFrame == INT_MAX) {
        return;
    }

    //We want to temporarily lock respondance to keyframe removal / addition.
    //Will unlock when scope exits.
    QScopedPointer<KeyframeReorderLock> lock(new KeyframeReorderLock(this));

    //Let's cancel all frame rendering for the time being.
    m_renderScheduler->cancelAllFrameRendering();

    KisNodeSP root = m_image->root();
    if (root && !m_freezeKeyframePosition) {
        KisLayerUtils::recursiveApplyNodes(root, [this, earliestFrame, frameAssociates](KisNodeSP node) {
            if (!node->isAnimated() || !node->paintDevice())
                return;

            KisRasterKeyframeChannel* rasterChan = node->paintDevice()->keyframeChannel();

            if (!rasterChan)
                return;

            // Gather all original keyframes and their associated time values.
            QHash<int, KisKeyframeSP> originalKeyframes;
            Q_FOREACH( const int& time, rasterChan->allKeyframeTimes()) {
                if (time >= earliestFrame && rasterChan->keyframeAt(time)) {
                    originalKeyframes.insert(time, rasterChan->keyframeAt(time));
                    rasterChan->removeKeyframe(time);
                }
            }

            //Now lets re-sort the raster channels...
            int intendedSceneFrameTime = earliestFrame;
            for (int i = 0; i < rowCount(); i++) {
                QModelIndex sceneIndex = index(i, 0);
                const int srcFrame = index(StoryboardItem::FrameNumber, 0, sceneIndex).data().toInt();
                Q_FOREACH( const int& associateFrameOffset, frameAssociates.values(sceneIndex) ) {
                    if (!originalKeyframes.contains(srcFrame + associateFrameOffset))
                        continue;

                    rasterChan->insertKeyframe(intendedSceneFrameTime + associateFrameOffset,
                                               originalKeyframes.value(srcFrame + associateFrameOffset));
                }

                intendedSceneFrameTime += data(sceneIndex, TotalSceneDurationInFrames).toInt();
            }
        });
    }

    //Lastly, let's update all of the frame values.
    int intendedFrameValue = earliestFrame;
    for (int i = 0; i < rowCount(); i++) {
        QModelIndex sceneIndex = index(i, 0);
        setData(index(StoryboardItem::FrameNumber, 0, sceneIndex), intendedFrameValue);
        slotUpdateThumbnailForFrame(intendedFrameValue);
        intendedFrameValue += data(sceneIndex, TotalSceneDurationInFrames).toInt();
    }

    m_renderScheduler->slotStartFrameRendering();
}

bool StoryboardModel::changeSceneHoldLength(int newDuration, QModelIndex itemIndex)
{
    if (!itemIndex.isValid()) {
        return false;
    }

    const int origSceneFrameLength = data(itemIndex, TotalSceneDurationInFrames).toInt();
    const int lastFrameOfScene = lastKeyframeWithin(itemIndex);

    int durationChange = newDuration - origSceneFrameLength;
    if (durationChange == 0) {
        return false;
    }

    if (origSceneFrameLength != 0) {
        shiftKeyframes(KisTimeSpan::infinite(lastFrameOfScene + 1), durationChange);
    }

    return true;
}

void StoryboardModel::shiftKeyframes(KisTimeSpan affected, int offset, KUndo2Command *cmd) {
    if (!m_image)
        return;

    KisNodeSP node = m_image->rootLayer();

    if (offset == 0)
        return;

    //We want to temporarily lock respondance to keyframe removal / addition.
    //Will unlock when scope exits.
    QScopedPointer<KeyframeReorderLock> lock(new KeyframeReorderLock(this));

    if (node && !m_freezeKeyframePosition) {
        KisLayerUtils::recursiveApplyNodes (node, [affected, offset, cmd] (KisNodeSP node) {
                const int startFrame = affected.start();
                if (node->isAnimated()) {
                    KisKeyframeChannel *keyframeChannel = node->paintDevice()->keyframeChannel();
                    if (keyframeChannel) {
                        if (offset > 0) {
                            int timeIter = affected.isInfinite() ?
                                                    keyframeChannel->lastKeyframeTime()
                                                  : keyframeChannel->activeKeyframeTime(affected.end());

                            KisKeyframeSP iterEnd = keyframeChannel->keyframeAt(keyframeChannel->previousKeyframeTime(startFrame));

                            while (keyframeChannel->keyframeAt(timeIter) &&
                                   keyframeChannel->keyframeAt(timeIter) != iterEnd) {
                                keyframeChannel->moveKeyframe(timeIter, timeIter + offset, cmd);
                                timeIter = keyframeChannel->previousKeyframeTime(timeIter);
                            }

                        } else {
                            int timeIter = keyframeChannel->keyframeAt(startFrame) ? startFrame : keyframeChannel->nextKeyframeTime(startFrame);

                            KisKeyframeSP iterEnd = affected.isInfinite() ?
                                                        nullptr
                                                      : keyframeChannel->keyframeAt(keyframeChannel->nextKeyframeTime(affected.end()));

                            while (keyframeChannel->keyframeAt(timeIter) != iterEnd) {
                                keyframeChannel->moveKeyframe(timeIter, timeIter + offset, cmd);
                                timeIter = keyframeChannel->nextKeyframeTime(timeIter);
                            }
                        }
                    }
                }
            });
    }
}

bool StoryboardModel::insertItem(QModelIndex index, bool after)
{
    //index is the index at which context menu was created, or the + button belongs to
    //after is whether we want the item before or after index

    //disable for vector layers
    if (!m_activeNode->paintDevice()) {
        return false;
    }

    int desiredIndex;
    if (!index.isValid()) {
        desiredIndex = rowCount();
    } else {
        desiredIndex = after ? index.row() + 1 : index.row();    
    }

    insertRow(desiredIndex);
    KisAddStoryboardCommand *command = new KisAddStoryboardCommand(desiredIndex, m_items.at(desiredIndex), this);
    insertChildRows(desiredIndex, command);
    const int currentTime = m_image->animationInterface()->currentTime();
    const int desiredTime = this->index(StoryboardItem::FrameNumber, 0, this->index(desiredIndex, 0)).data().toInt();

    if (m_image && currentTime != desiredTime) {
        KisSwitchCurrentTimeCommand *switchTimeCmd = new KisSwitchCurrentTimeCommand(m_image->animationInterface(),
                                                                                     currentTime,
                                                                                     desiredTime,
                                                                                     command);
        switchTimeCmd->redo();
    } else {
        m_view->setCurrentItem(currentTime);
    }

    pushUndoCommand(command);

    // Let's start rendering after adding new storyboard items.
    slotUpdateThumbnails();
    m_renderScheduler->slotStartFrameRendering();

    return true;
}

bool StoryboardModel::removeItem(QModelIndex index, KUndo2Command *command)
 {
    const int row = index.row();
    const int durationDeletedScene = data(index, TotalSceneDurationInFrames).toInt();

    //remove all keyframes within the scene with command as parent
    KisNodeSP node = m_image->rootLayer();
    const int firstFrameOfScene = data(this->index(StoryboardItem::FrameNumber, 0, index)).toInt();
    const int lastFrameOfScene = lastKeyframeWithin(index);
    if (command) {
        if (node) {
        KisLayerUtils::recursiveApplyNodes (node, [firstFrameOfScene, lastFrameOfScene, command] (KisNodeSP node){
            if (node->isAnimated()) {
                KisKeyframeChannel *keyframeChannel = node->paintDevice()->keyframeChannel();

                int timeIter = keyframeChannel->keyframeAt(firstFrameOfScene)
                                                            ? firstFrameOfScene
                                                            : keyframeChannel->nextKeyframeTime(firstFrameOfScene);

                while (keyframeChannel->keyframeAt(timeIter) && timeIter <= lastFrameOfScene) {
                    keyframeChannel->removeKeyframe(timeIter, command);
                    timeIter = keyframeChannel->nextKeyframeTime(timeIter);
                }
            }
        });
        }
        shiftKeyframes(KisTimeSpan::infinite(lastFrameOfScene), -durationDeletedScene, command);

        //If we're viewing the scene we're about the remove, let's jump back to the last valid scene.
        if (row > 0 && row <= rowCount()) {
            QModelIndex currentSceneFN = this->index(StoryboardItem::FrameNumber, 0, this->index(row, 0));
            if (m_image && m_image->animationInterface()->currentTime() == currentSceneFN.data().toInt()) {
                KisSwitchCurrentTimeCommand* switchTimeCmd = new KisSwitchCurrentTimeCommand(m_image->animationInterface(),
                                                                                             currentSceneFN.data().toInt(),
                                                                                             this->index(StoryboardItem::FrameNumber, 0, this->index(row-1, 0)).data().toInt(),
                                                                                             command);
                switchTimeCmd->redo();
            }
        }
    }

    removeRows(row, 1);
    for (int i = row; i < rowCount(); i++) {
        QModelIndex frameNumIndex = this->index(StoryboardItem::FrameNumber, 0, this->index(i, 0));
        setData(frameNumIndex, data(frameNumIndex).toInt() - durationDeletedScene);
    }

    //do we also need to make the 'change frame' command a child??
    // Render after removing
    slotUpdateThumbnails();
    m_renderScheduler->slotStartFrameRendering();

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

void StoryboardModel::pushUndoCommand(KUndo2Command* command)
{
    m_image->postExecutionUndoAdapter()->addCommand(toQShared(command));
}

void StoryboardModel::slotCurrentFrameChanged(int frameId)
{
    m_view->setCurrentItem(frameId);
}

void StoryboardModel::slotKeyframeAdded(const KisKeyframeChannel* channel, int time)
{
    if (m_reorderingKeyframes)
        return;

    const QModelIndex exactScene = indexFromFrame(time);
    const QModelIndex lastScene = lastIndexBeforeFrame(time);
    const QModelIndex nextScene = index( lastScene.row() + 1, 0);
    const bool extendsLastScene = !exactScene.isValid() && lastScene.isValid() && !nextScene.isValid();

    //Capture new keyframes after last scene and extend duration to include the new key.
    if (extendsLastScene) {
        const int sceneStartFrame = index(StoryboardItem::FrameNumber, 0, lastScene).data().toInt();
        const int desiredDuration = time - sceneStartFrame + 1;
        const int actualDuration = data(lastScene, TotalSceneDurationInFrames).toInt();
        const int duration = qMax(actualDuration, desiredDuration);
        KIS_ASSERT(duration > 0);
        const QSharedPointer<StoryboardChild> frameElement = m_items.at(lastScene.row())->child(StoryboardItem::DurationFrame);
        const QSharedPointer<StoryboardChild> secondElement = m_items.at(lastScene.row())->child(StoryboardItem::DurationSecond);
        frameElement->setData(QVariant::fromValue<int>(duration % getFramesPerSecond()));
        secondElement->setData(QVariant::fromValue<int>(duration / getFramesPerSecond()));

        emit dataChanged(lastScene, lastScene);
    }

    QModelIndexList affected = affectedIndexes(KisTimeSpan::fromTimeToTime(time, channel->nextKeyframeTime(time)));
    slotUpdateThumbnailsForItems(affected);
}

void StoryboardModel::slotKeyframeRemoved(const KisKeyframeChannel *channel, int time)
{
    if (m_reorderingKeyframes)
        return;

    QModelIndexList affected = affectedIndexes(KisTimeSpan::fromTimeToTime(channel->activeKeyframeTime(time), channel->nextKeyframeTime(time)));
    slotUpdateThumbnailsForItems(affected);
}

void StoryboardModel::slotNodeRemoved(KisNodeSP node)
{ 
    if (node->isAnimated() && node->paintDevice() && node->paintDevice()->keyframeChannel()) {
        KisKeyframeChannel *channel = node->paintDevice()->keyframeChannel();
        int keyframeTime = channel->firstKeyframeTime();
        while (channel->keyframeAt(keyframeTime)) {
            //sigKeyframeRemoved is not emitted when parent node is deleted so calling explicitly
            slotKeyframeRemoved(channel, keyframeTime);
            keyframeTime = channel->nextKeyframeTime(keyframeTime);
        }
    }

    slotUpdateThumbnails();
}

void StoryboardModel::slotFramerateChanged()
{
    QModelIndex sceneIndex = index(0,0);
    QModelIndex nextScene = index(1,0);
    if (nextScene.isValid()) {

        while (sceneIndex.isValid() && nextScene.isValid()) {
            StoryboardItemSP item = m_items.at(sceneIndex.row());
            const int nextSceneFrame = index(StoryboardItem::FrameNumber, 0, nextScene).data().toInt();
            const int sceneFrame = index(StoryboardItem::FrameNumber, 0, sceneIndex).data().toInt();
            const int duration = nextSceneFrame - sceneFrame;
            const int durationFrames = duration % getFramesPerSecond();
            const int durationSeconds = duration / getFramesPerSecond();

            item->child(StoryboardItem::DurationFrame)->setData(QVariant::fromValue<int>(durationFrames));
            item->child(StoryboardItem::DurationSecond)->setData(QVariant::fromValue<int>(durationSeconds));

            sceneIndex = nextScene;
            nextScene = index(sceneIndex.row() + 1, sceneIndex.column());
        }

        emit dataChanged(index(0,0), sceneIndex);
    } else if (sceneIndex.isValid()) {

        StoryboardItemSP item = m_items.at(sceneIndex.row());
        const int lastKeyframe = lastKeyframeGlobal();
        const int sceneFrame = index(StoryboardItem::FrameNumber, 0, sceneIndex).data().toInt();
        const int duration = lastKeyframe + 1 - sceneFrame;

        const int durationFrames = duration % getFramesPerSecond();
        const int durationSeconds = duration / getFramesPerSecond();

        item->child(StoryboardItem::DurationFrame)->setData(QVariant::fromValue<int>(durationFrames));
        item->child(StoryboardItem::DurationSecond)->setData(QVariant::fromValue<int>(durationSeconds));

        emit dataChanged(sceneIndex, sceneIndex);
    }
}

void StoryboardModel::slotUpdateThumbnailForFrame(int frame, bool delay)
{
    if (!m_image) {
        return;
    }

    QModelIndex index = indexFromFrame(frame);
    bool affected = true;
    if (index.isValid() && !isLocked()) {
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
        m_renderScheduler->slotStartFrameRendering();
    }
}

void StoryboardModel::slotUpdateThumbnailsForItems(QModelIndexList indices) {
    if (isLocked())
        return;

    Q_FOREACH( const QModelIndex& storyboardItemIndex, indices ) {
        if (!storyboardItemIndex.isValid())
            continue;

        //If this isn't a storyboard item (root) we will continue
        if (storyboardItemIndex.parent().isValid())
            continue;

        const int frame = index(StoryboardItem::FrameNumber, 0, storyboardItemIndex).data().toInt();
        slotUpdateThumbnailForFrame(frame, false);        
    }
}

void StoryboardModel::slotUpdateThumbnails()
{
    if (!m_image || isLocked()) {
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
        moveRowsImpl(parentIndex, start + 4, end - start + 1, parentIndex, destinationRow + 4);
    }
    slotCommentDataChanged();
}

void StoryboardModel::insertChildRows(int position, KUndo2Command *cmd)
{
    if (position + 1 < rowCount()) {
        const int frame = index(StoryboardItem::FrameNumber, 0, index(position + 1, 0)).data().toInt();
        shiftKeyframes(KisTimeSpan::infinite(frame), 1);
    }

    for (int row = position + 1; row < rowCount(); ++row) {
        const int frame = index(StoryboardItem::FrameNumber, 0, index(row, 0)).data().toInt();
        setData(index(StoryboardItem::FrameNumber, 0, index(row, 0)), frame + 1);
    }

    QModelIndex parentIndex = index(position, 0);
    insertRows(0, 4 + m_commentList.count(), parentIndex);

    m_lastScene++;
    QString sceneName = i18nc("default name for storyboard item", "scene ") + QString::number(m_lastScene);
    setData (index (StoryboardItem::ItemName, 0, parentIndex), sceneName);

    if (position == 0) {
        setData (index (StoryboardItem::FrameNumber, 0, index(position, 0)), 0);
        setData( index(StoryboardItem::DurationFrame, 0, index(position, 0)), lastKeyframeGlobal() - 0 + 1);
    } else {
        const int targetFrame = index(StoryboardItem::FrameNumber, 0, index(position - 1,0)).data().toInt()
                                + data(index(position - 1, 0), TotalSceneDurationInFrames).toInt();
        setData (index (StoryboardItem::FrameNumber, 0, index(position, 0)), targetFrame);

        if (!m_freezeKeyframePosition && m_activeNode) {
            KisKeyframeChannel* chan = m_activeNode->getKeyframeChannel(KisKeyframeChannel::Raster.id(), true);
            chan->addKeyframe(targetFrame, cmd);
        }
    }

    setData (index (StoryboardItem::DurationFrame, 0, parentIndex), 1);
    setData (index (StoryboardItem::DurationSecond, 0, parentIndex), 0);

    const int frameToSwitch = index(StoryboardItem::FrameNumber, 0, index(position, 0)).data().toInt();
    if (m_image) {
        KisSwitchCurrentTimeCommand* switchFrameCmd = new KisSwitchCurrentTimeCommand(m_image->animationInterface(), m_image->animationInterface()->currentTime(), frameToSwitch, cmd);
        switchFrameCmd->redo();
    }
}

void StoryboardModel::visualizeScene(const QModelIndex &scene)
{
    if (scene.parent().isValid() || !m_image) {
        return;
    }

    int frameTime = index(StoryboardItem::FrameNumber, 0, scene).data().toInt();

    if (frameTime != m_image->animationInterface()->currentTime()) {
        KisSwitchCurrentTimeCommand* cmd = new KisSwitchCurrentTimeCommand(m_image->animationInterface(), m_image->animationInterface()->currentTime(), frameTime);
        cmd->redo();
        pushUndoCommand(cmd);
    }
}

bool StoryboardModel::moveRowsImpl(const QModelIndex &sourceParent, int sourceRow, int count,
                                const QModelIndex &destinationParent, int destinationChild)
{
    if (sourceParent != destinationParent) {
        return false;
    }
    if (destinationChild == sourceRow || destinationChild == sourceRow + 1) {
        return false;
    }

    if (isLocked()) {
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

        reorderKeyframes();

        emit sigStoryboardItemListChanged();
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

        reorderKeyframes();

        emit sigStoryboardItemListChanged();
        return true;
    }
    else {
        return false;
    }
}

void StoryboardModel::insertChildRows(int position, StoryboardItemSP item)
{
    QModelIndex parentIndex = index(position, 0);
    insertRows(0, 4 + m_commentList.count(), parentIndex);

    setFreeze(true);
    for (int i = 0; i < item->childCount(); i++) {   
        QVariant data = item->child(i)->data();
        setData(index(i, 0, index(position, 0)), data);
    }
    setFreeze(false);
    slotUpdateThumbnails();
    m_renderScheduler->slotStartFrameRendering();
}
