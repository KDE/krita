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
    void fetchFrame(KisKeyframeSP keyframe, KisPaintDeviceSP targetDevice);

    QRect frameExtents(KisKeyframeSP keyframe);

    QString frameFilename(int frameId) const;

    bool hasScalarValue() const;
    qreal minScalarValue() const;
    qreal maxScalarValue() const;
    qreal scalarValue(const KisKeyframeSP keyframe) const;
    void setScalarValue(KisKeyframeSP keyframe, qreal value, KUndo2Command *parentCommand);

    QDomElement toXML(QDomDocument doc, const QString &layerFilename);
    void loadXML(const QDomElement &channelNode);

    void setOnionSkinsEnabled(bool value);
    bool onionSkinsEnabled() const;

protected:
    KisKeyframeSP createKeyframe(int time, const KisKeyframeSP copySrc, KUndo2Command *parentCommand);
    void destroyKeyframe(KisKeyframeSP key, KUndo2Command *parentCommand);
    void uploadExternalKeyframe(KisKeyframeChannel *srcChannel, int srcTime, KisKeyframeSP dstFrame);

    QRect affectedRect(KisKeyframeSP key);
    void requestUpdate(const KisTimeRange &range, const QRect &rect);

    void saveKeyframe(KisKeyframeSP keyframe, QDomElement keyframeElement, const QString &layerFilename);
    KisKeyframeSP loadKeyframe(const QDomElement &keyframeNode);

private:
    void setFrameFilename(int frameId, const QString &filename);
    QString chooseFrameFilename(int frameId, const QString &layerFilename);

    struct Private;
    QScopedPointer<Private> m_d;
};

#endif
