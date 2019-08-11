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
#include "kis_animation_utils.h"
#include "kis_processing_applicator.h"
#include "kis_command_utils.h"
#include "KisImageBarrierLockerWithFeedback.h"

struct KisAnimationCurve::Private
{
    Private(KisScalarKeyframeChannel *channel, QColor color)
        : channel(channel)
        , color(color)
        , visible(true)
    {}

    KisScalarKeyframeChannel *channel;
    QColor color;
    bool visible;
};

KisAnimationCurve::KisAnimationCurve(KisScalarKeyframeChannel *channel, QColor color)
    : m_d(new Private(channel, color))
{}

KisScalarKeyframeChannel *KisAnimationCurve::channel() const
{
    return m_d->channel;
}

QColor KisAnimationCurve::color() const
{
    return m_d->color;
}

void KisAnimationCurve::setVisible(bool visible)
{
    m_d->visible = visible;
}

bool KisAnimationCurve::visible() const
{
    return m_d->visible;
}

struct KisAnimationCurvesModel::Private
{
    QList<KisAnimationCurve*> curves;
    int nextColorHue;
    KUndo2Command *undoCommand;

    Private()
        : nextColorHue(0)
        , undoCommand(0)
    {}

    KisAnimationCurve *getCurveAt(const QModelIndex& index) {

        if (!index.isValid()) return 0;

        int row = index.row();

        if (row < 0 || row >= curves.size()) {
            return 0;
        }

        return curves.at(row);
    }

    int rowForCurve(KisAnimationCurve *curve) {
        return curves.indexOf(curve);
    }

    int rowForChannel(KisKeyframeChannel *channel) {
        for (int row = 0; row < curves.count(); row++) {
            if (curves.at(row)->channel() == channel) return row;
        }

        return -1;
    }

    QColor chooseNextColor() {
        if (curves.isEmpty()) nextColorHue = 0;

        QColor color = QColor::fromHsv(nextColorHue, 255, 255);
        nextColorHue += 94; // Value chosen experimentally for providing distinct colors
        nextColorHue = nextColorHue & 0xff;
        return color;
    }
};

KisAnimationCurvesModel::KisAnimationCurvesModel(QObject *parent)
    : KisTimeBasedItemModel(parent)
    , m_d(new Private())
{}

KisAnimationCurvesModel::~KisAnimationCurvesModel()
{
    qDeleteAll(m_d->curves);
}

int KisAnimationCurvesModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return m_d->curves.size();
}

QVariant KisAnimationCurvesModel::data(const QModelIndex &index, int role) const
{
    KisAnimationCurve *curve = m_d->getCurveAt(index);

    if (curve) {
        KisScalarKeyframeChannel *channel = curve->channel();

        int time = index.column();
        KisKeyframeSP keyframe = channel->keyframeAt(time);

        switch (role) {
        case SpecialKeyframeExists:
            return !keyframe.isNull();
        case ScalarValueRole:
            return channel->interpolatedValue(time);
        case LeftTangentRole:
            return (keyframe.isNull()) ? QVariant() : keyframe->leftTangent();
        case RightTangentRole:
            return (keyframe.isNull()) ? QVariant() : keyframe->rightTangent();
        case InterpolationModeRole:
            return (keyframe.isNull()) ? QVariant() : keyframe->interpolationMode();
        case TangentsModeRole:
            return (keyframe.isNull()) ? QVariant() : keyframe->tangentsMode();
        case CurveColorRole:
            return curve->color();
        case CurveVisibleRole:
            return curve->visible();
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
    if (!index.isValid()) return false;
    KisScalarKeyframeChannel *channel = m_d->getCurveAt(index)->channel();
    KUndo2Command *command = m_d->undoCommand;

    switch (role) {
    case ScalarValueRole:
    {
        KisKeyframeSP keyframe = channel->keyframeAt(index.column());
        if (keyframe) {
            if (!command) command = new KUndo2Command(kundo2_i18n("Adjust keyframe"));
            channel->setScalarValue(keyframe, value.toReal(), command);
        } else {
            if (!command) command = new KUndo2Command(kundo2_i18n("Insert keyframe"));
            auto *addKeyframeCommand = new KisScalarKeyframeChannel::AddKeyframeCommand(
                channel, index.column(), value.toReal(), command);
            addKeyframeCommand->redo();
        }
    }
        break;
    case LeftTangentRole:
    case RightTangentRole:
    {
        KisKeyframeSP keyframe = channel->keyframeAt(index.column());
        if (!keyframe) return false;

        QPointF leftTangent = (role == LeftTangentRole ? value.toPointF() : keyframe->leftTangent());
        QPointF rightTangent = (role == RightTangentRole ? value.toPointF() : keyframe->rightTangent());

        if (!command) command = new KUndo2Command(kundo2_i18n("Adjust tangent"));
        channel->setInterpolationTangents(keyframe, keyframe->tangentsMode(), leftTangent, rightTangent, command);
    }
        break;
    case InterpolationModeRole:
    {
        KisKeyframeSP keyframe = channel->keyframeAt(index.column());
        if (!keyframe) return false;

        if (!command) command = new KUndo2Command(kundo2_i18n("Set interpolation mode"));
        channel->setInterpolationMode(keyframe, (KisKeyframe::InterpolationMode)value.toInt(), command);
    }
        break;
    case TangentsModeRole:
    {
        KisKeyframeSP keyframe = channel->keyframeAt(index.column());
        if (!keyframe) return false;

        KisKeyframe::InterpolationTangentsMode mode = (KisKeyframe::InterpolationTangentsMode)value.toInt();
        QPointF leftTangent = keyframe->leftTangent();
        QPointF rightTangent = keyframe->rightTangent();

        if (!command) command = new KUndo2Command(kundo2_i18n("Set interpolation mode"));
        channel->setInterpolationTangents(keyframe, mode, leftTangent, rightTangent, command);
    }
        break;
    default:
        return KisTimeBasedItemModel::setData(index, value, role);
    }

    if (command && !m_d->undoCommand) {
        image()->postExecutionUndoAdapter()->addCommand(toQShared(command));
    }

    return true;
}

QVariant KisAnimationCurvesModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    // TODO
    return KisTimeBasedItemModel::headerData(section, orientation, role);
}

void KisAnimationCurvesModel::beginCommand(const KUndo2MagicString &text)
{
    KIS_ASSERT_RECOVER_RETURN(!m_d->undoCommand);
    m_d->undoCommand = new KUndo2Command(text);
}

void KisAnimationCurvesModel::endCommand()
{
    KIS_ASSERT_RECOVER_RETURN(m_d->undoCommand);
    image()->postExecutionUndoAdapter()->addCommand(toQShared(m_d->undoCommand));

    m_d->undoCommand = 0;
}


bool KisAnimationCurvesModel::adjustKeyframes(const QModelIndexList &indexes, int timeOffset, qreal valueOffset)
{
    QScopedPointer<KUndo2Command> command(
        new KUndo2Command(
            kundo2_i18np("Adjust Keyframe",
                         "Adjust %1 Keyframes",
                         indexes.size())));

    {
        KisImageBarrierLockerWithFeedback locker(image());

        if (timeOffset != 0) {
            bool ok = createOffsetFramesCommand(indexes, QPoint(timeOffset, 0), false, false, command.data());
            if (!ok) return false;
        }

        using KisAnimationUtils::FrameItem;
        using KisAnimationUtils::FrameItemList;
        FrameItemList frameItems;

        Q_FOREACH(QModelIndex index, indexes) {
            KisScalarKeyframeChannel *channel = m_d->getCurveAt(index)->channel();
            KIS_ASSERT_RECOVER_RETURN_VALUE(channel, false);

            frameItems << FrameItem(channel->node(),
                                    channel->id(),
                                    index.column() + timeOffset);
        };

        new KisCommandUtils::LambdaCommand(
            command.data(),
            [frameItems, valueOffset] () -> KUndo2Command* {

                QScopedPointer<KUndo2Command> cmd(new KUndo2Command());

                bool result = false;

                Q_FOREACH (const FrameItem &item, frameItems) {
                    const int time = item.time;
                    KisNodeSP node = item.node;

                    KisKeyframeChannel *channel = node->getKeyframeChannel(item.channel);

                    if (!channel) continue;

                    KisKeyframeSP keyframe = channel->keyframeAt(time);
                    if (!keyframe) continue;

                    const qreal currentValue = channel->scalarValue(keyframe);
                    channel->setScalarValue(keyframe, currentValue + valueOffset, cmd.data());
                    result = true;
                }

                return result ? new KisCommandUtils::SkipFirstRedoWrapper(cmd.take()) : 0;
        });
    }

    KisProcessingApplicator::runSingleCommandStroke(image(), command.take(),
                                                    KisStrokeJobData::BARRIER,
                                                    KisStrokeJobData::EXCLUSIVE);

    return true;
}

KisAnimationCurve *KisAnimationCurvesModel::addCurve(KisScalarKeyframeChannel *channel)
{
    beginInsertRows(QModelIndex(), m_d->curves.size(), m_d->curves.size());

    KisAnimationCurve *curve = new KisAnimationCurve(channel, m_d->chooseNextColor());
    m_d->curves.append(curve);

    endInsertRows();

    connect(channel, &KisScalarKeyframeChannel::sigKeyframeAdded,
            this, &KisAnimationCurvesModel::slotKeyframeChanged);

    connect(channel, &KisScalarKeyframeChannel::sigKeyframeMoved,
            this, &KisAnimationCurvesModel::slotKeyframeChanged);

    connect(channel, &KisScalarKeyframeChannel::sigKeyframeRemoved,
            this, &KisAnimationCurvesModel::slotKeyframeChanged);

    connect(channel, &KisScalarKeyframeChannel::sigKeyframeChanged,
            this, &KisAnimationCurvesModel::slotKeyframeChanged);

    return curve;
}

void KisAnimationCurvesModel::removeCurve(KisAnimationCurve *curve)
{
    int index = m_d->curves.indexOf(curve);
    if (index < 0) return;

    curve->channel()->disconnect(this);

    beginRemoveRows(QModelIndex(), index, index);

    m_d->curves.removeAt(index);
    delete curve;

    endRemoveRows();
}

void KisAnimationCurvesModel::setCurveVisible(KisAnimationCurve *curve, bool visible)
{
    curve->setVisible(visible);

    int row = m_d->rowForCurve(curve);
    emit dataChanged(index(row, 0), index(row, columnCount()));
}

KisNodeSP KisAnimationCurvesModel::nodeAt(QModelIndex index) const
{
    KisAnimationCurve *curve = m_d->getCurveAt(index);
    if (curve && curve->channel() && curve->channel()->node()) {
        return KisNodeSP(curve->channel()->node());
    }
    return 0;
}

QMap<QString, KisKeyframeChannel *> KisAnimationCurvesModel::channelsAt(QModelIndex index) const
{
    KisKeyframeChannel *channel = m_d->getCurveAt(index)->channel();
    QMap<QString, KisKeyframeChannel*> list;
    list[""] = channel;
    return list;
}

void KisAnimationCurvesModel::slotKeyframeChanged(KisKeyframeSP keyframe)
{
    int row = m_d->rowForChannel(keyframe->channel());
    QModelIndex changedIndex = index(row, keyframe->time());
    emit dataChanged(changedIndex, changedIndex);
}
