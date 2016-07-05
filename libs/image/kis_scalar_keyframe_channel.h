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

class KRITAIMAGE_EXPORT KisScalarKeyframeChannel : public KisKeyframeChannel
{
    Q_OBJECT

public:
    struct AddKeyframeCommand : public KisReplaceKeyframeCommand
    {
        AddKeyframeCommand(KisScalarKeyframeChannel *channel, int time, qreal value, KUndo2Command *parentCommand);
    };

    KisScalarKeyframeChannel(const KoID& id, qreal minValue, qreal maxValue, KisDefaultBoundsBaseSP defaultBounds, KisKeyframe::InterpolationMode defaultInterpolation=KisKeyframe::Constant);
    ~KisScalarKeyframeChannel();

    bool hasScalarValue() const;
    qreal minScalarValue() const;
    qreal maxScalarValue() const;
    qreal scalarValue(const KisKeyframeSP keyframe) const;
    void setScalarValue(KisKeyframeSP keyframe, qreal value, KUndo2Command *parentCommand = 0);

    qreal interpolatedValue(int time) const;
    qreal currentValue() const;

    static QPointF interpolate(QPointF point1, QPointF rightTangent, QPointF leftTangent, QPointF point2, qreal t);
protected:
    KisKeyframeSP createKeyframe(int time, const KisKeyframeSP copySrc, KUndo2Command *parentCommand);
    KisKeyframeSP createKeyframe(int time, qreal value, KUndo2Command *parentCommand);

    void destroyKeyframe(KisKeyframeSP key, KUndo2Command *parentCommand);
    void uploadExternalKeyframe(KisKeyframeChannel *srcChannel, int srcTime, KisKeyframeSP dstFrame);

    QRect affectedRect(KisKeyframeSP key);

    void saveKeyframe(KisKeyframeSP keyframe, QDomElement keyframeElement, const QString &layerFilename);
    KisKeyframeSP loadKeyframe(const QDomElement &keyframeNode);

private:
    void setScalarValueImpl(KisKeyframeSP keyframe, qreal value);

    struct Private;
    QScopedPointer<Private> m_d;
};

#endif
