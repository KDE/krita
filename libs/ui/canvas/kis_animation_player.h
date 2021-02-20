/*
 *  SPDX-FileCopyrightText: 2015 Jouni Pentikäinen <joupent@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KIS_ANIMATION_PLAYER_H
#define KIS_ANIMATION_PLAYER_H

#include <QScopedPointer>
#include <QObject>

#include "kritaui_export.h"


class KisCanvas2;

/**
 * @brief The KisAnimationPlayer class is Krita's high-level
 * animation playback and navigation interface.
 * Its main clients are Krita's Timeline and Curves dockers.
 * It makes heavy use of KisImageAnimationInterface.
 */
class KRITAUI_EXPORT KisAnimationPlayer : public QObject
{
    Q_OBJECT

public:
    KisAnimationPlayer(KisCanvas2 *canvas);
    ~KisAnimationPlayer() override;

    void goToPlaybackOrigin();
    void goToStartFrame();
    void displayFrame(int time);

    bool isPlaying();
    bool isPaused();
    bool isStopped();
    int visibleFrame();

    qreal playbackSpeed();

    void forcedStopOnExit();

    qreal effectiveFps() const;
    qreal realFps() const;
    qreal framesDroppedPortion() const;

Q_SIGNALS:
    void sigFrameChanged();
    void sigPlaybackStarted();
    void sigPlaybackStopped();
    void sigPlaybackStateChanged(bool value);
    void sigPlaybackStatisticsUpdated();
    void sigFullClipRangeChanged();
    void sigPlaybackSpeedChanged(double normalizedSpeed);

public Q_SLOTS:
    void play();
    void pause();
    void playPause();
    void halt();
    void stop();

    void seek(int frameIndex);
    void previousFrame();
    void nextFrame();
    void previousKeyframe();
    void nextKeyframe();
    void previousMatchingKeyframe();
    void nextMatchingKeyframe();

    void slotUpdate();
    void slotCancelPlayback();
    void slotCancelPlaybackSafe();

    void setPlaybackSpeedPercent(int value);
    void setPlaybackSpeedNormalized(double value);
    void slotUpdatePlaybackTimer();

    void slotUpdateDropFramesMode();

private Q_SLOTS:
    void slotSyncScrubbingAudio(int msecTime);
    void slotTryStopScrubbingAudio();
    void slotUpdateAudioChunkLength();
    void slotAudioChannelChanged();
    void slotAudioVolumeChanged();
    void slotOnAudioError(const QString &fileName, const QString &message);

private:
    void connectCancelSignals();
    void disconnectCancelSignals();
    void uploadFrame(int time, bool forceSyncAudio);

    void refreshCanvas();

private:
    struct Private;
    QScopedPointer<Private> m_d;

    enum PlaybackState {
        STOPPED,
        PAUSED,
        PLAYING
    };
};


#endif
