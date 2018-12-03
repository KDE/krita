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
#include "kundo2command.h"

class KisScalarKeyframeChannel;

class KisAnimationCurve {
public:
    KisAnimationCurve(KisScalarKeyframeChannel *channel, QColor color);
    KisScalarKeyframeChannel *channel() const;
    QColor color() const;

    void setVisible(bool visible);
    bool visible() const;

private:
    struct Private;
    const QScopedPointer<Private> m_d;
};

class KisAnimationCurvesModel : public KisTimeBasedItemModel
{
    Q_OBJECT
public:
    KisAnimationCurvesModel(QObject *parent);
    ~KisAnimationCurvesModel() override;

    bool hasConnectionToCanvas() const;

    KisAnimationCurve *addCurve(KisScalarKeyframeChannel *channel);
    void removeCurve(KisAnimationCurve *curve);
    void setCurveVisible(KisAnimationCurve *curve, bool visible);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    bool setData(const QModelIndex &index, const QVariant &value, int role) override;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;

    /**
     * Begins a block of commands. The following calls to setData will be grouped to a single undo step.
     * Note: MUST be followed by a call to endCommand().
     */
    void beginCommand(const KUndo2MagicString &text);
    void endCommand();

    bool adjustKeyframes(const QModelIndexList &indexes, int timeOffset, qreal valueOffset);

    enum ItemDataRole
    {
        ScalarValueRole = KisTimeBasedItemModel::UserRole + 101,
        InterpolationModeRole,
        TangentsModeRole,
        LeftTangentRole,
        RightTangentRole,
        CurveColorRole,
        CurveVisibleRole,
        PreviousKeyframeTime,
        NextKeyframeTime
    };

protected:
    KisNodeSP nodeAt(QModelIndex index) const override;
    QMap<QString, KisKeyframeChannel *> channelsAt(QModelIndex index) const override;

private Q_SLOTS:
    void slotKeyframeChanged(KisKeyframeBaseSP keyframe);

private:
    struct Private;
    const QScopedPointer<Private> m_d;
};

#endif
