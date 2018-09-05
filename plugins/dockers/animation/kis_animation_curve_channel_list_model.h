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

#ifndef _KIS_ANIMATION_CURVE_CHANNEL_LIST_MODEL_H
#define _KIS_ANIMATION_CURVE_CHANNEL_LIST_MODEL_H

#include <QAbstractItemModel>

#include "kis_types.h"

class KisAnimationCurvesModel;
class KisDummiesFacadeBase;
class KisKeyframeChannel;
class KisNodeDummy;

class KisAnimationCurveChannelListModel : public QAbstractItemModel
{
    Q_OBJECT
public:
    KisAnimationCurveChannelListModel(KisAnimationCurvesModel *curvesModel, QObject *parent);
    ~KisAnimationCurveChannelListModel() override;

    void setDummiesFacade(KisDummiesFacadeBase *facade);

    QModelIndex index(int row, int column, const QModelIndex &parent) const override;
    QModelIndex parent(const QModelIndex &child) const override;
    int rowCount(const QModelIndex &parent) const override;
    int columnCount(const QModelIndex &parent) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    bool setData(const QModelIndex &index, const QVariant &value, int role) override;

    enum ItemDataRole
    {
        CurveColorRole = Qt::UserRole,
        CurveVisibleRole
    };

public Q_SLOTS:
    void selectedNodesChanged(const KisNodeList &nodes);
    void clear();
    void keyframeChannelAddedToNode(KisKeyframeChannel *channel);

private Q_SLOTS:
    void slotNotifyDummyRemoved(KisNodeDummy *dummy);

private:
    struct Private;
    const QScopedPointer<Private> m_d;
};


#endif
