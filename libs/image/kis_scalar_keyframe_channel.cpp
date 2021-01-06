/*
 *  SPDX-FileCopyrightText: 2015 Jouni Pentikäinen <joupent@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_scalar_keyframe_channel.h"
#include "kis_node.h"
#include "kundo2command.h"
#include "kis_keyframe_commands.h"
#include "kis_time_span.h"

#include <kis_global.h>
#include <kis_dom_utils.h>


KisScalarKeyframe::KisScalarKeyframe(qreal value, QSharedPointer<ScalarKeyframeLimits> limits)
    : KisKeyframe(),
      m_value(value),
      m_interpolationMode(Constant),
      m_tangentsMode(Smooth),
      m_channelLimits(limits)
{
}

KisScalarKeyframe::KisScalarKeyframe(qreal value, InterpolationMode interpMode, TangentsMode tangentMode,
                                     QPointF leftTangent, QPointF rightTangent,
                                     QSharedPointer<ScalarKeyframeLimits> limits)
    : m_value(value),
      m_interpolationMode(interpMode),
      m_tangentsMode(tangentMode),
      m_leftTangent(leftTangent),
      m_rightTangent(rightTangent),
      m_channelLimits(limits)
{
}

KisKeyframeSP KisScalarKeyframe::duplicate(KisKeyframeChannel *newChannel)
{
    if (newChannel) {
        KisScalarKeyframeChannel *scalarChannel = dynamic_cast<KisScalarKeyframeChannel*>(newChannel);
        // When transitioning between channels, set limits to those of the new channel.
        KisScalarKeyframeSP scalarKey = toQShared(new KisScalarKeyframe(m_value, scalarChannel->limits()));
        scalarKey->setInterpolationMode(m_interpolationMode);
        scalarKey->setTangentsMode(m_tangentsMode);
        scalarKey->setInterpolationTangents(leftTangent(), rightTangent());
        return scalarKey;
    } else {
        return toQShared(new KisScalarKeyframe(value(), interpolationMode(), tangentsMode(),
                                               leftTangent(), rightTangent(),
                                               m_channelLimits.toStrongRef()));
    }
}

qreal KisScalarKeyframe::value() const
{
    return m_value;
}

void KisScalarKeyframe::setValue(qreal value, KUndo2Command *parentUndoCmd)
{
    if (parentUndoCmd) {
        KUndo2Command* cmd = new KisScalarKeyframeUpdateCommand(this, value, parentUndoCmd);
        cmd->redo();
    } else {
        m_value = value;

        QSharedPointer<ScalarKeyframeLimits> limits = m_channelLimits.toStrongRef();
        if (limits) {
            m_value = limits->clamp(m_value);
        }

        emit sigChanged(this);
    }
}

void KisScalarKeyframe::setInterpolationMode(InterpolationMode mode, KUndo2Command *parentUndoCmd)
{
    if (parentUndoCmd) {
        KUndo2Command* cmd = new KisScalarKeyframeUpdateCommand(this, mode, parentUndoCmd);
        cmd->redo();
    } else {
        m_interpolationMode = mode;
        emit sigChanged(this);
    }
}

KisScalarKeyframe::InterpolationMode KisScalarKeyframe::interpolationMode() const
{
    return m_interpolationMode;
}

void KisScalarKeyframe::setTangentsMode(TangentsMode mode, KUndo2Command *parentUndoCmd)
{
    if (parentUndoCmd) {
        KUndo2Command* cmd = new KisScalarKeyframeUpdateCommand(this, mode, parentUndoCmd);
        cmd->redo();
    } else {
        m_tangentsMode = mode;
        emit sigChanged(this);
    }
}

KisScalarKeyframe::TangentsMode KisScalarKeyframe::tangentsMode() const
{
    return m_tangentsMode;
}

void KisScalarKeyframe::setInterpolationTangents(QPointF leftTangent, QPointF rightTangent, KUndo2Command *parentUndoCmd)
{
    if (parentUndoCmd) {
        KUndo2Command* cmd = new KisScalarKeyframeUpdateCommand(this, leftTangent, rightTangent, parentUndoCmd);
        cmd->redo();
    } else {
        m_leftTangent = leftTangent;
        m_rightTangent = rightTangent;
        emit sigChanged(this);
    }
}

QPointF KisScalarKeyframe::leftTangent() const
{
    return m_leftTangent;
}

QPointF KisScalarKeyframe::rightTangent() const
{
    return m_rightTangent;
}

void KisScalarKeyframe::setLimits(QSharedPointer<ScalarKeyframeLimits> limits)
{
    m_channelLimits = limits;
}


// ========================================================================================================
// ==================================== KisScalarKeyframeChannel ==========================================
// ========================================================================================================


struct KisScalarKeyframeChannel::Private
{
public:
    Private()
        : defaultValue(0),
          defaultInterpolationMode(KisScalarKeyframe::Constant)
    {}

    Private(const Private &rhs)
        : defaultValue(rhs.defaultValue),
          defaultInterpolationMode(rhs.defaultInterpolationMode)
    {
        if (rhs.limits) {
            limits = toQShared(new ScalarKeyframeLimits(*rhs.limits));
        }
    }

    qreal defaultValue;
    KisScalarKeyframe::InterpolationMode defaultInterpolationMode;

    /** Optional structure that can be added to a channel in order to
     * limit its scalar values within a certain range. */
    QSharedPointer<ScalarKeyframeLimits> limits;
};

KisScalarKeyframeChannel::KisScalarKeyframeChannel(const KoID &id, KisDefaultBoundsBaseSP bounds)
    : KisKeyframeChannel(id, bounds)
    , m_d(new Private)
{
    // When keyframe is changed (value, tangents, etc), we should notify that the channel has been updated.
    connect(this, &KisScalarKeyframeChannel::sigKeyframeChanged, [](const KisKeyframeChannel *channel, int time) {
        const KisScalarKeyframeChannel* chan = dynamic_cast<const KisScalarKeyframeChannel*>(channel);
        chan->sigChannelUpdated(
                    chan->affectedFrames(time),
                    chan->affectedRect(time)
                    );
    });
}

KisScalarKeyframeChannel::KisScalarKeyframeChannel(const KisScalarKeyframeChannel &rhs)
    : KisKeyframeChannel(rhs)
{
    m_d.reset(new Private(*rhs.m_d));

    Q_FOREACH (int time, rhs.constKeys().keys()) {
        KisKeyframeChannel::copyKeyframe(&rhs, time, this, time);
    }

    connect(this, &KisScalarKeyframeChannel::sigKeyframeChanged, [](const KisKeyframeChannel *channel, int time) {
        const KisScalarKeyframeChannel* chan = dynamic_cast<const KisScalarKeyframeChannel*>(channel);
        chan->sigChannelUpdated(
                    chan->affectedFrames(time),
                    chan->affectedRect(time)
                    );
    });
}

KisScalarKeyframeChannel::~KisScalarKeyframeChannel()
{
}

void KisScalarKeyframeChannel::addScalarKeyframe(int time, qreal value, KUndo2Command *parentUndoCmd) {
    addKeyframe(time, parentUndoCmd);
    KisScalarKeyframeSP scalarKey = keyframeAt<KisScalarKeyframe>(time);
    if (scalarKey) {
        scalarKey->setValue(value, parentUndoCmd);
    }
}

QSharedPointer<ScalarKeyframeLimits> KisScalarKeyframeChannel::limits() const
{
    return m_d->limits;
}

void KisScalarKeyframeChannel::setLimits(qreal low, qreal high)
{
    m_d->limits = toQShared(new ScalarKeyframeLimits(low, high));
    QSet<int> keyEntries = allKeyframeTimes();
    foreach (const int &time, keyEntries) {
        KisScalarKeyframeSP scalarKey = keyframeAt<KisScalarKeyframe>(time);
        scalarKey->setLimits(m_d->limits);
        scalarKey->setValue(scalarKey->value());
    }
}

void KisScalarKeyframeChannel::removeLimits()
{
    if (m_d->limits) {
        m_d->limits.reset();
    }
}

qreal KisScalarKeyframeChannel::valueAt(int time) const
{
    const int activeKeyTime = activeKeyframeTime(time);
    KisScalarKeyframeSP activeKey = keyframeAt<KisScalarKeyframe>(activeKeyTime);
    KisScalarKeyframeSP nextKeyframe = keyframeAt<KisScalarKeyframe>(nextKeyframeTime(time));
    qreal result = qQNaN();

    if (activeKey) {
        if (!nextKeyframe) {
            result = activeKey->value();
        } else {
            switch (activeKey->interpolationMode()) {
            case KisScalarKeyframe::Constant: {
                    result = activeKey->value();
                    break;
                }
            case KisScalarKeyframe::Linear: {
                    const int nextKeyTime = nextKeyframeTime(time);
                    const qreal activeKeyValue = activeKey->value();
                    const qreal nextKeyValue = keyframeAt<KisScalarKeyframe>(nextKeyTime)->value();
                    result = activeKeyValue + (nextKeyValue - activeKeyValue) * (time - activeKeyTime) / (nextKeyTime - activeKeyTime);
                    break;
                }
            case KisScalarKeyframe::Bezier: {
                    const int nextKeyTime = nextKeyframeTime(time);
                    const KisScalarKeyframeSP nextKey = keyframeAt<KisScalarKeyframe>(nextKeyTime);
                    QPointF point0 = QPointF(activeKeyTime, activeKey->value());
                    QPointF point1 = QPointF(nextKeyTime, nextKey->value());

                    QPointF tangent0 = activeKey->rightTangent();
                    QPointF tangent1 = nextKey->leftTangent();

                    normalizeTangents(point0, tangent0, tangent1, point1);
                    qreal t = KisScalarKeyframeChannel::findCubicCurveParameter(point0.x(), tangent0.x(), tangent1.x(), point1.x(), time);
                    result = KisScalarKeyframeChannel::interpolate(point0, tangent0, tangent1, point1, t).y();
                    break;
                }
            default: {
                    KIS_ASSERT_RECOVER_BREAK(false);
                    break;
                }
            }
        }
    } else {
        if (nextKeyframe) {
            result = nextKeyframe->value();
        } else {
            return qQNaN();
        }
    }

    // Output value must be also be clamped to account for interpolation.
    if (m_d->limits) {
        return m_d->limits->clamp(result);
    } else {
        return result;
    }
}

bool KisScalarKeyframeChannel::isCurrentTimeAffectedBy(int keyTime) {
    return affectedFrames(activeKeyframeTime(keyTime)).contains(currentTime());
}

void KisScalarKeyframeChannel::setDefaultValue(qreal value)
{
    m_d->defaultValue = value;
}

void KisScalarKeyframeChannel::setDefaultInterpolationMode(KisScalarKeyframe::InterpolationMode mode)
{
    m_d->defaultInterpolationMode = mode;
}

QPointF KisScalarKeyframeChannel::interpolate(QPointF point1, QPointF rightTangent, QPointF leftTangent, QPointF point2, qreal t)
{
    normalizeTangents(point1, rightTangent, leftTangent, point2);

    qreal x = cubicBezier(point1.x(), rightTangent.x(), leftTangent.x(), point2.x(), t);
    qreal y = cubicBezier(point1.y(), rightTangent.y(), leftTangent.y(), point2.y(), t);

    return QPointF(x,y);
}

void KisScalarKeyframeChannel::insertKeyframe(int time, KisKeyframeSP keyframe, KUndo2Command *parentUndoCmd)
{
    KisKeyframeChannel::insertKeyframe(time, keyframe, parentUndoCmd);

    KisScalarKeyframeSP scalarKeyframe = keyframe.dynamicCast<KisScalarKeyframe>();
    if (scalarKeyframe) {
        scalarKeyframe->valueChangedChannelConnection =
                QObject::connect(scalarKeyframe.data(),
                                 &KisScalarKeyframe::sigChanged,
                                 [this, time](const KisScalarKeyframe* key){
                                     Q_UNUSED(key);
                                     emit sigKeyframeChanged(this, time);
                                 });
    }
}

void KisScalarKeyframeChannel::removeKeyframe(int time, KUndo2Command *parentUndoCmd)
{
    KisKeyframeChannel::removeKeyframe(time, parentUndoCmd);

    KisScalarKeyframeSP keyframe = keyframeAt<KisScalarKeyframe>(time);
    if (keyframe) {
        disconnect(keyframe->valueChangedChannelConnection);
    }
}

KisTimeSpan KisScalarKeyframeChannel::affectedFrames(int time) const
{
    KisTimeSpan normalSpan = KisKeyframeChannel::affectedFrames(time);

    const int activeKeyTime = activeKeyframeTime(time);
    const int previousKeyTime = previousKeyframeTime(activeKeyTime);
    const KisScalarKeyframeSP prevScalarKey = keyframeAt<KisScalarKeyframe>(previousKeyTime);

    if(prevScalarKey) {
        // In the case that a previous keyframe is present with a non-constant interpolation mode,
        // the affected frames must include all the frames just after the previous keyframe.
        if (prevScalarKey->interpolationMode() == KisScalarKeyframe::Constant) {
            return normalSpan;
        } else {
            return normalSpan | KisTimeSpan::fromTimeToTime(previousKeyTime + 1, activeKeyTime);
        }
    } else {
        const KisScalarKeyframeSP firstScalarKey = keyframeAt<KisScalarKeyframe>(firstKeyframeTime());
        if (!firstScalarKey) {
            return KisTimeSpan::infinite(0);
        }
        return normalSpan | KisTimeSpan::fromTimeToTime(0, activeKeyTime);
    }
}

KisTimeSpan KisScalarKeyframeChannel::identicalFrames(int time) const
{
    //Failsafe == no keys should mean all frames are identical!
    if (allKeyframeTimes().count() == 0) {
        return KisTimeSpan::infinite(0);
    }

    KisScalarKeyframeSP activeScalarKey = activeKeyframeAt<KisScalarKeyframe>(time);
    if ( activeScalarKey &&
         activeScalarKey->interpolationMode() != KisScalarKeyframe::Constant &&
         activeScalarKey != keyframeAt(lastKeyframeTime()) ) {
        //TODO: Two keyframes should be considered identical if linear with same value..
        //TODO: Also, if bezier with same value AND tangents lie between points.
        //                                          (tangenty == keyframey)
        return KisTimeSpan::fromTimeToTime(time, time);
    }

    const int nextKeyTime = nextKeyframeTime(time);

    //Before the first frame => there's no active frame but a valid next frame.
    if (!activeScalarKey && keyframeAt(nextKeyTime)) {
        return KisTimeSpan::fromTimeToTime(0, nextKeyTime);
    }

    //No next frame, all frames after are identical.
    if (!keyframeAt(nextKeyTime)) {
       return KisTimeSpan::infinite(activeKeyframeTime(time));
    }

    return KisTimeSpan::fromTimeToTime(activeKeyframeTime(time), nextKeyTime - 1);
}

qreal KisScalarKeyframeChannel::findCubicCurveParameter(int time0, qreal delta0, qreal delta1, int time1, int time)
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

qreal KisScalarKeyframeChannel::cubicBezier(qreal p0, qreal delta1, qreal delta2, qreal p3, qreal t) {
    qreal p1 = p0 + delta1;
    qreal p2 = p3 + delta2;

    qreal c = 1-t;
    return c*c*c * p0 + 3*c*c*t * p1 + 3*c*t*t * p2 + t*t*t * p3;
}

void KisScalarKeyframeChannel::normalizeTangents(const QPointF point1, QPointF &rightTangent, QPointF &leftTangent, const QPointF point2)
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

KisKeyframeSP KisScalarKeyframeChannel::createKeyframe()
{
    KisScalarKeyframe *keyframe = new KisScalarKeyframe(m_d->defaultValue, m_d->limits);
    keyframe->setInterpolationMode(m_d->defaultInterpolationMode);
    return toQShared(keyframe);
}

QRect KisScalarKeyframeChannel::affectedRect(int time) const
{
    Q_UNUSED(time);

    if (node()) {
        return node()->exactBounds();
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

    KisScalarKeyframeSP keyframe = createKeyframe().dynamicCast<KisScalarKeyframe>();
    keyframe->setValue(value);

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
