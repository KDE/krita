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

#include "kis_animation_curves_model.h"

#include <QAbstractItemModel>

#include "kis_global.h"
#include "kis_image.h"
#include "kis_node.h"
#include "kis_keyframe_channel.h"
#include "kis_scalar_keyframe_channel.h"
#include "kis_post_execution_undo_adapter.h"

struct KisAnimationCurvesModel::Private
{
    QList<KisKeyframeChannel*> channels;

    KisKeyframeChannel * getChannelAt(const QModelIndex& index) {
        int row = index.row();

        if (row < 0 || row >= channels.size()) {
            return 0;
        }

        return channels.at(row);
    }
};

KisAnimationCurvesModel::KisAnimationCurvesModel(QObject *parent)
    : KisTimeBasedItemModel(parent)
    , m_d(new Private())
{}

KisAnimationCurvesModel::~KisAnimationCurvesModel()
{}

int KisAnimationCurvesModel::rowCount(const QModelIndex &parent) const
{
    return m_d->channels.size();
}

QVariant KisAnimationCurvesModel::data(const QModelIndex &index, int role) const
{
    KisKeyframeChannel *channel = m_d->getChannelAt(index);
    KisScalarKeyframeChannel *scalarChannel = dynamic_cast<KisScalarKeyframeChannel*>(channel);

    if (scalarChannel) {
        int time = index.column();
        KisKeyframeSP keyframe = channel->keyframeAt(time);

        switch (role) {
        case SpecialKeyframeExists:
            return !keyframe.isNull();
        case ScalarValueRole:
            return (!scalarChannel) ? QVariant() : scalarChannel->interpolatedValue(time);
        case LeftTangentRole:
            return (keyframe.isNull()) ? QVariant() : keyframe->leftTangent();
        case RightTangentRole:
            return (keyframe.isNull()) ? QVariant() : keyframe->rightTangent();
        case InterpolationModeRole:
            return (keyframe.isNull()) ? QVariant() : keyframe->interpolationMode();
        case CurveColorRole:
            return QColor(Qt::red);
        case PreviousKeyframeTime:
        {
            KisKeyframeSP active = channel->activeKeyframeAt(time);
            if (active.isNull()) return QVariant();
            if (active->time() < time) {
                return active->time();
            }
            KisKeyframeSP previous = channel->previousKeyframe(active);
            if (previous.isNull()) return QVariant();
            return previous->time();
        }
        case NextKeyframeTime:
        {
            KisKeyframeSP active = channel->activeKeyframeAt(time);
            if (active.isNull()) {
                KisKeyframeSP first = channel->firstKeyframe();
                if (!first.isNull() && first->time() > time) {
                    return first->time();
                }
                return QVariant();
            }
            KisKeyframeSP next = channel->nextKeyframe(active);
            if (next.isNull()) return QVariant();
            return next->time();
        }
        default:
            break;
        }
    }

    return KisTimeBasedItemModel::data(index, role);
}

bool KisAnimationCurvesModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    KisKeyframeChannel *channel = m_d->getChannelAt(index);
    KisScalarKeyframeChannel *scalarChannel = dynamic_cast<KisScalarKeyframeChannel*>(channel);

    switch (role) {
    case LeftTangentRole:
    case RightTangentRole:
    {
        KisKeyframeSP keyframe = channel->keyframeAt(index.column());
        if (keyframe) {
            QPointF leftTangent = (role == LeftTangentRole ? value.toPointF() : keyframe->leftTangent());
            QPointF rightTangent = (role == RightTangentRole ? value.toPointF() : keyframe->rightTangent());

            KUndo2Command *command = new KUndo2Command(kundo2_i18n("Adjust tangent"));
            scalarChannel->setInterpolationTangents(keyframe, leftTangent, rightTangent, command);
            image()->postExecutionUndoAdapter()->addCommand(toQShared(command));

            return true;
        }
    }
        break;
    default:
        break;
    }

    return KisTimeBasedItemModel::setData(index, value, role);
}

QVariant KisAnimationCurvesModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    // TODO
    return KisTimeBasedItemModel::headerData(section, orientation, role);
}

void KisAnimationCurvesModel::slotCurrentNodeChanged(KisNodeSP node)
{
    beginResetModel();
    m_d->channels = node->keyframeChannels();
    endResetModel();
}

bool KisAnimationCurvesModel::removeFrames(const QModelIndexList &indexes)
{
    // TODO
    return false;
}

bool KisAnimationCurvesModel::adjustKeyframes(const QModelIndexList &indexes, int timeOffset, qreal valueOffset)
{
    KUndo2Command *command = new KUndo2Command(
        kundo2_i18np("Adjust Keyframe",
                     "Adjust %1 Keyframes",
                     indexes.size()));

    bool ok = offsetFrames(indexes, QPoint(timeOffset, 0), false, command);
    if (!ok) return false;

    Q_FOREACH(QModelIndex oldIndex, indexes) {
        KisKeyframeChannel *channel = m_d->getChannelAt(oldIndex);
        KIS_ASSERT_RECOVER_RETURN_VALUE(channel, false);

        KisKeyframeSP keyframe = channel->keyframeAt(oldIndex.column() + timeOffset);
        KIS_ASSERT_RECOVER_RETURN_VALUE(!keyframe.isNull(), false);

        qreal currentValue = channel->scalarValue(keyframe);
        channel->setScalarValue(keyframe, currentValue + valueOffset, command);
    };

    image()->postExecutionUndoAdapter()->addCommand(toQShared(command));

    return true;
}

KisNodeSP KisAnimationCurvesModel::nodeAt(QModelIndex index) const
{
    return KisNodeSP(m_d->getChannelAt(index)->node());
}

QList<KisKeyframeChannel *> KisAnimationCurvesModel::channelsAt(QModelIndex index) const
{
    QList<KisKeyframeChannel*> list({m_d->getChannelAt(index)});
    return list;
}
