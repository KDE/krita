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
#ifndef _KIS_SCALAR_KEYFRAME_CHANNEL_H
#define _KIS_SCALAR_KEYFRAME_CHANNEL_H

#include "kis_keyframe_channel.h"


struct ScalarKeyframeLimits {
    qreal lower;
    qreal upper;

    ScalarKeyframeLimits(qreal x, qreal y){
        KIS_ASSERT(x != y);

        // Supports (high, low) or (low, high) assignment.
        lower = x < y ? x : y;
        upper = x > y ? x : y;
    }

    /** Clamp input value to within limits. */
    qreal clamp(qreal value){
        value = value < lower ? lower : value;
        value = value > upper ? upper : value;
        return value;
    }
};

struct KRITAIMAGE_EXPORT KisScalarKeyframe : public KisKeyframe
{
public:
    enum InterpolationMode {
        Constant,
        Linear,
        Bezier
    };

    enum TangentsMode {
        Sharp,
        Smooth
    };

    KisScalarKeyframe(qreal value, QWeakPointer<ScalarKeyframeLimits> limits);
    KisScalarKeyframe(const KisScalarKeyframe &rhs);

    KisKeyframeSP duplicate(KisKeyframeChannel* newChannel = 0) override;

    qreal value() const;
    void setValue(qreal val, KUndo2Command* parentUndoCmd = nullptr);

    void setInterpolationMode(InterpolationMode mode, KUndo2Command* parentUndoCmd = nullptr);
    InterpolationMode interpolationMode() const;
    void setTangentsMode(TangentsMode mode, KUndo2Command* parentUndoCmd = nullptr);
    TangentsMode tangentsMode() const;

    void setInterpolationTangents(QPointF leftTangent, QPointF rightTangent, KUndo2Command* parentUndoCmd = nullptr);

    QPointF leftTangent() const;
    QPointF rightTangent() const;

private:
    qreal m_value;

    InterpolationMode m_interpolationMode;
    TangentsMode m_tangentsMode;
    QPointF m_leftTangent;
    QPointF m_rightTangent;

    QWeakPointer<ScalarKeyframeLimits> m_channelLimits;
};


class KRITAIMAGE_EXPORT KisScalarKeyframeChannel : public KisKeyframeChannel
{
    Q_OBJECT
public:
    KisScalarKeyframeChannel(const KoID& id, KisNodeWSP node);
    KisScalarKeyframeChannel(const KoID& id, KisDefaultBoundsBaseSP bounds);
    KisScalarKeyframeChannel(const KisScalarKeyframeChannel &rhs, KisNodeWSP newParent);
    ~KisScalarKeyframeChannel() override;

    QWeakPointer<ScalarKeyframeLimits> limits() const;
    void setLimits(qreal low, qreal high);
    void removeLimits();

    qreal valueAt(int time) const;
    qreal currentValue() { return valueAt(currentTime()); }

    void setDefaultValue(qreal value);
    void setDefaultInterpolationMode(KisScalarKeyframe::InterpolationMode mode);

    static QPointF interpolate(QPointF point1, QPointF rightTangent, QPointF leftTangent, QPointF point2, qreal t);

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
