/*
 *  SPDX-FileCopyrightText: 2015 Jouni Pentikäinen <joupent@gmail.com>
 *  SPDX-FileCopyrightText: 2020 Emmet O 'Neill <emmetoneill.pdx@gmail.com>
 *  SPDX-FileCopyrightText: 2020 Eoin O 'Neill <eoinoneill1991@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef _KIS_RASTER_KEYFRAME_CHANNEL_H
#define _KIS_RASTER_KEYFRAME_CHANNEL_H

#include "kis_keyframe_channel.h"


/** @brief The KisRasterKeyframe class is a concrete subclass of KisKeyframe
 * that wraps a physical raster image frame on a KisPaintDevice.
 *
 * Whenever a "virtual" KisRasterKeyframe is created, a "physical" raster frame
 * is created on the associated KisPaintDevice and its frameID is stored.
 * Likewise, whenever a "virtual" KisRasterKeyframe is destroyed, the "physical" frame
 * associated with its frameID on the KisPaintDevice is automatically freed.
*/
class KRITAIMAGE_EXPORT KisRasterKeyframe : public KisKeyframe
{
    Q_OBJECT
public:
    KisRasterKeyframe(KisPaintDeviceWSP paintDevice);
    KisRasterKeyframe(KisPaintDeviceWSP paintDevice, const int &premadeFrameID, const int &colorLabelId = 0);
    ~KisRasterKeyframe() override;

    /** @brief Get the frameID of the "phsyical" raster frame on the associated KisPaintDevice. */
    int frameID() const;
    bool hasContent();
    QRect contentBounds();

    /** @brief Write this frame's raster content to another paint device.
     * Useful for things like onion skinning where the contents of the frame
     * are drawn to a second, external device.
     */
    void writeFrameToDevice(KisPaintDeviceSP writeTarget);

    KisKeyframeSP duplicate(KisKeyframeChannel *newChannel = 0) override;

private:
    KisRasterKeyframe(const KisRasterKeyframe &rhs);

    /** @brief m_frameID is a handle that references the "physical" frame stored in this keyframe's KisPaintDevice, m_paintDevice.
     * This handle is created by the KisPaintDevice upon construction of the KisRasterKeyframe,
     * and it is passed back to the KisPaintDevice for cleanup upon destruction of the KisRasterKeyframe.
     */
    int m_frameID;
    KisPaintDeviceWSP m_paintDevice;
};


/** @brief The KisRasterKeyframeChannel is a concrete KisKeyframeChannel
 * subclass that stores and manages KisRasterKeyframes.
 *
 * Like a traditional animation dopesheet, this class maps individual units of times (in frames)
 * to "virtual" KisRasterKeyframes, which wrap and manage the "physical" raster images on
 * this channel's associated KisPaintDevice.
 *
 * Often, a raster channel will be represented by an individual track
 * with Krita's KisAnimationTimelineDocker.
*/
class KRITAIMAGE_EXPORT KisRasterKeyframeChannel : public KisKeyframeChannel
{
    Q_OBJECT
public:
    KisRasterKeyframeChannel(const KoID& id, const KisPaintDeviceWSP paintDevice, const KisDefaultBoundsBaseSP bounds);
    KisRasterKeyframeChannel(const KisRasterKeyframeChannel &rhs, const KisPaintDeviceWSP newPaintDevice);
    ~KisRasterKeyframeChannel() override;

    /** Copy the active frame at given time to target device.
     * @param  keyframe  Keyframe to copy from.
     * @param  targetDevice  Device to copy the frame to.
     */
    void writeToDevice(int time, KisPaintDeviceSP targetDevice);

    /** Copy the content of the sourceDevice into a new keyframe at given time.
     * @param  time  Position of new keyframe.
     * @param  sourceDevice  Source for content.
     * @param  parentCommand  Parent undo command used for stacking.
     */
    void importFrame(int time, KisPaintDeviceSP sourceDevice, KUndo2Command *parentCommand);

    /** Get the rectangular area that the content of this frame occupies. */
    QRect frameExtents(KisKeyframeSP keyframe);

    QString frameFilename(int frameId) const;
    /** When choosing filenames for frames, this will be appended to the node filename. */
    void setFilenameSuffix(const QString &suffix);

    QDomElement toXML(QDomDocument doc, const QString &layerFilename) override;
    void loadXML(const QDomElement &channelNode) override;

    void setOnionSkinsEnabled(bool value);
    bool onionSkinsEnabled() const;

    KisPaintDeviceWSP paintDevice();

    virtual void insertKeyframe(int time, KisKeyframeSP keyframe, KUndo2Command *parentUndoCmd = nullptr) override;
    virtual void removeKeyframe(int time, KUndo2Command *parentUndoCmd = nullptr) override;

    void cloneKeyframe(int source, int destination, KUndo2Command *parentUndoCmd = nullptr);
    bool areClones(int timeA, int timeB);
    QSet<int> clonesOf(int time);
    QSet<int> timesForFrameID(int frameID) const;
    static QSet<int> clonesOf(const KisNode *node, int time);

    void makeUnique(int time, KUndo2Command *parentUndoCmd = nullptr);


private:
    QRect affectedRect(int time) const override;

    void saveKeyframe(KisKeyframeSP keyframe, QDomElement keyframeElement, const QString &layerFilename) override;
    QPair<int, KisKeyframeSP> loadKeyframe(const QDomElement &keyframeNode) override;

    KisKeyframeSP createKeyframe() override;

    void setFrameFilename(int frameId, const QString &filename);
    QString chooseFrameFilename(int frameId, const QString &layerFilename);

    struct Private;
    QScopedPointer<Private> m_d;
};

#endif
