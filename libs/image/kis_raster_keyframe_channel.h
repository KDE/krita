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
    KisRasterKeyframeChannel(const KoID& id, const KisPaintDeviceWSP paintDevice, KisDefaultBoundsBaseSP defaultBounds);
    KisRasterKeyframeChannel(const KisRasterKeyframeChannel &rhs, KisNode *newParentNode, const KisPaintDeviceWSP newPaintDevice);
    ~KisRasterKeyframeChannel() override;

public:
    /**
     * Return the ID of the active frame at a given time. The active frame is
     * defined by the keyframe at the given time or the last keyframe before it.
     * @param time
     * @return active frame id
     */
    int frameIdAt(int time) const;

    /**
     * Copy the active frame at given time to target device.
     * @param keyframe keyframe to copy from
     * @param targetDevice device to copy the frame to
     */
    void fetchFrame(KisKeyframeSP keyframe, KisPaintDeviceSP targetDevice);

    /**
     * Copy the content of the sourceDevice into a new keyframe at given time
     * @param time position of new keyframe
     * @param sourceDevice source for content
     * @param parentCommand parent command used for stacking
     */
    void importFrame(int time, KisPaintDeviceSP sourceDevice, KUndo2Command *parentCommand);

    QRect frameExtents(KisKeyframeSP keyframe);

    QString frameFilename(int frameId) const;

    /**
     * When choosing filenames for frames, this will be appended to the node filename
     */
    void setFilenameSuffix(const QString &suffix);

    bool hasScalarValue() const override;

    QDomElement toXML(QDomDocument doc, const QString &layerFilename) override;
    void loadXML(const QDomElement &channelNode) override;

    void setOnionSkinsEnabled(bool value);
    bool onionSkinsEnabled() const;

protected:
    KisKeyframeSP createKeyframe(int time, const KisKeyframeSP copySrc, KUndo2Command *parentCommand) override;
    void destroyKeyframe(KisKeyframeSP key, KUndo2Command *parentCommand) override;
    void uploadExternalKeyframe(KisKeyframeChannel *srcChannel, int srcTime, KisKeyframeSP dstFrame) override;

    QRect affectedRect(KisKeyframeSP key) override;

    void saveKeyframe(KisKeyframeSP keyframe, QDomElement keyframeElement, const QString &layerFilename) override;
    KisKeyframeSP loadKeyframe(const QDomElement &keyframeNode) override;

    friend class KisRasterKeyframe;
    bool keyframeHasContent(const KisKeyframe *keyframe) const;

private:
    void setFrameFilename(int frameId, const QString &filename);
    QString chooseFrameFilename(int frameId, const QString &layerFilename);
    int frameId(KisKeyframeSP keyframe) const;
    int frameId(const KisKeyframe *keyframe) const;

    struct Private;
    QScopedPointer<Private> m_d;
};

#endif
