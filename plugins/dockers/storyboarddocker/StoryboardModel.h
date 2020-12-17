/*
 *  SPDX-FileCopyrightText: 2020 Saurabh Kumar <saurabhk660@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef STORYBOARD_MODEL
#define STORYBOARD_MODEL

#include "StoryboardItem.h"
#include "CommentModel.h"

#include <QAbstractListModel>
#include <QItemSelection>

#include <kis_keyframe_channel.h>
#include "kis_idle_watcher.h"
#include <kritastoryboarddocker_export.h>
#include <kis_image.h>
#include <kis_signal_compressor.h>

class StoryboardView;
class KisTimeSpan;
class KisStoryboardThumbnailRenderScheduler;

/**
 * @class StoryboardModel
 * @brief The main storyboard model. This class manages a @c StoryboardItemList
 * which is a list of @c StoryboardItem objects. It provides the interface to
 * manipulate and access the data.
 */
class KRITASTORYBOARDDOCKER_EXPORT StoryboardModel : public QAbstractItemModel
{
    Q_OBJECT

public:
    StoryboardModel(QObject *parent);
    ~StoryboardModel() override;

    QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex &index) const override;

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;

    /**
     * @brief Sets the @c scrollValue of the CommentBox object
     * @param index The index of the CommentBox object whose @c scrollValue is changed
     * @param value The new @c scrollValue
     * @return @c True if data was set
     * @sa CommentBox
     */
    bool setCommentScrollData(const QModelIndex & index, const QVariant & value);

    /**
     * @brief Sets the Pixmap data.
     * @param parentIndex The index of item whose thumbnail changed.
     * @param dev Projection of the new pixmap.
     * @return @c True if data was set
     * @sa ThumbnailData
     */
    bool setThumbnailPixmapData(const QModelIndex & parentIndex, const KisPaintDeviceSP & dev);

    /**
     * @brief updates the duration data of item at @c parentIndex to the number
     * of frame to the next @c keyframe in any @c layer.
     * @param parentIndex The index whose duration is to be updated.
     * @return @c True if data was set
     * @note If there are no keyframes after this index's frame duration is set to 0s 0f
     */
    bool updateDurationData(const QModelIndex & parentIndex);

    Qt::ItemFlags flags(const QModelIndex &index) const override;
    
    //for removing and inserting rows
    bool insertRows(int position, int rows, const QModelIndex &index = QModelIndex()) override;
    bool removeRows(int position, int rows, const QModelIndex &index = QModelIndex())override;
    bool moveRows(const QModelIndex &sourceParent, int sourceRow, int count,
                    const QModelIndex &destinationParent, int destinationChild) override;

    //for drag and drop
    QStringList mimeTypes() const override;
    QMimeData *mimeData(const QModelIndexList &indexes) const override;
    bool dropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent) override;
    Qt::DropActions supportedDropActions() const override;
    Qt::DropActions supportedDragActions() const override;

    //these function access the value from the comment model
    /**
     * @brief Used in @c StoryboardDelegate and @c StoryboardView to get size of one storyboard item.
     * @return Number of visible comments.
     * @sa StoryboardDelegate::sizeHint(QStyleOptionViewItem,QModelIndex)
     * @sa StoryboardView::visualRect(QModelIndex)
     */
    int visibleCommentCount() const;

    /**
     * @brief Used in @c StoryboardView to design the layout of storyboard item.
     * @return Number of visible comments upto index.
     * @sa StoryboardView::visualRect(QModelIndex)
     */
    int visibleCommentsUpto(QModelIndex index) const;

    /**
     * @brief Sets the commentModel in StoryboardModel and creates connections
     * to keep the local copy of comments in sync with the commentModel's.
     * @sa slotCommentDataChanged();
     * @sa slotCommentRowInserted(QModelIndex,int,int)
     * @sa slotCommentRowRemoved(QModelIndex,int,int)
     * @sa slotCommentRowMoved();
     */
    void setCommentModel(StoryboardCommentModel *commentModel);

    /**
     * @param row The row of the comment.
     * @return The Comment object at row in comment's list.
     * @note The Comment object contains the name of the comment.
     * Not to be confused with CommentBox.
     * @sa Comment
     */
    StoryboardComment getComment(int row) const;

    void setFreeze(bool);
    bool isFrozen() const;
    void setLocked(bool);
    bool isLocked() const;
    int getFramesPerSecond() const;
    void setView(StoryboardView *view);
    void setImage(KisImageWSP image);

    /**
     * @brief Returns the index of the item corresponding the frame,
     * if there is an item with that frame
     * @param frame The frame whose index is needed.
     * @return The index corresponding to frame, if exists.
     */
    QModelIndex indexFromFrame(int frame) const;

    /**
     * @brief Returns the index of the item with largest frame smaller
     * than arguemnt frame
     * @param frame
     * @return The index with largest frame less than arguemnt frame.
     */
    QModelIndex lastIndexBeforeFrame(int frame) const;

    /**
     * @brief Returns a list of index of items that have frame in between
     * argument range
     * @param range The range of frames
     * @return The list of index corresponding to the range.
     */
    QModelIndexList affectedIndexes(KisTimeSpan range) const;

    /**
     * @brief the next time at which there is a keyframe in any layer after @c keyframeTime
     * @param keyframeTime The time after which keyframe is wanted.
     * @return The time of the next keyframe in any layer.
     */
    int nextKeyframeGlobal(int keyframeTime) const;

    /**
     * @brief Gets the last keyframe that exists within an index's duration.
     * Used to prevent duration from overwriting keyframes that exist internal
     * to an existing scene.
     */
    int lastKeyframeWithin(QModelIndex index);

    /**
     * @brief reorders all keyframes to reflect storyboard docker's arrangement.
     * typically used after drag and drop to keep storyboard timing acurate
     * to timeline timing.
     */
    void reorderKeyframes();

    /**
     * @brief moves all keyframes in all layers after the frame of the parent of @c durationIndex
     * Keyframes are moved to the left or right based on the difference (newDuration-oldDuration)
     * @param oldDuration The old duration in frames assigned to item
     * @param itemIndex The storyboard item index.
     * @return True if keyframes were moved, otherwise False
     */
    bool changeSceneHoldLength(int oldDuration, QModelIndex itemIndex);

    /**
     * @brief inserts item after or before @c index based on @c after parameter
     * @param index The index at which right click was clicked or the plus button belonged to.
     * @param after If True item is added after index, otherwise before
     * @return True if item was inserted, otherwise false
     */
    bool insertItem(QModelIndex index, bool after);

    /**
     * @brief resets @c m_items to @c list
     * @param list The new list of StoryboardItem*
     */
    void resetData(StoryboardItemList list);

    /**
     * @return The list of StoryboardItem* stored in the model.
     */
    StoryboardItemList getData();


    void shiftKeyframes(KisTimeSpan affected, int offset);

    int lastKeyframeGlobal() const;
    void slotUpdateThumbnailsForItems(QModelIndexList indices);
private Q_SLOTS:
    /**
     * @brief called when currentUiTime changes
     * @sa KisImageAnimationInterface::sigUiTimeChanged(int)
     */
    void slotCurrentFrameChanged(int frameId);

    /**
     * @brief called when selection in storyboardView changes. Switches the current time
     * to the frame of first item selected.
     * @sa QItemSelectionModel::selectionChanged(QItemSelection, QItemSelection)
     */
    void slotChangeFrameGlobal(QItemSelection selected, QItemSelection deselected);

    void slotKeyframeAdded(const KisKeyframeChannel *channel, int time);
    void slotKeyframeRemoved(const KisKeyframeChannel *channel, int time);
    void slotNodeRemoved(KisNodeSP node);

    void slotFramerateChanged();

    /**
     * @brief calls regeneration of @c frame in the background i.e. in another thread.
     * @param frame The frame to be regenrated.
     * @param delay Update thumbnail with delay if true
     */
    void slotUpdateThumbnailForFrame(int frame, bool delay = true);

    /**
     * @brief calls regeneration of the currentUiTime() and all frames in @c affectedIndexes(KisTimeSpan)
     */
    void slotUpdateThumbnails();

    /**
     * @brief called @c KisStoryboardThumbnailRenderScheduler when frame render is complete
     * @param frame The frame whose regeneration was requested
     * @param dev The projection of the frame
     */
    void slotFrameRenderCompleted(int frame, KisPaintDeviceSP dev);

    /**
     * @brief called @c KisStoryboardThumbnailRenderScheduler when frame render is cancelled.
     */
    void slotFrameRenderCancelled(int frame);

    void slotCommentDataChanged();
    void slotCommentRowInserted(const QModelIndex, int, int);
    void slotCommentRowRemoved(const QModelIndex, int, int);
    void slotCommentRowMoved(const QModelIndex &sourceParent, int sourceRow, int count,
                            const QModelIndex &destinationParent, int destinationChild);

    /**
     * @brief called when a first level index is inserted. Adds child nodes to the
     * first level indices
     * @param parent The parent of the inseted first level indices.
     * @param first index of the first item inseted.
     * @param last index of the last itme inserted.
     */
    void slotInsertChildRows(const QModelIndex parent, int first, int last);

public Q_SLOTS:
    void slotSetActiveNode(KisNodeSP);

Q_SIGNALS:
    /**
     * @brief This signal is emitted whenever m_items is changed.
     * it is used to keep the StoryboardItemList in KisDocument
     * in sync with m_items
     */
    void sigStoryboardItemListChanged();

private:
    StoryboardItemList m_items;
    QVector<StoryboardComment> m_commentList;
    StoryboardCommentModel *m_commentModel;
    bool m_freezeKeyframePosition;
    bool m_lockBoards;
    bool m_reorderingKeyframes;
    int m_lastScene = 0;
    KisIdleWatcher m_imageIdleWatcher;
    KisImageWSP m_image;
    StoryboardView *m_view;
    KisNodeSP m_activeNode;
    KisStoryboardThumbnailRenderScheduler *m_renderScheduler;
    KisSignalCompressor m_renderSchedulingCompressor;
    KisImageSP cloneImage;
};

#endif
