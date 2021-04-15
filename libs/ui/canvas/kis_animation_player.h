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

    /**
     * @brief previousMatchingKeyframe && nextMatchingKeyframe
     * Navigate to the next keyframe that has the same color-label
     * as the current keyframe. Useful to quickly navigate to user-specified
     * 'similar' keyframes. E.g. Contact points in an animation might have
     * a specific color to specify importance and be quickly swapped between.
     */

    void previousMatchingKeyframe();
    void nextMatchingKeyframe();

    /**
     * @brief previousUnfilteredKeyframe && nextUnfilteredKeyframe
     * Navigate to keyframes based on the current onion skin filtration.
     * This lets users easily navigate to the next visible "onion-skinned"
     * keyframe on the active layer.
     */
    void previousUnfilteredKeyframe();
    void nextUnfilteredKeyframe();

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

    void nextKeyframeWithColor(int color);
    void nextKeyframeWithColor(const QSet<int> &validColors);
    void previousKeyframeWithColor(int color);
    void previousKeyframeWithColor(const QSet<int> &validColors);

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
