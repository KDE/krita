/* This file is part of the KDE project
   SPDX-FileCopyrightText: 2022 Emmet O'Neill <emmetoneill.pdx@gmail.com>
   SPDX-FileCopyrightText: 2022 Eoin O'Neill <eoinoneill1991@gmail.com>

   SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KISPLAYBACKENGINE_H
#define KISPLAYBACKENGINE_H

#include "KoCanvasObserverBase.h"
#include <QObject>

#include <kritaui_export.h>

/**
 * @brief The SeekOption enum represents additional behaviors associated with seeking to a new frame.
 * For example, sometimes you want to push audio when seeking, and other times you might want to reload the image without using the cache.
 * Other optional, uncommon or specialized seeking behaviors might make sense to add here.
 */
enum SeekOption {
    SEEK_NONE = 0,
    SEEK_PUSH_AUDIO = 1, // Whether we should be pushing audio or not. Used to prevent double-takes on scrubbing.
    SEEK_FINALIZE = 1 << 1 // Force reload of KisImage to specific frame, ignore caching ability.
};

Q_DECLARE_FLAGS(SeekOptionFlags, SeekOption)
Q_DECLARE_OPERATORS_FOR_FLAGS(SeekOptionFlags)


/** @brief Krita's base animation playback engine for producing image frame changes and associated audio.
 *
 *  Krita stores a main playback engine in KisPart (a singleton) to be used by the active document's canvas.
 *  It can be thought of as being just below the GUI layer of Krita's animation system,
 *  responding to various GUI events, controlling transport controls (play, stop, next, etc.),
 *  and generally driving the playback of animation within Krita.
 *
 *  It's implemented by KisPlaybackEngineQT and KisPlaybackEngineMLT, one of which is typically selected
 *  at compile time depending on available dependencies. Specific implementations may or may not support
 *  certain features (audio, speed, dropping frames, etc.) and have other different characteristics.
 */
class KRITAUI_EXPORT KisPlaybackEngine : public QObject, public KoCanvasObserverBase
{
    Q_OBJECT
public:
    KisPlaybackEngine(QObject* parent = nullptr);
    ~KisPlaybackEngine();

    struct PlaybackStats {
        qreal expectedFps {0.0};
        qreal realFps {0.0};
        qreal droppedFramesPortion {0.0};
    };

public Q_SLOTS:
    // Basic transport controls...
    virtual void play();
    virtual void pause();
    virtual void playPause();
    virtual void stop();

    virtual void seek( int frameIndex, SeekOptionFlags options = SEEK_FINALIZE | SEEK_PUSH_AUDIO ) = 0;
    virtual void previousFrame();
    virtual void nextFrame();
    virtual void previousKeyframe();
    virtual void nextKeyframe();
    virtual void firstFrame();
    virtual void lastFrame();

    /**
     * @brief previousMatchingKeyframe && nextMatchingKeyframe
     * Navigate to the next keyframe that has the same color-label
     * as the current keyframe. Useful to quickly navigate to user-specified
     * 'similar' keyframes. E.g. Contact points in an animation might have
     * a specific color to specify importance and be quickly swapped between.
     */
    virtual void previousMatchingKeyframe();
    virtual void nextMatchingKeyframe();

    /**
     * @brief previousUnfilteredKeyframe && nextUnfilteredKeyframe
     * Navigate to keyframes based on the current onion skin filtration.
     * This lets users easily navigate to the next visible "onion-skinned"
     * keyframe on the active layer.
     */
    virtual void previousUnfilteredKeyframe();
    virtual void nextUnfilteredKeyframe();

    // Audio controls...
    virtual void setMute(bool val) = 0;
    virtual bool isMute() = 0;

    virtual void setDropFramesMode(bool value);
    bool dropFrames() const;

    virtual bool supportsAudio() = 0;
    virtual bool supportsVariablePlaybackSpeed() = 0;

    virtual PlaybackStats playbackStatistics() const = 0;

Q_SIGNALS:
    void sigDropFramesModeChanged(bool value);

protected:
    class KisCanvas2* activeCanvas() const;
    int frameWrap(int frame, int startFrame, int endFrame);

protected Q_SLOTS:
    virtual void setCanvas(KoCanvasBase* p_canvas) override;
    virtual void unsetCanvas() override;

private:
    /**
     * @brief Move the active frame of the animation player forwards or backwards by some number of frames.
     * @param The signed number of frames to move. Negative frames move backwards in time (left on a left-to-right system).
     */
    void moveActiveFrameBy(int frames);

    // Used by previous/next unfiltered keyframe functions...
    void nextKeyframeWithColor(int color);
    void nextKeyframeWithColor(const QSet<int> &validColors);
    void previousKeyframeWithColor(int color);
    void previousKeyframeWithColor(const QSet<int> &validColors);

private:
    struct Private;
    QScopedPointer<Private> m_d;
};

#endif // KISPLAYBACKENGINE_H
