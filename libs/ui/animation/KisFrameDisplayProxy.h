/*
 *  SPDX-FileCopyrightText: 2022 Eoin O'Neill <eoinoneill1991@gmail.com>
 *  SPDX-FileCopyrightText: 2022 Emmet O'Neill <emmetoneill.pdx@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KISFRAMEDISPLAYPROXY_H
#define KISFRAMEDISPLAYPROXY_H

#include <QObject>
#include <QPointer>

#include "kritaui_export.h"

#include "kis_animation_frame_cache.h"

/**
 * @brief The KisFrameDisplayProxy class sits between the KisCanvas (within its KisCanvasAnimationState) and its associated KisImage.
 * Its mainly responsible for choosing whether to reproject the image (always accurate) OR reuse the canvas' KisAnimationFrameCache (usually fast). :)
 */
class KRITAUI_EXPORT KisFrameDisplayProxy : public QObject
{
    Q_OBJECT
public:
    KisFrameDisplayProxy(class KisCanvas2 *canvas, QObject *parent = nullptr);
    ~KisFrameDisplayProxy();

    /**
     * @brief Tell the DisplayProxy to show a new frame.
     * @param forceReproject demands that the image used instead of the canvas' animation cache.
     */
    bool displayFrame(int frame, bool forceReproject);

    /**
     * @brief Gets the active frame, the frame that is intended to be shown.
     * This should always reflect the actual time position of the canvas at any given point.
     * In certain circumstances this value can be different from what is currently being shown.
     * (For example, when showing the activeKeyframe instead.)
     */
    int activeFrame() const;

Q_SIGNALS:
    void sigFrameChange();
    void sigFrameDisplayRefreshed();

    /**
     * @brief sigFrameRefreshSkipped tracks whether asynchronous "slow" refreshes are skipped
     * due to the frame being the same. In the case of waiting for next frame to render, it is necessary
     * to let any binding classes know that the there was no need to refresh the display at all.
     */
    void sigFrameRefreshSkipped();

private:
    /**
     * @brief Get the active keyframe. This is the latest unique frame that is actually visible.
     * It is not always the same as the position of playback, since it only changes when there's new content to be shown. (Consider "held" frames.)
     */
    int activeKeyframe() const;

    bool shouldUploadFrame(KisAnimationFrameCacheSP cache, int from, int to);
    bool needsReprojection(KisAnimationFrameCacheSP cache, int from, int to);

    QScopedPointer<struct Private> m_d;
};

#endif // KISFRAMEDISPLAYPROXY_H
