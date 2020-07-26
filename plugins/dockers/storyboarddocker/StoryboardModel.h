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

#include "StoryboardItem.h"
#include "CommentModel.h"

#include <QAbstractListModel>
#include <QItemSelection>

#include <kis_keyframe_channel.h>
#include "kis_idle_watcher.h"
#include <kritastoryboarddocker_export.h>
#include <kis_image.h>

/*
    The main storyboard model. 
*/
class StoryboardView;
class KisTimeRange;
class KisStoryboardThumbnailRenderScheduler;
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

class ThumbnailData
{
public:
    ThumbnailData()
    : frameNum("")
    , pixmap(QPixmap())
    {}
    ThumbnailData(const ThumbnailData& other)
    : frameNum(other.frameNum)
    , pixmap(other.pixmap)
    {}
    ~ThumbnailData()
    {}
    QVariant frameNum;
    QVariant pixmap;
};

Q_DECLARE_METATYPE(CommentBox)
Q_DECLARE_METATYPE(ThumbnailData)

class KRITASTORYBOARDDOCKER_EXPORT StoryboardModel : public QAbstractItemModel
{
    Q_OBJECT

public:
    StoryboardModel(QObject *parent = nullptr);

    QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex &index) const override;

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;
    bool setCommentScrollData(const QModelIndex & index, const QVariant & value);
    bool setThumbnailPixmapData(const QModelIndex & parentIndex, const KisPaintDeviceSP & dev);
    bool updateDurationData(const QModelIndex & parentIndex);
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
    void setLocked(bool);
    bool isLocked() const;
    void setView(StoryboardView *view);
    void setImage(KisImageWSP image);

    QModelIndex indexFromFrame(int frame) const;
    QModelIndex lastIndexBeforeFrame(int frame) const;
    QModelIndexList affectedIndexes(KisTimeRange range) const;
    bool isOnlyKeyframe(KisKeyframeSP keyframe, int time) const;
    int nextKeyframeGlobal(int keyframeTime) const;
    bool insertHoldFramesAfter(int newDuration, int oldDuration, QModelIndex durationIndex);
    bool insertItem(QModelIndex index, bool after);

    enum childIndexType{
        FrameNumber,
        ItemName,
        DurationSecond,
        DurationFrame,
        Comments
    };

private Q_SLOTS:
    void slotFrameChanged(int frameId);
    void slotChangeFrameGlobal(QItemSelection selected, QItemSelection deselected);
    void slotKeyframeAdded(KisKeyframeSP keyframe);
    void slotKeyframeRemoved(KisKeyframeSP);
    void slotKeyframeMoved(KisKeyframeSP, int);
    void slotUpdateThumbnailForFrame(int frame);
    void slotUpdateThumbnails();
    void slotFrameRenderCompleted(int frame, KisPaintDeviceSP dev);
    void slotFrameRenderCancelled(int frame);

    void slotCommentDataChanged();
    void slotCommentRowInserted(const QModelIndex, int, int);
    void slotCommentRowRemoved(const QModelIndex, int, int);
    void slotCommentRowMoved(const QModelIndex &sourceParent, int sourceRow, int count,
                            const QModelIndex &destinationParent, int destinationChild);
    void slotInsertChildRows(const QModelIndex parent, int first, int last);

public Q_SLOTS:
    void slotSetActiveNode(KisNodeSP);
private:
    QVector<StoryboardItem*> m_items;
    QVector<Comment> m_commentList;
    CommentModel *m_commentModel;
    bool m_locked;
    int m_lastScene = 0;
    KisIdleWatcher m_imageIdleWatcher;
    KisImageWSP m_image;
    StoryboardView *m_view;
    KisNodeSP m_activeNode;
    KisStoryboardThumbnailRenderScheduler *m_renderScheduler;
    KisImageSP cloneImage;
};

#endif
