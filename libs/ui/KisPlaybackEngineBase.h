#ifndef KISPLAYBACKENGINE_H
#define KISPLAYBACKENGINE_H

#include "KoCanvasObserverBase.h"
#include <QObject>

#include <kritaui_export.h>

enum SeekOption {
    SEEK_NONE = 0,
    SEEK_PUSH_AUDIO = 1, // Whether we should be pushing audio or not. Used to prevent double-takes on scrubbing.
    SEEK_FORCE_RECACHE = 1 << 1,
    SEEK_FINALIZE = 1 << 2 // Force reload of KisImage to specific frame, ignore caching ability.
};

Q_DECLARE_FLAGS(SeekOptionFlags, SeekOption);
Q_DECLARE_OPERATORS_FOR_FLAGS(SeekOptionFlags);

class KRITAUI_EXPORT KisPlaybackEngineBase : public QObject, public KoCanvasObserverBase
{
    Q_OBJECT
public:
    KisPlaybackEngineBase(QObject* parent = nullptr);

public Q_SLOTS:
    virtual void play() = 0;
    virtual void pause() = 0;
    virtual void playPause() = 0;
    virtual void stop() = 0;

    virtual void seek( int frameIndex, SeekOptionFlags options = SEEK_FINALIZE | SEEK_PUSH_AUDIO ) = 0;
    virtual void previousFrame() = 0;
    virtual void nextFrame() = 0;
    virtual void previousKeyframe() = 0;
    virtual void nextKeyframe() = 0;

    /**
     * @brief previousMatchingKeyframe && nextMatchingKeyframe
     * Navigate to the next keyframe that has the same color-label
     * as the current keyframe. Useful to quickly navigate to user-specified
     * 'similar' keyframes. E.g. Contact points in an animation might have
     * a specific color to specify importance and be quickly swapped between.
     */
    virtual void previousMatchingKeyframe() = 0;
    virtual void nextMatchingKeyframe() = 0;

    /**
     * @brief previousUnfilteredKeyframe && nextUnfilteredKeyframe
     * Navigate to keyframes based on the current onion skin filtration.
     * This lets users easily navigate to the next visible "onion-skinned"
     * keyframe on the active layer.
     */
    virtual void previousUnfilteredKeyframe() = 0;
    virtual void nextUnfilteredKeyframe() = 0;

    virtual void setPlaybackSpeedPercent(int value) = 0;
    virtual void setPlaybackSpeedNormalized(double value) = 0;

    virtual void setMute(bool val) = 0;
    virtual bool isMute() = 0;
};

#endif // KISPLAYBACKENGINE_H
