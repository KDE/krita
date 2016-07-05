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

#ifndef _KIS_ANIMATION_CURVES_MODEL_H
#define _KIS_ANIMATION_CURVES_MODEL_H

#include <QScopedPointer>
#include <QIcon>
#include <QAbstractItemModel>

#include "kis_time_based_item_model.h"
#include "kis_types.h"

class KisAnimationCurvesModel : public KisTimeBasedItemModel
{
    Q_OBJECT
public:
    KisAnimationCurvesModel(QObject *parent);
    ~KisAnimationCurvesModel();

    bool hasConnectionToCanvas() const;

    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    QVariant data(const QModelIndex &index, int role) const;
    bool setData(const QModelIndex &index, const QVariant &value, int role);
    QVariant headerData(int section, Qt::Orientation orientation, int role) const;

    bool removeFrames(const QModelIndexList &indexes);
    bool offsetFrames(QVector<QPoint> srcIndexes, const QPoint &offset, bool copyFrames);

    enum ItemDataRole
    {
        ScalarValueRole = KisTimeBasedItemModel::UserRole + 101,
        InterpolationModeRole,
        LeftTangentRole,
        RightTangentRole,
        CurveColorRole,
        PreviousKeyframeTime,
        NextKeyframeTime
    };

public Q_SLOTS:
    void slotCurrentNodeChanged(KisNodeSP node);

private:
    struct Private;
    const QScopedPointer<Private> m_d;
};

#endif
