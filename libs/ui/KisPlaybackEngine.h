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

class KRITAUI_EXPORT KisPlaybackEngine : public QObject, public KoCanvasObserverBase
{
    Q_OBJECT
public:
    KisPlaybackEngine(QObject* parent = nullptr);
    ~KisPlaybackEngine();

public Q_SLOTS:
    virtual void play();
    virtual void pause();
    virtual void playPause();
    virtual void stop();

    virtual void seek( int frameIndex, SeekOptionFlags options = SEEK_FINALIZE | SEEK_PUSH_AUDIO ) = 0;
    virtual void previousFrame();
    virtual void nextFrame();
    virtual void previousKeyframe();
    virtual void nextKeyframe();

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

    virtual void setPlaybackSpeedPercent(int value) = 0;
    virtual void setPlaybackSpeedNormalized(double value) = 0;

    virtual void setMute(bool val) = 0;
    virtual bool isMute() = 0;

    virtual bool supportsAudio() = 0;
    virtual bool supportsVariablePlaybackSpeed() = 0;

protected:
    class KisCanvas2* activeCanvas() const;
    int frameWrap(int frame, int startFrame, int endFrame);

protected Q_SLOTS:
    virtual void setCanvas(KoCanvasBase* p_canvas) override;
    virtual void unsetCanvas() override;

private:
    void moveBy(int direction);
    void nextKeyframeWithColor(int color);
    void nextKeyframeWithColor(const QSet<int> &validColors);
    void previousKeyframeWithColor(int color);
    void previousKeyframeWithColor(const QSet<int> &validColors);

private:
    struct Private;
    QScopedPointer<Private> m_d;
};

#endif // KISPLAYBACKENGINE_H
