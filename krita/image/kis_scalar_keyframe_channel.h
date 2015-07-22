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

class KRITAIMAGE_EXPORT KisScalarKeyframeChannel : public KisKeyframeChannel
{
    Q_OBJECT

public:
    KisScalarKeyframeChannel(const KoID& id, KisNodeWSP node, qreal minValue, qreal maxValue);
    ~KisScalarKeyframeChannel();

    bool hasScalarValue() const;
    qreal minScalarValue() const;
    qreal maxScalarValue() const;
    qreal scalarValue(const KisKeyframe *keyframe) const;
    void setScalarValue(KisKeyframe *keyframe, qreal value);

protected:
    KisKeyframe *createKeyframe(int time, const KisKeyframe *copySrc);
    bool canDeleteKeyframe(KisKeyframe *key);
    void destroyKeyframe(KisKeyframe *key);

    QRect affectedRect(KisKeyframe *key);

    void saveKeyframe(KisKeyframe *keyframe, QDomElement keyframeElement, const QString &layerFilename);
    KisKeyframe *loadKeyframe(const QDomElement &keyframeNode);

private:
    struct Private;
    QScopedPointer<Private> m_d;
};

#endif
