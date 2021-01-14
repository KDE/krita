/*
 *  SPDX-FileCopyrightText: 2016 Jouni Pentikäinen <joupent@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef _KIS_ANIMATION_CURVE_CHANNEL_LIST_MODEL_H
#define _KIS_ANIMATION_CURVE_CHANNEL_LIST_MODEL_H

#include <QAbstractItemModel>

#include "kis_types.h"

class KisAnimCurvesModel;
class KisDummiesFacadeBase;
class KisKeyframeChannel;
class KisNodeDummy;

class KisAnimCurvesChannelsModel : public QAbstractItemModel
{
    Q_OBJECT
public:
    KisAnimCurvesChannelsModel(KisAnimCurvesModel *curvesModel, QObject *parent);
    ~KisAnimCurvesChannelsModel() override;

    void setDummiesFacade(KisDummiesFacadeBase *facade);

    QModelIndex index(int row, int column, const QModelIndex &parent) const override;
    QModelIndex parent(const QModelIndex &child) const override;

    int rowCount(const QModelIndex &parent) const override;
    int columnCount(const QModelIndex &parent) const override;

    QVariant data(const QModelIndex &index, int role) const override;
    bool setData(const QModelIndex &index, const QVariant &value, int role) override;

    enum ItemDataRole
    {
        CurveRole = Qt::UserRole,
        CurveColorRole,
        CurveVisibilityRole,
        CurveIsIsolatedRole,
        NodeColorRole
    };

public Q_SLOTS:
    void selectedNodesChanged(const KisNodeList &nodes);
    void reset(const QModelIndex& index);
    void clear();
    void keyframeChannelAddedToNode(KisKeyframeChannel *channel);

private Q_SLOTS:
    void slotNotifyDummyRemoved(KisNodeDummy *dummy);

private:
    struct Private;
    const QScopedPointer<Private> m_d;
};


#endif
