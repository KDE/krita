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
#include "kis_time_span.h"

#include <kis_global.h>
#include <kis_dom_utils.h>

KisScalarKeyframe::KisScalarKeyframe(qreal value)
    : KisKeyframe(),
      m_value(value),
      m_interpolationMode(Constant),
      m_tangentsMode(Smooth)
{}

KisScalarKeyframe::KisScalarKeyframe(const KisScalarKeyframe &rhs)
    : KisKeyframe(rhs),
      m_value(rhs.m_value)
{}

KisKeyframeSP KisScalarKeyframe::duplicate(KisKeyframeChannel *channel) {
    return toQShared(new KisScalarKeyframe(m_value));
}

qreal KisScalarKeyframe::value() const
{
    return m_value;
}

void KisScalarKeyframe::setValue(qreal val)
{
    m_value = val;
}

void KisScalarKeyframe::setInterpolationMode(InterpolationMode mode)
{
    m_interpolationMode = mode;
}

KisScalarKeyframe::InterpolationMode KisScalarKeyframe::interpolationMode() const
{
    return m_interpolationMode;
}

void KisScalarKeyframe::setTangentsMode(InterpolationTangentsMode mode)
{
    m_tangentsMode = mode;
}

KisScalarKeyframe::InterpolationTangentsMode KisScalarKeyframe::tangentsMode() const
{
    return m_tangentsMode;
}

void KisScalarKeyframe::setInterpolationTangents(QPointF leftTangent, QPointF rightTangent)
{
    m_leftTangent = leftTangent;
    m_rightTangent = rightTangent;
}

QPointF KisScalarKeyframe::leftTangent() const
{
    return m_leftTangent;
}

QPointF KisScalarKeyframe::rightTangent() const
{
    return m_rightTangent;
}


// ========================================================================================================

struct KisScalarKeyframeChannel::Private
{
public:
    Private(qreal min, qreal max, KisScalarKeyframe::InterpolationMode defaultInterpolation)
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

    KisScalarKeyframe::InterpolationMode defaultInterpolation;

    struct SetValueCommand;
    struct SetTangentsCommand;
    struct SetInterpolationModeCommand;
};

KisScalarKeyframeChannel::KisScalarKeyframeChannel(const KoID& id, qreal minValue, qreal maxValue, KisNodeWSP parent, KisScalarKeyframe::InterpolationMode defaultInterpolation)
    : KisKeyframeChannel(id, parent),
      m_d(new Private(minValue, maxValue, defaultInterpolation))
{
}

KisScalarKeyframeChannel::KisScalarKeyframeChannel(const KisScalarKeyframeChannel &rhs, KisNodeWSP newParent)
    : KisKeyframeChannel(rhs, newParent),
      m_d(new Private(*rhs.m_d))
{
}

KisScalarKeyframeChannel::~KisScalarKeyframeChannel()
{}

qreal KisScalarKeyframeChannel::minScalarValue() const
{
    return m_d->minValue;
}

qreal KisScalarKeyframeChannel::maxScalarValue() const
{
    return m_d->maxValue;
}

qreal KisScalarKeyframeChannel::scalarValue(const int time) const
{
    KisScalarKeyframeSP key = keyframeAt(time).dynamicCast<KisScalarKeyframe>();
    Q_ASSERT(key != 0);
    return key->value();
}

struct KisScalarKeyframeChannel::Private::SetValueCommand : public KUndo2Command
{
    SetValueCommand(KisScalarKeyframeChannel *channel, KisKeyframeSP keyframe, int time, qreal oldValue, qreal newValue, KUndo2Command *parentCommand)
        : KUndo2Command(parentCommand),
          m_channel(channel),
          m_keyframe(keyframe),
          m_time(time),
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
        key->setValue(value);
        m_channel->notifyKeyframeChanged(m_time);
    }

private:
    KisScalarKeyframeChannel *m_channel;
    KisKeyframeSP m_keyframe;
    int m_time;
    qreal m_oldValue;
    qreal m_newValue;
};

struct KisScalarKeyframeChannel::Private::SetTangentsCommand : public KUndo2Command
{
    SetTangentsCommand(KisScalarKeyframeChannel *channel, KisKeyframeSP keyframe, int time,
                       KisScalarKeyframe::InterpolationTangentsMode oldMode, QPointF oldLeftTangent, QPointF oldRightTangent,
                       KisScalarKeyframe::InterpolationTangentsMode newMode, QPointF newLeftTangent, QPointF newRightTangent,
                       KUndo2Command *parentCommand)
        : KUndo2Command(parentCommand),
          m_channel(channel),
          m_keyframe(keyframe),
          m_time(time),
          m_oldMode(oldMode),
          m_oldLeftTangent(oldLeftTangent),
          m_oldRightTangent(oldRightTangent),
          m_newMode(newMode),
          m_newLeftTangent(newLeftTangent),
          m_newRightTangent(newRightTangent)
    {
    }

    void redo() override {
        KisScalarKeyframeSP scalarKey = m_keyframe.dynamicCast<KisScalarKeyframe>();
        KIS_SAFE_ASSERT_RECOVER_RETURN(scalarKey);
        scalarKey->setTangentsMode(m_newMode);
        scalarKey->setInterpolationTangents(m_newLeftTangent, m_newRightTangent);
        m_channel->notifyKeyframeChanged(m_time);
    }

    void undo() override {
        KisScalarKeyframeSP scalarKey = m_keyframe.dynamicCast<KisScalarKeyframe>();
        KIS_SAFE_ASSERT_RECOVER_RETURN(scalarKey);
        scalarKey->setTangentsMode(m_oldMode);
        scalarKey->setInterpolationTangents(m_oldLeftTangent, m_oldRightTangent);
        m_channel->notifyKeyframeChanged(m_time);
    }

private:
    KisScalarKeyframeChannel *m_channel;
    KisKeyframeSP m_keyframe;
    int m_time;
    KisScalarKeyframe::InterpolationTangentsMode m_oldMode;
    QPointF m_oldLeftTangent;
    QPointF m_oldRightTangent;
    KisScalarKeyframe::InterpolationTangentsMode m_newMode;
    QPointF m_newLeftTangent;
    QPointF m_newRightTangent;
};

struct KisScalarKeyframeChannel::Private::SetInterpolationModeCommand : public KUndo2Command
{
    SetInterpolationModeCommand(KisScalarKeyframeChannel *channel, KisKeyframeSP keyframe, int time, KisScalarKeyframe::InterpolationMode oldMode, KisScalarKeyframe::InterpolationMode newMode, KUndo2Command *parentCommand)
        : KUndo2Command(parentCommand),
          m_channel(channel),
          m_keyframe(keyframe),
          m_time(time),
          m_oldMode(oldMode),
          m_newMode(newMode)
    {
    }

    void redo() override {
        KisScalarKeyframeSP scalarKey = m_keyframe.dynamicCast<KisScalarKeyframe>();
        KIS_SAFE_ASSERT_RECOVER_RETURN(scalarKey);
        scalarKey->setInterpolationMode(m_newMode);
        m_channel->notifyKeyframeChanged(m_time);
    }

    void undo() override {
        KisScalarKeyframeSP scalarKey = m_keyframe.dynamicCast<KisScalarKeyframe>();
        KIS_SAFE_ASSERT_RECOVER_RETURN(scalarKey);
        scalarKey->setInterpolationMode(m_oldMode);
        m_channel->notifyKeyframeChanged(m_time);
    }

private:
    KisScalarKeyframeChannel *m_channel;
    KisKeyframeSP m_keyframe;
    int m_time;
    KisScalarKeyframe::InterpolationMode m_oldMode;
    KisScalarKeyframe::InterpolationMode m_newMode;
};

void KisScalarKeyframeChannel::setScalarValue(const int time, qreal value, KUndo2Command *parentCommand)
{
    KisKeyframeSP keyframe = keyframeAt(time);

    KIS_SAFE_ASSERT_RECOVER_RETURN(keyframe);

    QScopedPointer<KUndo2Command> tempCommand;
    if (!parentCommand) {
        tempCommand.reset(new KUndo2Command());
        parentCommand = tempCommand.data();
    }

    qreal oldValue = scalarValue(time);
    KUndo2Command *cmd = new Private::SetValueCommand(this, keyframe, time, oldValue, value, parentCommand);
    cmd->redo();
}

void KisScalarKeyframeChannel::setInterpolationMode(const int time, KisScalarKeyframe::InterpolationMode mode, KUndo2Command *parentCommand)
{
    KisScalarKeyframeSP keyframe = keyframeAt(time).dynamicCast<KisScalarKeyframe>();

    KIS_SAFE_ASSERT_RECOVER_RETURN(keyframe);

    QScopedPointer<KUndo2Command> tempCommand;
    if (!parentCommand) {
        tempCommand.reset(new KUndo2Command());
        parentCommand = tempCommand.data();
    }

    KisScalarKeyframe::InterpolationMode oldMode = keyframe->interpolationMode();

    KUndo2Command *cmd = new Private::SetInterpolationModeCommand(this, keyframe, time, oldMode, mode, parentCommand);
    cmd->redo();
}

void KisScalarKeyframeChannel::setInterpolationTangents(const int time, QPointF leftTangent, QPointF rightTangent, KUndo2Command *parrentCommand)
{
    KisScalarKeyframeSP keyframe = keyframeAt(time).dynamicCast<KisScalarKeyframe>();
    KIS_SAFE_ASSERT_RECOVER_RETURN(keyframe);

    setInterpolationTangents(time, keyframe->tangentsMode(), leftTangent, rightTangent, parrentCommand);
}

void KisScalarKeyframeChannel::setInterpolationTangents(const int time, KisScalarKeyframe::InterpolationTangentsMode mode, QPointF leftTangent, QPointF rightTangent, KUndo2Command *parentCommand)
{
    KisScalarKeyframeSP keyframe = keyframeAt(time).dynamicCast<KisScalarKeyframe>();
    KIS_SAFE_ASSERT_RECOVER_RETURN(keyframe);

    QScopedPointer<KUndo2Command> tempCommand;
    if (!parentCommand) {
        tempCommand.reset(new KUndo2Command());
        parentCommand = tempCommand.data();
    }

    KisScalarKeyframe::InterpolationTangentsMode oldMode = keyframe->tangentsMode();
    QPointF oldLeftTangent = keyframe->leftTangent();
    QPointF oldRightTangent = keyframe->rightTangent();

    KUndo2Command *cmd = new Private::SetTangentsCommand(this, keyframe, time, oldMode, oldLeftTangent, oldRightTangent, mode, leftTangent, rightTangent, parentCommand);
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
    const int activeKeyTime = activeKeyframeTime(time);
    KisScalarKeyframeSP activeKey = keyframeAt<KisScalarKeyframe>(activeKeyTime);
    if (activeKey.isNull()) return qQNaN();

    qreal result = qQNaN();
    if (time == activeKeyTime || keyframeAt(nextKeyframeTime(time)) == nullptr ) {
        result = activeKey->value();
    } else {
        switch (activeKey->interpolationMode()) {
        case KisScalarKeyframe::Constant:
            result = activeKey->value();
            break;
        case KisScalarKeyframe::Linear:
        {
            const int nextKeyTime = nextKeyframeTime(time);
            const qreal activeKeyValue = activeKey->value();
            const qreal nextKeyValue = keyframeAt<KisScalarKeyframe>(nextKeyTime)->value();
            result = activeKeyValue + (nextKeyValue - activeKeyValue) * (time - activeKeyTime) / (nextKeyTime - activeKeyTime);
            break;
        }
        case KisScalarKeyframe::Bezier:
        {
            const int nextKeyTime = nextKeyframeTime(time);
            const KisScalarKeyframeSP nextKey = keyframeAt<KisScalarKeyframe>(nextKeyTime);
            QPointF point0 = QPointF(activeKeyTime, activeKey->value());
            QPointF point1 = QPointF(nextKeyTime, nextKey->value());

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

KisKeyframeSP KisScalarKeyframeChannel::createKeyframe(qreal value)
{
    KisScalarKeyframe *keyframe = new KisScalarKeyframe(value);
    keyframe->setInterpolationMode(m_d->defaultInterpolation);
    return toQShared(keyframe);
}

KisKeyframeSP KisScalarKeyframeChannel::createKeyframe() // DOUBLE-CHECK!
{
    KisScalarKeyframe *keyframe = new KisScalarKeyframe(0.0f);
    keyframe->setInterpolationMode(m_d->defaultInterpolation);
    return toQShared(keyframe);
}

QRect KisScalarKeyframeChannel::affectedRect(int time)
{
    Q_UNUSED(time);

    if (node()) {
        return node()->extent();
    } else {
        return QRect();
    }
}

void KisScalarKeyframeChannel::saveKeyframe(KisKeyframeSP keyframe, QDomElement keyframeElement, const QString &layerFilename)
{
    Q_UNUSED(layerFilename);
    KisScalarKeyframeSP scalarKey = keyframe.dynamicCast<KisScalarKeyframe>();
    KIS_SAFE_ASSERT_RECOVER_RETURN(scalarKey);
    const qreal value = scalarKey->value();
    keyframeElement.setAttribute("value", KisDomUtils::toString(value));

    QString interpolationMode;
    if (scalarKey->interpolationMode() == KisScalarKeyframe::Constant) interpolationMode = "constant";
    if (scalarKey->interpolationMode() == KisScalarKeyframe::Linear) interpolationMode = "linear";
    if (scalarKey->interpolationMode() == KisScalarKeyframe::Bezier) interpolationMode = "bezier";

    QString tangentsMode;
    if (scalarKey->tangentsMode() == KisScalarKeyframe::Smooth) tangentsMode = "smooth";
    if (scalarKey->tangentsMode() == KisScalarKeyframe::Sharp) tangentsMode = "sharp";

    keyframeElement.setAttribute("interpolation", interpolationMode);
    keyframeElement.setAttribute("tangents", tangentsMode);
    KisDomUtils::saveValue(&keyframeElement, "leftTangent", scalarKey->leftTangent());
    KisDomUtils::saveValue(&keyframeElement, "rightTangent", scalarKey->rightTangent());
}

QPair<int, KisKeyframeSP> KisScalarKeyframeChannel::loadKeyframe(const QDomElement &keyframeNode)
{
    int time = keyframeNode.toElement().attribute("time").toInt();
    workaroundBrokenFrameTimeBug(&time);

    qreal value = KisDomUtils::toDouble(keyframeNode.toElement().attribute("value"));

    KisKeyframeSP keyframe = createKeyframe();
    setScalarValue(keyframe, value);

    KisScalarKeyframeSP scalarKey = keyframe.dynamicCast<KisScalarKeyframe>();

    QString interpolationMode = keyframeNode.toElement().attribute("interpolation");
    if (interpolationMode == "constant") {
        scalarKey->setInterpolationMode(KisScalarKeyframe::Constant);
    } else if (interpolationMode == "linear") {
        scalarKey->setInterpolationMode(KisScalarKeyframe::Linear);
    } else if (interpolationMode == "bezier") {
        scalarKey->setInterpolationMode(KisScalarKeyframe::Bezier);
    }

    QString tangentsMode = keyframeNode.toElement().attribute("tangents");
    if (tangentsMode == "smooth") {
        scalarKey->setTangentsMode(KisScalarKeyframe::Smooth);
    } else if (tangentsMode == "sharp") {
        scalarKey->setTangentsMode(KisScalarKeyframe::Sharp);
    }

    QPointF leftTangent;
    QPointF rightTangent;
    KisDomUtils::loadValue(keyframeNode, "leftTangent", &leftTangent);
    KisDomUtils::loadValue(keyframeNode, "rightTangent", &rightTangent);
    scalarKey->setInterpolationTangents(leftTangent, rightTangent);

    return QPair<int, KisKeyframeSP>(time, keyframe);
}

void KisScalarKeyframeChannel::setScalarValue(KisKeyframeSP keyframe, qreal value)
{
    KisScalarKeyframeSP scalarKey = keyframe.dynamicCast<KisScalarKeyframe>();
    KIS_SAFE_ASSERT_RECOVER_RETURN(scalarKey);
    scalarKey->setValue( value );
}

void KisScalarKeyframeChannel::notifyKeyframeChanged(int time)
{
    QRect rect = affectedRect(time);
    KisTimeSpan range = affectedFrames(time);

    requestUpdate(range, rect);

    emit sigKeyframeChanged(this, keyframeAt(time), time);
}
