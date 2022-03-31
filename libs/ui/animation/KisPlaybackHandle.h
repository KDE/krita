/*
 *  SPDX-FileCopyrightText: 2022 Eoin O'Neill <eoinoneill1991@gmail.com>
 *  SPDX-FileCopyrightText: 2022 Emmet O'Neill <emmetoneill.pdx@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KISMLTPLAYER_H
#define KISMLTPLAYER_H

#include <QObject>
#include <QSharedPointer>
#include <QFileInfo>
#include <mlt++/MltProducer.h>
#include "KisPlaybackEngine.h"
#include "animation/KisFrameDisplayProxy.h"

class KisFrameDisplayProxy;

/**
 * @brief The KisPlaybackHandle class
 * An abstraction of playback related callbacks. Used to detach MLT and related
 * playback subsystems away from the underlying canvas code.
 *
 * By removing playback related subsystems from the canvas, we enable canvas switching
 * where we can detach and reattach canvases to underlying playback systems on the fly.
 * This should also enable us to play back unfocused canvases using a less accurate timer
 * to take up less system resource later down the line.
 */
class KisPlaybackHandle : public QObject
{
    Q_OBJECT

public:
    KisPlaybackHandle(KisFrameDisplayProxy* displayProxy, QObject* parent = nullptr);
    ~KisPlaybackHandle();

    void seek(int p_frame);
    int visibleFrame();

    void pushAudio();

    void setFrameRate(int p_frameRate);
    int frameRate();

    void setMode(PlaybackMode p_setting);
    PlaybackMode getMode();

    void setPlaybackMedia(QFileInfo toload);
    QFileInfo* playbackMedia();

Q_SIGNALS:
    void sigFrameShow(int p_frame);

    void sigDesiredFrameChanged(int p_frame);
    void sigRequestPushAudio();
    void sigFrameRateChanged(int p_frameRate);
    void sigModeChange(PlaybackMode p_mode) const;
    void sigPlaybackMediaChanged(QFileInfo toLoad);

private:
    struct Private;
    QScopedPointer<Private> m_d;
};

#endif // KISMLTPLAYER_H
