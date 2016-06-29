/*
 *  Copyright (c) 2016 Jouni Pentikäinen <joupent@gmail.com>
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

#ifndef _KIS_TIME_BASED_ITEM_MODEL_H
#define _KIS_TIME_BASED_ITEM_MODEL_H

#include <QAbstractTableModel>

#include "kritaanimationdocker_export.h"

#include "kis_types.h"

class KisTimeRange;
class KisAnimationPlayer;

class KRITAANIMATIONDOCKER_EXPORT KisTimeBasedItemModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    KisTimeBasedItemModel(QObject *parent);
    ~KisTimeBasedItemModel();

    void setImage(KisImageWSP image);
    void setFrameCache(KisAnimationFrameCacheSP cache);
    void setAnimationPlayer(KisAnimationPlayer *player);

    void setLastVisibleFrame(int time);

    int columnCount(const QModelIndex &parent = QModelIndex()) const;

    virtual QVariant data(const QModelIndex &index, int role) const;
    virtual bool setData(const QModelIndex &index, const QVariant &value, int role);
    virtual QVariant headerData(int section, Qt::Orientation orientation, int role) const;
    virtual bool setHeaderData(int section, Qt::Orientation orientation, const QVariant &value, int role);

    virtual bool removeFrames(const QModelIndexList &indexes) = 0;
    virtual bool offsetFrames(QVector<QPoint> srcIndexes, const QPoint &offset, bool copyFrames) = 0;

    void setScrubState(bool active);
    void scrubTo(int time, bool preview);

    void setPlaybackRange(const KisTimeRange &range);
    bool isPlaybackActive() const;

    enum ItemDataRole
    {
        ActiveFrameRole = Qt::UserRole + 101,
        FrameExistsRole,
        SpecialKeyframeExists,
        FrameCachedRole,
        FrameEditableRole,
        FramesPerSecondRole,
        UserRole
    };

private Q_SLOTS:
        void slotFramerateChanged();
    void slotCurrentTimeChanged(int time);

    void slotCacheChanged();

    void slotInternalScrubPreviewRequested(int time);

    void slotPlaybackFrameChanged();
    void slotPlaybackStopped();

private:
    struct Private;
    const QScopedPointer<Private> m_d;
};

#endif
