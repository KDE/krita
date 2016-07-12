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

    qreal value;
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

    qreal minValue;
    qreal maxValue;
    int firstFreeIndex;

    KisKeyframe::InterpolationMode defaultInterpolation;

    struct SetValueCommand;
    struct SetTangentsCommand;
};

KisScalarKeyframeChannel::KisScalarKeyframeChannel(const KoID &id, qreal minValue, qreal maxValue, KisDefaultBoundsBaseSP defaultBounds, KisKeyframe::InterpolationMode defaultInterpolation)
    : KisKeyframeChannel(id, defaultBounds),
      m_d(new Private(minValue, maxValue, defaultInterpolation))
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

    void redo() {
        m_channel->setScalarValueImpl(m_keyframe, m_newValue);
    }

    void undo() {
        m_channel->setScalarValueImpl(m_keyframe, m_oldValue);
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
                       QPointF oldLeftTangent, QPointF oldRightTangent,
                       QPointF newLeftTangent, QPointF newRightTangent,
                       KUndo2Command *parentCommand)
        : KUndo2Command(parentCommand),
          m_channel(channel),
          m_keyframe(keyframe),
          m_oldLeftTangent(oldLeftTangent),
          m_oldRightTangent(oldRightTangent),
          m_newLeftTangent(newLeftTangent),
          m_newRightTangent(newRightTangent)
    {
    }

    void redo() {
        m_channel->setTangentsImpl(m_keyframe, m_newLeftTangent, m_newRightTangent);
    }

    void undo() {
        m_channel->setTangentsImpl(m_keyframe, m_oldLeftTangent, m_oldRightTangent);
    }

private:
    KisScalarKeyframeChannel *m_channel;
    KisKeyframeSP m_keyframe;
    QPointF m_oldLeftTangent;
    QPointF m_oldRightTangent;
    QPointF m_newLeftTangent;
    QPointF m_newRightTangent;
};
void KisScalarKeyframeChannel::setScalarValueImpl(KisKeyframeSP keyframe, qreal value)
{
    KisScalarKeyframe *key = dynamic_cast<KisScalarKeyframe*>(keyframe.data());
    Q_ASSERT(key != 0);

    key->value = value;

    QRect rect = affectedRect(keyframe);
    KisTimeRange range = affectedFrames(keyframe->time());

    requestUpdate(range, rect);
}

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

void KisScalarKeyframeChannel::setInterpolationTangents(KisKeyframeSP keyframe, QPointF leftTangent, QPointF rightTangent, KUndo2Command *parentCommand)
{
    QScopedPointer<KUndo2Command> tempCommand;
    if (!parentCommand) {
        tempCommand.reset(new KUndo2Command());
        parentCommand = tempCommand.data();
    }

    QPointF oldLeftTangent = keyframe->leftTangent();
    QPointF oldRightTangent = keyframe->rightTangent();

    KUndo2Command *cmd = new Private::SetTangentsCommand(this, keyframe, oldLeftTangent, oldRightTangent, leftTangent, rightTangent, parentCommand);
    cmd->redo();
}

void KisScalarKeyframeChannel::setTangentsImpl(KisKeyframeSP keyframe, QPointF leftTangent, QPointF rightTangent)
{
    keyframe->setInterpolationTangents(leftTangent, rightTangent);

    QRect rect = affectedRect(keyframe);
    KisTimeRange range = affectedFrames(keyframe->time());

    requestUpdate(range, rect);}

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
    if (activeKey.isNull()) {
        return qQNaN();
    }

    if (activeKey->interpolationMode() == KisKeyframe::Constant) return scalarValue(activeKey);

    KisKeyframeSP nextKey = nextKeyframe(activeKey);
    if (nextKey.isNull()) return scalarValue(activeKey);

    QPointF point0 = QPointF(activeKey->time(), scalarValue(activeKey));
    QPointF point1 = QPointF(nextKey->time(), scalarValue(nextKey));

    QPointF tangent0 = activeKey->rightTangent();
    QPointF tangent1 = nextKey->leftTangent();

    normalizeTangents(point0, tangent0, tangent1, point1);
    qreal t = findCubicCurveParameter(point0.x(), tangent0.x(), tangent1.x(), point1.x(), time);
    qreal res = interpolate(point0, tangent0, tangent1, point1, t).y();

    if (res > m_d->maxValue) return m_d->maxValue;
    if (res < m_d->minValue) return m_d->minValue;

    return res;
}

qreal KisScalarKeyframeChannel::currentValue() const
{
    return interpolatedValue(currentTime());
}

KisKeyframeSP KisScalarKeyframeChannel::createKeyframe(int time, const KisKeyframeSP copySrc, KUndo2Command *parentCommand)
{
    qreal value = (copySrc.isNull() ? 0 : scalarValue(copySrc));
    return createKeyframe(time, value, parentCommand);
}

KisKeyframeSP KisScalarKeyframeChannel::createKeyframe(int time, qreal value, KUndo2Command *parentCommand)
{
    KisScalarKeyframe *keyframe = new KisScalarKeyframe(this, time, value);

    keyframe->setInterpolationMode(m_d->defaultInterpolation);

    return toQShared(keyframe);
}

void KisScalarKeyframeChannel::destroyKeyframe(KisKeyframeSP key, KUndo2Command *parentCommand)
{
}

void KisScalarKeyframeChannel::uploadExternalKeyframe(KisKeyframeChannel *srcChannel, int srcTime, KisKeyframeSP dstFrame)
{
    KisScalarKeyframeChannel *srcScalarChannel = dynamic_cast<KisScalarKeyframeChannel*>(srcChannel);
    KIS_ASSERT_RECOVER_RETURN(srcScalarChannel);

    KisKeyframeSP srcFrame = srcScalarChannel->keyframeAt(srcTime);
    KIS_ASSERT_RECOVER_RETURN(srcFrame);

    const qreal newValue = srcChannel->scalarValue(srcFrame);

    setScalarValueImpl(dstFrame, newValue);
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

    QString interpolation;
    if (keyframe->interpolationMode() == KisKeyframe::Linear) interpolation = "linear";
    if (keyframe->interpolationMode() == KisKeyframe::Sharp) interpolation = "sharp";
    if (keyframe->interpolationMode() == KisKeyframe::Smooth) interpolation = "smooth";

    if (!interpolation.isEmpty()) {
        keyframeElement.setAttribute("interpolation", interpolation);
        KisDomUtils::saveValue(&keyframeElement, "leftTangent", keyframe->leftTangent());
        KisDomUtils::saveValue(&keyframeElement, "rightTangent", keyframe->rightTangent());
    }
}

KisKeyframeSP KisScalarKeyframeChannel::loadKeyframe(const QDomElement &keyframeNode)
{
    int time = keyframeNode.toElement().attribute("time").toUInt();
    qreal value = KisDomUtils::toDouble(keyframeNode.toElement().attribute("value"));
    QString interpolation = keyframeNode.toElement().attribute("interpolation");

    KUndo2Command tempParentCommand;
    KisKeyframeSP keyframe = createKeyframe(time, KisKeyframeSP(), &tempParentCommand);
    setScalarValue(keyframe, value);

    if (interpolation == "constant") {
        keyframe->setInterpolationMode(KisKeyframe::Constant);
    } else if (interpolation == "linear") {
        keyframe->setInterpolationMode(KisKeyframe::Linear);
    } else if (interpolation == "sharp") {
        keyframe->setInterpolationMode(KisKeyframe::Sharp);
    } else if (interpolation == "smooth") {
        keyframe->setInterpolationMode(KisKeyframe::Smooth);
    }

    QPointF leftTangent;
    QPointF rightTangent;
    KisDomUtils::loadValue(keyframeNode, "leftTangent", &leftTangent);
    KisDomUtils::loadValue(keyframeNode, "rightTangent", &rightTangent);
    keyframe->setInterpolationTangents(leftTangent, rightTangent);

    return keyframe;
}
