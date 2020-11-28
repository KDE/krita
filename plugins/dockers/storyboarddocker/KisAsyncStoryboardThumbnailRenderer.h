/*
 *  SPDX-FileCopyrightText: 2020 Saurabh Kumar <saurabhk660@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KISASYNCSTORYBOARDTHUMBNAILRENDERER_H
#define KISASYNCSTORYBOARDTHUMBNAILRENDERER_H

#include <KisAsyncAnimationRendererBase.h>

class KisPaintDevice;

/**
 * @class KisAsyncStoryboardThumbnailRenderer
 * @brief requests regeneration of a frame. The regeneration should
 * be requested after switching the @c KisImage to the relevant frame.
 */
class KisAsyncStoryboardThumbnailRenderer : public KisAsyncAnimationRendererBase
{
    Q_OBJECT
public:
    KisAsyncStoryboardThumbnailRenderer(QObject *parent);
    ~KisAsyncStoryboardThumbnailRenderer();

protected:
    void frameCompletedCallback(int frame, const KisRegion &requestedRegion) override;
    void frameCancelledCallback(int frame) override;
    void clearFrameRegenerationState(bool isCancelled) override;

Q_SIGNALS:
    void sigNotifyFrameCompleted(int frameTime, KisPaintDeviceSP frameContents);
    void sigNotifyFrameCompleted(int frameTime);
    void sigNotifyFrameCancelled(int frame);

};

#endif
