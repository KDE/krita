/*
 *  SPDX-FileCopyrightText: 2015 Jouni Pentikäinen <joupent@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef _KIS_SCALAR_KEYFRAME_CHANNEL_H
#define _KIS_SCALAR_KEYFRAME_CHANNEL_H

#include "kis_keyframe_channel.h"


/** @brief This structure represents an optional limited range of
 * values that be handled by KisScalarKeyframeChannel.
*/
struct ScalarKeyframeLimits {
    qreal lower;
    qreal upper;

    ScalarKeyframeLimits(qreal x, qreal y){
        KIS_ASSERT(x != y);

        // Supports (high, low) or (low, high) assignment.
        lower = x < y ? x : y;
        upper = x > y ? x : y;
    }

    /** Clamp input value within limits. */
    qreal clamp(qreal value){
        value = value < lower ? lower : value;
        value = value > upper ? upper : value;
        return value;
    }
};


/** @brief The KisScalarKeyframe class is a concrete subclass of KisKeyframe
 * that wraps a scalar value and interpolation parameters.
*/
class KRITAIMAGE_EXPORT KisScalarKeyframe : public KisKeyframe
{
    Q_OBJECT
public:
    /** @brief Controls the type of interpolation between
     * this KisScalarKeyframe and the next. */
    enum InterpolationMode {
        Constant, /**< Constant value until the next keyframe. */
        Linear, /**< Linear interpolation between this keyframe and the next. */
        Bezier /**< Bezier curve between this keyframe and the next,
                 controlled by individual tangent handles on each. */
    };

    /** @brief Controls the behavior of the left and right
     * tangents on a given keyframe for different curve shapes. */
    enum TangentsMode {
        Sharp, /**< Independent control of each tangent for sudden, sharp curve changes. */
        Smooth /**< Tangents are locked inline for smooth transitions across key values. */
    };

    KisScalarKeyframe(qreal value, QSharedPointer<ScalarKeyframeLimits> limits);
    KisScalarKeyframe(qreal value, InterpolationMode interpMode, TangentsMode tangentMode, QPointF leftTangent, QPointF rightTangent, QSharedPointer<ScalarKeyframeLimits> limits);

    KisKeyframeSP duplicate(KisKeyframeChannel* newChannel = 0) override;

    void setValue(qreal val, KUndo2Command* parentUndoCmd = nullptr);
    qreal value() const;

    void setInterpolationMode(InterpolationMode mode, KUndo2Command* parentUndoCmd = nullptr);
    InterpolationMode interpolationMode() const;

    void setTangentsMode(TangentsMode mode, KUndo2Command* parentUndoCmd = nullptr);
    TangentsMode tangentsMode() const;

    void setInterpolationTangents(QPointF leftTangent, QPointF rightTangent, KUndo2Command* parentUndoCmd = nullptr);
    QPointF leftTangent() const;
    QPointF rightTangent() const;

    void setLimits(QSharedPointer<ScalarKeyframeLimits> limits);

    /** @brief For now, scalar keyframes have a callback connection to
     * the channel that owns them in order to signal that their
     * internal state has changed. Created by the channel.
     */
    QMetaObject::Connection valueChangedChannelConnection;

    // Friend class for commands to avoid multi-signal calls..
    friend class KisScalarKeyframeUpdateCommand;

Q_SIGNALS:
    void sigChanged(const KisScalarKeyframe* scalarKey);

private:
    qreal m_value; /**< Scalar value of this keyframe. Optionally clamped to m_channelLimtis. */
    InterpolationMode m_interpolationMode;
    TangentsMode m_tangentsMode;
    QPointF m_leftTangent; /**< Controls part of between this and PREVIOUS keyframe. */
    QPointF m_rightTangent; /**< Controls part of between this and NEXT keyframe. */

    /** Weak pointer back to the owning channel's limits,
     * optionally used when setting the value of a keyframe
     * to conform to the limited range of its current channel,
     * Should change if keyframe is moved to a different channel.
     */
    QWeakPointer<ScalarKeyframeLimits> m_channelLimits;
};


/** @brief The KisScalarKeyframeChannel is a concrete KisKeyframeChannel
 * subclass that stores and manages KisScalarKeyframes.
 *
 * This class maps units of time (in frames) to points along a curve for
 * the animation of various interpolated scalar parameters within Krita.
 * Each scalar channel can be provided with default values and interpolation modes,
 * as well as an optional ScalarKeyframeLimits object that can be used
 * to limit the range of possible values.
 *
 * Generally, each scalar channel will be represented as an individual curve
 * within Krita's KisAnimationCurvesDocker.
*/
class KRITAIMAGE_EXPORT KisScalarKeyframeChannel : public KisKeyframeChannel
{
    Q_OBJECT
public:
    KisScalarKeyframeChannel(const KoID& id, KisNodeWSP node);
    KisScalarKeyframeChannel(const KoID& id, KisDefaultBoundsBaseSP bounds);
    KisScalarKeyframeChannel(const KisScalarKeyframeChannel &rhs, KisNodeWSP newParent);
    ~KisScalarKeyframeChannel() override;

    /** Utility for adding keyframe with non-default value. */
    void addScalarKeyframe(int time, qreal value, KUndo2Command *parentUndoCmd = nullptr);

    QSharedPointer<ScalarKeyframeLimits> limits() const;
    /** Limit channel within scalar value range. */
    void setLimits(qreal low, qreal high);
    /** Remove limits, allowing channel to operate within any range of values. */
    void removeLimits();

    /** @brief Quickly get the interpolated value at the given time. */
    qreal valueAt(int time) const;
    qreal currentValue() { return valueAt(currentTime()); }

    bool isCurrentTimeAffectedBy(int keyTime);

    void setDefaultValue(qreal value);
    void setDefaultInterpolationMode(KisScalarKeyframe::InterpolationMode mode);

    static QPointF interpolate(QPointF point1, QPointF rightTangent, QPointF leftTangent, QPointF point2, qreal t);

    virtual void insertKeyframe(int time, KisKeyframeSP keyframe, KUndo2Command *parentUndoCmd = nullptr) override;
    virtual void removeKeyframe(int time, KUndo2Command *parentUndoCmd = nullptr) override;
    virtual KisTimeSpan affectedFrames(int time) const override;
    virtual KisTimeSpan identicalFrames(int time) const override;

Q_SIGNALS:
    void sigKeyframeChanged(const KisKeyframeChannel *channel, int time);

private:
    static qreal findCubicCurveParameter(int time0, qreal delta0, qreal delta1, int time1, int time);
    static qreal cubicBezier(qreal p0, qreal delta1, qreal delta2, qreal p3, qreal t);
    static void normalizeTangents(const QPointF point1, QPointF &rightTangent, QPointF &leftTangent, const QPointF point2);

    virtual KisKeyframeSP createKeyframe() override;
    virtual QRect affectedRect(int time) const override;
    virtual QPair<int, KisKeyframeSP> loadKeyframe(const QDomElement &keyframeNode) override;
    virtual void saveKeyframe(KisKeyframeSP keyframe, QDomElement keyframeElement, const QString &layerFilename) override;

    struct Private;
    QScopedPointer<Private> m_d;
};

#endif
