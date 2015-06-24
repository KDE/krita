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
#ifndef _KIS_RASTER_KEYFRAME_CHANNEL_H
#define _KIS_RASTER_KEYFRAME_CHANNEL_H

#include "kis_keyframe_channel.h"

class KRITAIMAGE_EXPORT KisRasterKeyframeChannel : public KisKeyframeChannel
{
    Q_OBJECT

public:
    KisRasterKeyframeChannel(const KoID& id, const KisNodeWSP node, const KisPaintDeviceWSP paintDevice);
    ~KisRasterKeyframeChannel();

public:
    /**
     * Return the ID of the active frame at a given time. The active frame is
     * defined by the keyframe at the given time or the last keyframe before it.
     * @param time
     * @return active frame id
     */
    int frameIdAt(int time) const;

    /**
     * Copy the active frame at given time and offset to target device.
     * Note: offset is the number of keyframes back or forth counted
     * from the active keyframe at the given time.
     * @param targetDevice device to copy the frame to
     * @param time time to determine base keyframe
     * @param offset number of keyframes to offset from base keyframe
     */
    bool fetchFrame(KisPaintDeviceSP targetDevice, int time, int offset);

    bool hasScalarValue() const;
    qreal minScalarValue() const;
    qreal maxScalarValue() const;
    qreal scalarValue(const KisKeyframe *keyframe) const;
    void setScalarValue(KisKeyframe *keyframe, qreal value);

protected:
    KisKeyframe *createKeyframe(int time, const KisKeyframe *copySrc);
    bool canDeleteKeyframe(KisKeyframe *key);
    void destroyKeyframe(KisKeyframe *key);
    void saveKeyframe(KisKeyframe *keyframe, QDomElement keyframeElement) const;
    KisKeyframe *loadKeyframe(KoXmlNode keyframeNode);

private:
    struct Private;
    Private * const m_d;
};

#endif
