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
#include "kis_keyframe_commands.h"


struct KRITAIMAGE_EXPORT KisScalarKeyframe : public KisKeyframe
{
public:
    enum InterpolationMode {
        Constant,
        Linear,
        Bezier
    };

    enum InterpolationTangentsMode {
        Sharp,
        Smooth
    };

    KisScalarKeyframe(qreal value);
    KisScalarKeyframe(const KisScalarKeyframe &rhs);

    KisKeyframeSP duplicate(KisKeyframeChannel* channel = 0) override;

    qreal value() const;
    void setValue(qreal val);

    void setInterpolationMode(InterpolationMode mode);
    InterpolationMode interpolationMode() const;
    void setTangentsMode(InterpolationTangentsMode mode);
    InterpolationTangentsMode tangentsMode() const;

    void setInterpolationTangents(QPointF leftTangent, QPointF rightTangent);

    QPointF leftTangent() const;
    QPointF rightTangent() const;

private:
    qreal m_value;

    InterpolationMode m_interpolationMode;
    InterpolationTangentsMode m_tangentsMode;
    QPointF m_leftTangent;
    QPointF m_rightTangent;
};


class KRITAIMAGE_EXPORT KisScalarKeyframeChannel : public KisKeyframeChannel
{
    Q_OBJECT

public:
    KisScalarKeyframeChannel(const KoID& id, qreal minValue, qreal maxValue, KisNodeWSP parent, KisScalarKeyframe::InterpolationMode defaultInterpolation=KisScalarKeyframe::Constant);
    KisScalarKeyframeChannel(const KisScalarKeyframeChannel &rhs, KisNodeWSP newParent);
    ~KisScalarKeyframeChannel() override;

    qreal minScalarValue() const;
    qreal maxScalarValue() const;
    qreal scalarValue(const int time) const;
    void setScalarValue(const int time, qreal value, KUndo2Command *parentCommand = 0);

    void setInterpolationMode(const int time, KisScalarKeyframe::InterpolationMode mode, KUndo2Command *parentCommand = 0);
    void setInterpolationTangents(const int time, QPointF leftTangent, QPointF rightTangent, KUndo2Command *parrentCommand = 0);
    void setInterpolationTangents(const int time, KisScalarKeyframe::InterpolationTangentsMode, QPointF leftTangent, QPointF rightTangent, KUndo2Command *parentCommand = 0);

    qreal interpolatedValue(int time) const;
    qreal currentValue() const;

    static QPointF interpolate(QPointF point1, QPointF rightTangent, QPointF leftTangent, QPointF point2, qreal t);

Q_SIGNALS:
    void sigKeyframeChanged(KisKeyframeChannel* channel, int time);

private:
    Q_DECL_DEPRECATED KisKeyframeSP createKeyframe(qreal value);

    KisKeyframeSP createKeyframe() override;

    QRect affectedRect(int time) const override;

    void saveKeyframe(KisKeyframeSP keyframe, QDomElement keyframeElement, const QString &layerFilename);
    QPair<int, KisKeyframeSP> loadKeyframe(const QDomElement &keyframeNode) override;

    void setScalarValue(KisKeyframeSP keyframe, qreal value);

    void notifyKeyframeChanged(int time);

    struct Private;
    QScopedPointer<Private> m_d;
};

#endif
