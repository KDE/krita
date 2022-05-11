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

class KRITAUI_EXPORT KisFrameDisplayProxy : public QObject
{
    Q_OBJECT
public:
    KisFrameDisplayProxy(class KisCanvas2 *canvas, QObject *parent = nullptr);
    ~KisFrameDisplayProxy();

    bool displayFrame(int frame);

    /**
     * @brief visibleFrame
     * @return int Frame that is actually visible. This is not the same as the position of playback
     * since the visible frame only changes to actual keyframe content.
     */
    int visibleFrame() const;

    /**
     * @brief frame
     * @return int Frame that is intended to be shown. This should always reflect the actual
     * time position of the canvas at any given point.
     */
    int frame() const;

Q_SIGNALS:
    void sigFrameChange();
    void sigFrameDisplayRefreshed();

protected:
    bool shouldUploadFrame(KisAnimationFrameCacheSP cache, int from, int to);
    bool forceRegeneration(KisAnimationFrameCacheSP cache, int from, int to);

private:
    QScopedPointer<struct Private> m_d;
};

#endif // KISFRAMEDISPLAYPROXY_H
