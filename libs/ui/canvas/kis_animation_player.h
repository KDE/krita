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

#include <kis_time_span.h>

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

    enum PlaybackState {
        STOPPED,
        PAUSED,
        PLAYING
    };

    enum SeekFlags {
        NONE = 0,
        PUSH_AUDIO = 1,
        FORCE_RECACHE = 1 << 1
    };

    PlaybackState playbackState();
    qreal playbackSpeed();

public Q_SLOTS:
    void play();
    void pause();
    void playPause();
    void stop();

    void seek(int frameIndex, SeekFlags flags = PUSH_AUDIO);
    void previousFrame();
    void nextFrame();
    void previousKeyframe();
    void nextKeyframe();

    int visibleFrame();

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

    void setPlaybackSpeedPercent(int value);
    void setPlaybackSpeedNormalized(double value);

Q_SIGNALS:
    void sigPlaybackStateChanged(PlaybackState state);
    void sigFrameChanged();
    void sigPlaybackSpeedChanged(double normalizedSpeed);
    void sigPlaybackStatisticsUpdated();

private:
    void setPlaybackState(PlaybackState state);

    void nextKeyframeWithColor(int color);
    void nextKeyframeWithColor(const QSet<int> &validColors);
    void previousKeyframeWithColor(int color);
    void previousKeyframeWithColor(const QSet<int> &validColors);

    void updateDropFramesMode();
    KisTimeSpan activePlaybackRange();

    void setupAudioTracks();

    struct Private;
    QScopedPointer<Private> m_d;
};


class KRITAUI_EXPORT SingleShotSignal : public QObject {
    Q_OBJECT
public:
    SingleShotSignal(QObject* parent = nullptr)
        : QObject(parent)
        , lock(false)
    {
    }

    ~SingleShotSignal() {}

public Q_SLOTS:
    void tryFire() {
        if (!lock) {
            lock = true;
            emit output();
        }
    }

Q_SIGNALS:
    void output();

private:
    bool lock;

};


#endif
