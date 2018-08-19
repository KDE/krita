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

#ifndef KIS_KEYFRAME_H
#define KIS_KEYFRAME_H

#include <qglobal.h>
#include <qmetatype.h>
#include <QScopedPointer>

#include "kritaimage_export.h"
#include "kis_types.h"

class KisKeyframeChannel;

class KRITAIMAGE_EXPORT KisKeyframe
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

    KisKeyframe(KisKeyframeChannel *channel, int time);
    virtual ~KisKeyframe();

    /**
     * Create a copy of the keyframe for insertion into given channel.
     * Used when constructing a copy of a keyframe channel.
     */
    virtual KisKeyframeSP cloneFor(KisKeyframeChannel *channel) const = 0;

    int time() const;
    void setTime(int time);

    void setInterpolationMode(InterpolationMode mode);
    InterpolationMode interpolationMode() const;
    void setTangentsMode(InterpolationTangentsMode mode);
    InterpolationTangentsMode tangentsMode() const;
    void setInterpolationTangents(QPointF leftTangent, QPointF rightTangent);
    QPointF leftTangent() const;
    QPointF rightTangent() const;

    int colorLabel() const;
    void setColorLabel(int label);
    virtual bool hasContent() const; // does any content exist in keyframe, or is it empty?

    KisKeyframeChannel *channel() const;

protected:
    KisKeyframe(const KisKeyframe *rhs, KisKeyframeChannel *channel);

private:
    struct Private;
    QScopedPointer<Private> m_d;
};

Q_DECLARE_METATYPE(KisKeyframe*)
Q_DECLARE_METATYPE(KisKeyframeSP)
#endif
