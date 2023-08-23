/*
 *  SPDX-FileCopyrightText: 2015 Jouni Pentikäinen <joupent@gmail.com>
 *  SPDX-FileCopyrightText: 2021 Eoin O'Neill <eoinoneill1991@gmail.com>
 *  SPDX-FileCopyrightText: 2021 Emmet O'Neill <emmetoneill.pdx@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KIS_ANIMATION_PLAYER_H
#define KIS_ANIMATION_PLAYER_H

#include <QScopedPointer>
#include <QObject>
#include <QFileInfo>

#include <kis_time_span.h>
#include <boost/optional.hpp>

#include "kritaui_export.h"


class KisCanvas2;

enum PlaybackState : unsigned int {
    STOPPED,
    PAUSED,
    PLAYING
};

/**
 * @brief The KisCanvasAnimationState class stores all of the canvas-specific animation state.
 *
 * Krita drives animation using a single KisPlaybackEngine instance (found in KisPart).
 * However, there is some data that we want/need to store per-canvas (typically per-document).
 * This might include the frame where playback started, audio volume, or anything else
 * that we may need to persist between canvas changes.
 */
class KRITAUI_EXPORT KisCanvasAnimationState : public QObject
{
    Q_OBJECT

public:
    KisCanvasAnimationState(KisCanvas2 *canvas);
    ~KisCanvasAnimationState() override;

    /**
    * @brief setPlaybackState changes the animation playback state for this canvas.
    * KisPlaybackEngine should respond to changes in state while canvas is active,
    * and use appropriate state when a new canvas is made active.
    */
    void setPlaybackState(PlaybackState state);
    PlaybackState playbackState();

    /**
     * @brief Get the media file info associated with this canvas, if available.
     */
    boost::optional<QFileInfo> mediaInfo();
    qreal currentVolume();

    /**
     * @brief Get the animation frame to return to (for this canvas) when playback is stopped, if available.
     */
    boost::optional<int> playbackOrigin();

    class KisFrameDisplayProxy *displayProxy();

    void setPlaybackSpeed(qreal value);
    qreal playbackSpeed() const;

    /**
     * @brief Sets up the audio tracks for a given animation.
     * (The only reason this is public is because we have to defer this until after canvas construction.)
     */
    void setupAudioTracks();

public Q_SLOTS:
    void showFrame(int frame, bool finalize = false);

Q_SIGNALS:
    void sigPlaybackStateChanged(PlaybackState state);
    void sigPlaybackStatisticsUpdated();
    void sigPlaybackSpeedChanged(qreal value);
    void sigFrameChanged();
    void sigPlaybackMediaChanged();
    void sigAudioLevelChanged(qreal value);

    void sigCancelPlayback();

private:
    KisTimeSpan activePlaybackRange();

    struct Private;
    QScopedPointer<Private> m_d;
};

#endif
