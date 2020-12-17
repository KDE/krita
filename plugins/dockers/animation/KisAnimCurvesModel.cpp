/*
 *  SPDX-FileCopyrightText: 2016 Jouni Pentikäinen <joupent@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "KisAnimCurvesModel.h"

#include <QAbstractItemModel>

#include "kis_global.h"
#include "kis_image.h"
#include "kis_node.h"
#include "kis_keyframe_channel.h"
#include "kis_scalar_keyframe_channel.h"
#include "kis_post_execution_undo_adapter.h"
#include "KisAnimUtils.h"
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

struct KisAnimCurvesModel::Private
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

    int rowForChannel(const KisKeyframeChannel *channel) {
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

KisAnimCurvesModel::KisAnimCurvesModel(QObject *parent)
    : KisTimeBasedItemModel(parent)
    , m_d(new Private())
{}

KisAnimCurvesModel::~KisAnimCurvesModel()
{
    qDeleteAll(m_d->curves);
}

int KisAnimCurvesModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return m_d->curves.size();
}

QVariant KisAnimCurvesModel::data(const QModelIndex &index, int role) const
{
    KisAnimationCurve *curve = m_d->getCurveAt(index);

    if (curve) {
        KisScalarKeyframeChannel *channel = curve->channel();

        const int time = index.column();
        KisScalarKeyframeSP keyframe = channel->keyframeAt(time).dynamicCast<KisScalarKeyframe>();

        switch (role) {
        case SpecialKeyframeExists:
            return !keyframe.isNull();
        case ScalarValueRole:
            return channel->valueAt(time);
        case LeftTangentRole: {
            if (keyframe.isNull())
                return QVariant();

            const int previousKeyTime = channel->previousKeyframeTime(time);
            KisScalarKeyframeSP previousKeyframe = channel->keyframeAt(previousKeyTime).dynamicCast<KisScalarKeyframe>();

            //Tangent null whenever there's no previous keyframe.
            if (!previousKeyframe)
                return QVariant();

            //Tangent should be null if previous keyframe not set to bezier.
            if (previousKeyframe->interpolationMode() != KisScalarKeyframe::Bezier)
                return QVariant();

            return keyframe->leftTangent();
        }
        case RightTangentRole: {
            if (keyframe.isNull())
                return QVariant();

            if (keyframe->interpolationMode() != KisScalarKeyframe::Bezier)
                return QVariant();

            return keyframe->rightTangent();
        }
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
            int activeKeyframeIndex = channel->activeKeyframeTime(time);
            if (!channel->keyframeAt(activeKeyframeIndex)) return QVariant();
            if (activeKeyframeIndex < time) {
                return activeKeyframeIndex;
            }

            int previousKeyframeIndex = channel->previousKeyframeTime(activeKeyframeIndex);
            if (!channel->keyframeAt(previousKeyframeIndex)) return QVariant();
            return previousKeyframeIndex;
        }
        case NextKeyframeTime:
        {
            int activeKeyframeIndex = channel->activeKeyframeTime(time);
            if (!channel->keyframeAt(activeKeyframeIndex)) {
                int firstKeyIndex = channel->firstKeyframeTime();
                if (firstKeyIndex != -1 && firstKeyIndex > time) {
                    return firstKeyIndex;
                }
                return QVariant();
            }

            int nextKeyframeIndex = channel->nextKeyframeTime(activeKeyframeIndex);
            if (!channel->keyframeAt(nextKeyframeIndex)) return QVariant();
            return nextKeyframeIndex;
        }
        case ChannelLimits:
        {
            if (!channel)
                return QVariant();

            QSharedPointer<ScalarKeyframeLimits> limits = channel->limits();

            if (!limits)
                return QVariant();

            return QVariant::fromValue<ChannelLimitsMetatype>( ChannelLimitsMetatype(limits->lower, limits->upper) );
        }
        default:
            break;
        }
    }

    return KisTimeBasedItemModel::data(index, role);
}

bool KisAnimCurvesModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (!index.isValid())
        return false;

    KisScalarKeyframeChannel *channel = m_d->getCurveAt(index)->channel();
    KUndo2Command *command = m_d->undoCommand;

    switch (role) {
    case ScalarValueRole:
    {
        if (channel->keyframeAt(index.column())) {

            if (!command) command = new KUndo2Command(kundo2_i18n("Adjust keyframe"));
            channel->keyframeAt<KisScalarKeyframe>(index.column())->setValue(value.toReal(), command);
        } else {

            if (!command) command = new KUndo2Command(kundo2_i18n("Insert keyframe"));
            channel->addScalarKeyframe(index.column(), value.toReal(), command);
        }
        dataChanged(index,index);
    }
        break;
    case LeftTangentRole:
    case RightTangentRole:
    {
        KisScalarKeyframeSP keyframe = channel->keyframeAt<KisScalarKeyframe>(index.column());
        if (!keyframe) return false;

        QPointF leftTangent = (role == LeftTangentRole ? value.toPointF() : keyframe->leftTangent());
        QPointF rightTangent = (role == RightTangentRole ? value.toPointF() : keyframe->rightTangent());

        if (!command) command = new KUndo2Command(kundo2_i18n("Adjust tangent"));
        keyframe->setInterpolationTangents(leftTangent, rightTangent, command);
        dataChanged(index, index);
    }
        break;
    case InterpolationModeRole:
    {
        KisScalarKeyframeSP key = channel->keyframeAt<KisScalarKeyframe>(index.column());

        if (!command) command = new KUndo2Command(kundo2_i18n("Set interpolation mode"));
        key->setInterpolationMode((KisScalarKeyframe::InterpolationMode)value.toInt(), command);
        dataChanged(index, index);
    }
        break;
    case TangentsModeRole:
    {
        KisScalarKeyframeSP keyframe = channel->keyframeAt<KisScalarKeyframe>(index.column());
        if (!keyframe) return false;

        KisScalarKeyframe::TangentsMode mode = (KisScalarKeyframe::TangentsMode)value.toInt();

        if (!command) command = new KUndo2Command(kundo2_i18n("Set interpolation mode"));
        keyframe->setTangentsMode( mode, command );
        dataChanged(index,index);
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

QVariant KisAnimCurvesModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    return KisTimeBasedItemModel::headerData(section, orientation, role);
}

bool KisAnimCurvesModel::setHeaderData(int section, Qt::Orientation orientation, const QVariant &value, int role)
{
    return KisTimeBasedItemModel::setHeaderData(section, orientation, value, role);
}

void KisAnimCurvesModel::beginCommand(const KUndo2MagicString &text)
{
    KIS_ASSERT_RECOVER_RETURN(!m_d->undoCommand);
    m_d->undoCommand = new KUndo2Command(text);
}

void KisAnimCurvesModel::endCommand()
{
    KIS_ASSERT_RECOVER_RETURN(m_d->undoCommand);
    image()->postExecutionUndoAdapter()->addCommand(toQShared(m_d->undoCommand));

    m_d->undoCommand = 0;
}


bool KisAnimCurvesModel::adjustKeyframes(const QModelIndexList &indexes, int timeOffset, qreal valueOffset)
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

        using KisAnimUtils::FrameItem;
        using KisAnimUtils::FrameItemList;
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

                    KisScalarKeyframeSP scalarKeyframe = channel->keyframeAt(time).dynamicCast<KisScalarKeyframe>();

                    if (!scalarKeyframe) continue;

                    const qreal currentValue = scalarKeyframe->value();
                    //TODO Undo considerations.
                    scalarKeyframe->setValue(currentValue + valueOffset, cmd.data());
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

KisAnimationCurve *KisAnimCurvesModel::addCurve(KisScalarKeyframeChannel *channel)
{
    beginInsertRows(QModelIndex(), m_d->curves.size(), m_d->curves.size());

    KisAnimationCurve *curve = new KisAnimationCurve(channel, m_d->chooseNextColor());
    m_d->curves.append(curve);

    endInsertRows();

    connect(channel, &KisScalarKeyframeChannel::sigAddedKeyframe,
            this, &KisAnimCurvesModel::slotKeyframeChanged);

    connect(channel, &KisScalarKeyframeChannel::sigAddedKeyframe,
            this, &KisAnimCurvesModel::slotKeyframeAdded);

    connect(channel, &KisScalarKeyframeChannel::sigRemovingKeyframe,
            this, [this](const KisKeyframeChannel* channel, int time) {
        this->slotKeyframeChanged(channel, time);
    });

    connect(channel, SIGNAL(sigKeyframeChanged(const KisKeyframeChannel*,int)),
            this, SLOT(slotKeyframeChanged(const KisKeyframeChannel*,int)));

    return curve;
}

void KisAnimCurvesModel::removeCurve(KisAnimationCurve *curve)
{
    int index = m_d->curves.indexOf(curve);
    if (index < 0) return;

    curve->channel()->disconnect(this);

    beginRemoveRows(QModelIndex(), index, index);

    m_d->curves.removeAt(index);
    delete curve;

    endRemoveRows();
}

void KisAnimCurvesModel::setCurveVisible(KisAnimationCurve *curve, bool visible)
{
    curve->setVisible(visible);

    int row = m_d->rowForCurve(curve);
    emit dataChanged(index(row, 0), index(row, columnCount()));
}

KisNodeSP KisAnimCurvesModel::nodeAt(QModelIndex index) const
{
    KisAnimationCurve *curve = m_d->getCurveAt(index);
    if (curve && curve->channel() && curve->channel()->node()) {
        return KisNodeSP(curve->channel()->node());
    }
    return 0;
}

QMap<QString, KisKeyframeChannel *> KisAnimCurvesModel::channelsAt(QModelIndex index) const
{
    KisKeyframeChannel *channel = m_d->getCurveAt(index)->channel();
    QMap<QString, KisKeyframeChannel*> list;
    list[""] = channel;
    return list;
}

KisKeyframeChannel *KisAnimCurvesModel::channelByID(QModelIndex index, const QString &id) const
{
    return nodeAt(index)->getKeyframeChannel(id);
}

void KisAnimCurvesModel::slotKeyframeChanged(const KisKeyframeChannel *channel, int time)
{
    int row = m_d->rowForChannel(channel);
    QModelIndex changedIndex = index(row, time);
    emit dataChanged(changedIndex, changedIndex);
}

void KisAnimCurvesModel::slotKeyframeAdded(const KisKeyframeChannel *channel, int time)
{
    emit dataAdded(index(m_d->rowForChannel(channel), time));
}


