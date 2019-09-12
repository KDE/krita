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

#include "kis_scalar_keyframe_channel.h"
#include "kis_node.h"
#include "kundo2command.h"
#include "kis_time_range.h"

#include <kis_global.h>
#include <kis_dom_utils.h>

struct KisScalarKeyframe : public KisKeyframe
{
    KisScalarKeyframe(KisKeyframeChannel *channel, int time, qreal value)
        : KisKeyframe(channel, time)
        , value(value)
    {}

    KisScalarKeyframe(const KisScalarKeyframe *rhs, KisKeyframeChannel *channel)
        : KisKeyframe(rhs, channel)
        , value(rhs->value)
    {}

    qreal value;

    KisKeyframeSP cloneFor(KisKeyframeChannel *channel) const override
    {
        return toQShared(new KisScalarKeyframe(this, channel));
    }
};


KisScalarKeyframeChannel::AddKeyframeCommand::AddKeyframeCommand(KisScalarKeyframeChannel *channel, int time, qreal value, KUndo2Command *parentCommand)
    : KisReplaceKeyframeCommand(channel, time, channel->createKeyframe(time, value, parentCommand), parentCommand)
{}

struct KisScalarKeyframeChannel::Private
{
public:
    Private(qreal min, qreal max, KisKeyframe::InterpolationMode defaultInterpolation)
        : minValue(min), maxValue(max), firstFreeIndex(0), defaultInterpolation(defaultInterpolation)
    {}

    Private(const Private &rhs)
        : minValue(rhs.minValue),
          maxValue(rhs.maxValue),
          firstFreeIndex(rhs.firstFreeIndex),
          defaultInterpolation(rhs.defaultInterpolation)
    {}

    qreal minValue;
    qreal maxValue;
    int firstFreeIndex;

    KisKeyframe::InterpolationMode defaultInterpolation;

    struct SetValueCommand;
    struct SetTangentsCommand;
    struct SetInterpolationModeCommand;
};

KisScalarKeyframeChannel::KisScalarKeyframeChannel(const KoID &id, qreal minValue, qreal maxValue, KisDefaultBoundsBaseSP defaultBounds, KisKeyframe::InterpolationMode defaultInterpolation)
    : KisKeyframeChannel(id, defaultBounds),
      m_d(new Private(minValue, maxValue, defaultInterpolation))
{
}

KisScalarKeyframeChannel::KisScalarKeyframeChannel(const KisScalarKeyframeChannel &rhs, KisNode *newParentNode)
    : KisKeyframeChannel(rhs, newParentNode),
      m_d(new Private(*rhs.m_d))
{
}

KisScalarKeyframeChannel::~KisScalarKeyframeChannel()
{}

bool KisScalarKeyframeChannel::hasScalarValue() const
{
    return true;
}

qreal KisScalarKeyframeChannel::minScalarValue() const
{
    return m_d->minValue;
}

qreal KisScalarKeyframeChannel::maxScalarValue() const
{
    return m_d->maxValue;
}

qreal KisScalarKeyframeChannel::scalarValue(const KisKeyframeSP keyframe) const
{
    KisScalarKeyframe *key = dynamic_cast<KisScalarKeyframe*>(keyframe.data());
    Q_ASSERT(key != 0);
    return key->value;
}

struct KisScalarKeyframeChannel::Private::SetValueCommand : public KUndo2Command
{
    SetValueCommand(KisScalarKeyframeChannel *channel, KisKeyframeSP keyframe, qreal oldValue, qreal newValue, KUndo2Command *parentCommand)
        : KUndo2Command(parentCommand),
          m_channel(channel),
          m_keyframe(keyframe),
          m_oldValue(oldValue),
          m_newValue(newValue)
    {
    }

    void redo() override {
        setValue(m_newValue);
    }

    void undo() override {
        setValue(m_oldValue);
    }

    void setValue(qreal value) {
        KisScalarKeyframe *key = dynamic_cast<KisScalarKeyframe*>(m_keyframe.data());
        Q_ASSERT(key != 0);
        key->value = value;
        m_channel->notifyKeyframeChanged(m_keyframe);
    }

private:
    KisScalarKeyframeChannel *m_channel;
    KisKeyframeSP m_keyframe;
    qreal m_oldValue;
    qreal m_newValue;
};

struct KisScalarKeyframeChannel::Private::SetTangentsCommand : public KUndo2Command
{
    SetTangentsCommand(KisScalarKeyframeChannel *channel, KisKeyframeSP keyframe,
                       KisKeyframe::InterpolationTangentsMode oldMode, QPointF oldLeftTangent, QPointF oldRightTangent,
                       KisKeyframe::InterpolationTangentsMode newMode, QPointF newLeftTangent, QPointF newRightTangent,
                       KUndo2Command *parentCommand)
        : KUndo2Command(parentCommand),
          m_channel(channel),
          m_keyframe(keyframe),
          m_oldMode(oldMode),
          m_oldLeftTangent(oldLeftTangent),
          m_oldRightTangent(oldRightTangent),
          m_newMode(newMode),
          m_newLeftTangent(newLeftTangent),
          m_newRightTangent(newRightTangent)
    {
    }

    void redo() override {
        m_keyframe->setTangentsMode(m_newMode);
        m_keyframe->setInterpolationTangents(m_newLeftTangent, m_newRightTangent);
        m_channel->notifyKeyframeChanged(m_keyframe);
    }

    void undo() override {
        m_keyframe->setTangentsMode(m_oldMode);
        m_keyframe->setInterpolationTangents(m_oldLeftTangent, m_oldRightTangent);
        m_channel->notifyKeyframeChanged(m_keyframe);
    }

private:
    KisScalarKeyframeChannel *m_channel;
    KisKeyframeSP m_keyframe;
    KisKeyframe::InterpolationTangentsMode m_oldMode;
    QPointF m_oldLeftTangent;
    QPointF m_oldRightTangent;
    KisKeyframe::InterpolationTangentsMode m_newMode;
    QPointF m_newLeftTangent;
    QPointF m_newRightTangent;
};

struct KisScalarKeyframeChannel::Private::SetInterpolationModeCommand : public KUndo2Command
{
    SetInterpolationModeCommand(KisScalarKeyframeChannel *channel, KisKeyframeSP keyframe, KisKeyframe::InterpolationMode oldMode, KisKeyframe::InterpolationMode newMode, KUndo2Command *parentCommand)
        : KUndo2Command(parentCommand),
          m_channel(channel),
          m_keyframe(keyframe),
          m_oldMode(oldMode),
          m_newMode(newMode)
    {
    }

    void redo() override {
        m_keyframe->setInterpolationMode(m_newMode);
        m_channel->notifyKeyframeChanged(m_keyframe);
    }

    void undo() override {
        m_keyframe->setInterpolationMode(m_oldMode);
        m_channel->notifyKeyframeChanged(m_keyframe);
    }

private:
    KisScalarKeyframeChannel *m_channel;
    KisKeyframeSP m_keyframe;
    KisKeyframe::InterpolationMode m_oldMode;
    KisKeyframe::InterpolationMode m_newMode;
};

void KisScalarKeyframeChannel::setScalarValue(KisKeyframeSP keyframe, qreal value, KUndo2Command *parentCommand)
{
    QScopedPointer<KUndo2Command> tempCommand;
    if (!parentCommand) {
        tempCommand.reset(new KUndo2Command());
        parentCommand = tempCommand.data();
    }

    qreal oldValue = scalarValue(keyframe);
    KUndo2Command *cmd = new Private::SetValueCommand(this, keyframe, oldValue, value, parentCommand);
    cmd->redo();
}

void KisScalarKeyframeChannel::setInterpolationMode(KisKeyframeSP keyframe, KisKeyframe::InterpolationMode mode, KUndo2Command *parentCommand)
{
    QScopedPointer<KUndo2Command> tempCommand;
    if (!parentCommand) {
        tempCommand.reset(new KUndo2Command());
        parentCommand = tempCommand.data();
    }

    KisKeyframe::InterpolationMode oldMode = keyframe->interpolationMode();

    KUndo2Command *cmd = new Private::SetInterpolationModeCommand(this, keyframe, oldMode, mode, parentCommand);
    cmd->redo();
}

void KisScalarKeyframeChannel::setInterpolationTangents(KisKeyframeSP keyframe, KisKeyframe::InterpolationTangentsMode mode, QPointF leftTangent, QPointF rightTangent, KUndo2Command *parentCommand)
{
    QScopedPointer<KUndo2Command> tempCommand;
    if (!parentCommand) {
        tempCommand.reset(new KUndo2Command());
        parentCommand = tempCommand.data();
    }

    KisKeyframe::InterpolationTangentsMode oldMode = keyframe->tangentsMode();
    QPointF oldLeftTangent = keyframe->leftTangent();
    QPointF oldRightTangent = keyframe->rightTangent();

    KUndo2Command *cmd = new Private::SetTangentsCommand(this, keyframe, oldMode, oldLeftTangent, oldRightTangent, mode, leftTangent, rightTangent, parentCommand);
    cmd->redo();
}

qreal cubicBezier(qreal p0, qreal delta1, qreal delta2, qreal p3, qreal t) {
    qreal p1 = p0 + delta1;
    qreal p2 = p3 + delta2;

    qreal c = 1-t;
    return c*c*c * p0 + 3*c*c*t * p1 + 3*c*t*t * p2 + t*t*t * p3;
}

void normalizeTangents(const QPointF point1, QPointF &rightTangent, QPointF &leftTangent, const QPointF point2)
{
    // To ensure that the curve is monotonic wrt time,
    // check that control points lie between the endpoints.
    // If not, force them into range by scaling down the tangents

    float interval = point2.x() - point1.x();
    if (rightTangent.x() < 0) rightTangent *= 0;
    if (leftTangent.x() > 0) leftTangent *= 0;

    if (rightTangent.x() > interval) {
        rightTangent *= interval / rightTangent.x();
    }
    if (leftTangent.x() < -interval) {
        leftTangent *= interval / -leftTangent.x();
    }
}

QPointF KisScalarKeyframeChannel::interpolate(QPointF point1, QPointF rightTangent, QPointF leftTangent, QPointF point2, qreal t)
{
    normalizeTangents(point1, rightTangent, leftTangent, point2);

    qreal x = cubicBezier(point1.x(), rightTangent.x(), leftTangent.x(), point2.x(), t);
    qreal y = cubicBezier(point1.y(), rightTangent.y(), leftTangent.y(), point2.y(), t);

    return QPointF(x,y);
}

qreal findCubicCurveParameter(int time0, qreal delta0, qreal delta1, int time1, int time)
{
    if (time == time0) return 0.0;
    if (time == time1) return 1.0;

    qreal min_t = 0.0;
    qreal max_t = 1.0;

    while (true) {
        qreal t = (max_t + min_t) / 2;
        qreal time_t = cubicBezier(time0, delta0, delta1, time1, t);

        if (time_t < time - 0.05) {
            min_t = t;
        } else if (time_t > time + 0.05) {
            max_t = t;
        } else {
            // Close enough
            return t;
        }
    }
}

qreal KisScalarKeyframeChannel::interpolatedValue(int time) const
{
    KisKeyframeSP activeKey = activeKeyframeAt(time);
    if (activeKey.isNull()) return qQNaN();

    KisKeyframeSP nextKey = nextKeyframe(activeKey);

    qreal result = qQNaN();
    if (time == activeKey->time() || nextKey.isNull()) {
        result = scalarValue(activeKey);
    } else {
        switch (activeKey->interpolationMode()) {
        case KisKeyframe::Constant:
            result = scalarValue(activeKey);
            break;
        case KisKeyframe::Linear:
        {
            int time0 = activeKey->time();
            int time1 = nextKey->time();
            qreal value0 = scalarValue(activeKey);
            qreal value1 = scalarValue(nextKey);
            result = value0 + (value1 - value0) * (time - time0) / (time1 - time0);
        }
            break;
        case KisKeyframe::Bezier:
        {
            QPointF point0 = QPointF(activeKey->time(), scalarValue(activeKey));
            QPointF point1 = QPointF(nextKey->time(), scalarValue(nextKey));

            QPointF tangent0 = activeKey->rightTangent();
            QPointF tangent1 = nextKey->leftTangent();

            normalizeTangents(point0, tangent0, tangent1, point1);
            qreal t = findCubicCurveParameter(point0.x(), tangent0.x(), tangent1.x(), point1.x(), time);
            result = interpolate(point0, tangent0, tangent1, point1, t).y();
        }
            break;
        default:
            KIS_ASSERT_RECOVER_BREAK(false);
            break;
        }
    }

    if (result > m_d->maxValue) return m_d->maxValue;
    if (result < m_d->minValue) return m_d->minValue;

    return result;
}

qreal KisScalarKeyframeChannel::currentValue() const
{
    return interpolatedValue(currentTime());
}

KisKeyframeSP KisScalarKeyframeChannel::createKeyframe(int time, const KisKeyframeSP copySrc, KUndo2Command *parentCommand)
{
    if (copySrc) {
        KisScalarKeyframe *srcKeyframe = dynamic_cast<KisScalarKeyframe*>(copySrc.data());
        Q_ASSERT(srcKeyframe);
        KisScalarKeyframe *keyframe = new KisScalarKeyframe(srcKeyframe, this);
        keyframe->setTime(time);
        return toQShared(keyframe);
    } else {
        return createKeyframe(time, 0, parentCommand);
    }
}

KisKeyframeSP KisScalarKeyframeChannel::createKeyframe(int time, qreal value, KUndo2Command *parentCommand)
{
    Q_UNUSED(parentCommand);
    KisScalarKeyframe *keyframe = new KisScalarKeyframe(this, time, value);
    keyframe->setInterpolationMode(m_d->defaultInterpolation);
    return toQShared(keyframe);
}

void KisScalarKeyframeChannel::destroyKeyframe(KisKeyframeSP key, KUndo2Command *parentCommand)
{
    Q_UNUSED(parentCommand);
    Q_UNUSED(key);
}

void KisScalarKeyframeChannel::uploadExternalKeyframe(KisKeyframeChannel *srcChannel, int srcTime, KisKeyframeSP dstFrame)
{
    KisScalarKeyframeChannel *srcScalarChannel = dynamic_cast<KisScalarKeyframeChannel*>(srcChannel);
    KIS_ASSERT_RECOVER_RETURN(srcScalarChannel);

    KisKeyframeSP srcFrame = srcScalarChannel->keyframeAt(srcTime);
    KIS_ASSERT_RECOVER_RETURN(srcFrame);

    KisScalarKeyframe *dstKey = dynamic_cast<KisScalarKeyframe*>(dstFrame.data());
    if (dstKey) {
        dstKey->value = srcChannel->scalarValue(srcFrame);
        notifyKeyframeChanged(dstFrame);
    }
}

QRect KisScalarKeyframeChannel::affectedRect(KisKeyframeSP key)
{
    Q_UNUSED(key);

    if (node()) {
        return node()->extent();
    } else {
        return QRect();
    }
}

void KisScalarKeyframeChannel::saveKeyframe(KisKeyframeSP keyframe, QDomElement keyframeElement, const QString &layerFilename)
{
    Q_UNUSED(layerFilename);
    keyframeElement.setAttribute("value", KisDomUtils::toString(scalarValue(keyframe)));

    QString interpolationMode;
    if (keyframe->interpolationMode() == KisKeyframe::Constant) interpolationMode = "constant";
    if (keyframe->interpolationMode() == KisKeyframe::Linear) interpolationMode = "linear";
    if (keyframe->interpolationMode() == KisKeyframe::Bezier) interpolationMode = "bezier";

    QString tangentsMode;
    if (keyframe->tangentsMode() == KisKeyframe::Smooth) tangentsMode = "smooth";
    if (keyframe->tangentsMode() == KisKeyframe::Sharp) tangentsMode = "sharp";

    keyframeElement.setAttribute("interpolation", interpolationMode);
    keyframeElement.setAttribute("tangents", tangentsMode);
    KisDomUtils::saveValue(&keyframeElement, "leftTangent", keyframe->leftTangent());
    KisDomUtils::saveValue(&keyframeElement, "rightTangent", keyframe->rightTangent());
}

KisKeyframeSP KisScalarKeyframeChannel::loadKeyframe(const QDomElement &keyframeNode)
{
    int time = keyframeNode.toElement().attribute("time").toInt();
    workaroundBrokenFrameTimeBug(&time);

    qreal value = KisDomUtils::toDouble(keyframeNode.toElement().attribute("value"));

    KUndo2Command tempParentCommand;
    KisKeyframeSP keyframe = createKeyframe(time, KisKeyframeSP(), &tempParentCommand);
    setScalarValue(keyframe, value);

    QString interpolationMode = keyframeNode.toElement().attribute("interpolation");
    if (interpolationMode == "constant") {
        keyframe->setInterpolationMode(KisKeyframe::Constant);
    } else if (interpolationMode == "linear") {
        keyframe->setInterpolationMode(KisKeyframe::Linear);
    } else if (interpolationMode == "bezier") {
        keyframe->setInterpolationMode(KisKeyframe::Bezier);
    }

    QString tangentsMode = keyframeNode.toElement().attribute("tangents");
    if (tangentsMode == "smooth") {
        keyframe->setTangentsMode(KisKeyframe::Smooth);
    } else if (tangentsMode == "sharp") {
        keyframe->setTangentsMode(KisKeyframe::Sharp);
    }

    QPointF leftTangent;
    QPointF rightTangent;
    KisDomUtils::loadValue(keyframeNode, "leftTangent", &leftTangent);
    KisDomUtils::loadValue(keyframeNode, "rightTangent", &rightTangent);
    keyframe->setInterpolationTangents(leftTangent, rightTangent);

    return keyframe;
}

void KisScalarKeyframeChannel::notifyKeyframeChanged(KisKeyframeSP keyframe)
{
    QRect rect = affectedRect(keyframe);
    KisTimeRange range = affectedFrames(keyframe->time());

    requestUpdate(range, rect);

    emit sigKeyframeChanged(keyframe);
}
