/*
 *  Copyright (c) 2015 Jouni Pentikäinen <joupent@gmail.com>
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

#ifndef _TIMELINE_MODEL_H_
#define _TIMELINE_MODEL_H_

#include <QAbstractItemModel>

#include "krita_export.h"
#include "kis_image.h"
#include "kis_node_model.h"

/**
 * A model extending the node model to include keyframe channels and
 * keyframes.
 *
 * The nodes and channels are represented in a hierarchy of two column
 * rows. The left column (0) is used for the hierarchy of nodes and
 * keyframe channels with one for row each. The cells in the right
 * column (1) point to KisKeyframeChannels and have a single row of
 * child cells with each keyframe in a column.
 */

class KRITAUI_EXPORT KisTimelineModel : public KisNodeModel
{
    Q_OBJECT

public:

    KisTimelineModel(QObject *parent);
    ~KisTimelineModel();

    /// Extensions to Qt::ItemDataRole. Only applicable to keyframes.
    enum KeyframeItemDataRole
    {
        /// The time associated with the keyframe
        TimeRole = Qt::UserRole + 1,
    };

public: // KisNodeModel
    void setDummiesFacade(KisDummiesFacadeBase *dummiesFacade, KisImageWSP image, KisShapeController *shapeController);

public: // QAbstractItemModel

    QModelIndex index(int row, int column, const QModelIndex &parent) const;
    QModelIndex parent(const QModelIndex &child) const;
    int rowCount(const QModelIndex &parent) const;
    int columnCount(const QModelIndex &parent) const;
    QVariant data(const QModelIndex &index, int role) const;
    bool setData(const QModelIndex &index, const QVariant &value, int role);
    Qt::ItemFlags flags(const QModelIndex &index) const;

private slots:
    // Base class has these slots. Renamed with "2" to avoid hiding them
    void slotEndInsertDummy2(KisNodeDummy *dummy);
    void slotBeginRemoveDummy2(KisNodeDummy *dummy);

    void slotKeyframeAboutToBeAdded(KisKeyframe *keyframe);
    void slotKeyframeAdded(KisKeyframe *keyframe);
    void slotKeyframeAboutToBeRemoved(KisKeyframe *keyframe);
    void slotKeyframeRemoved(KisKeyframe *keyframe);
    void slotKeyframeAboutToBeMoved(KisKeyframe *keyframe, int toTime);
    void slotKeyframeMoved(KisKeyframe *keyframe);

private:
    struct Private;
    Private * const m_d;

    QModelIndex getChannelIndex(KisKeyframeChannel *channel, int column) const;
    void connectAllChannels(KisNodeDummy *nodeDummy, bool needConnect);
    void connectChannels(KisKeyframeSequence *seq, bool needConnect);
    int getInsertionPointByTime(KisKeyframeChannel *channel, int time);
};

#endif
