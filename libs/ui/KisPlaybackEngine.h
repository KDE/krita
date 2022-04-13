#ifndef KISPLAYBACKENGINE_H
#define KISPLAYBACKENGINE_H

#include <QObject>
#include "KoCanvasObserverBase.h"
#include <kritaui_export.h>

#include <QScopedPointer>
#include <QFileInfo>


enum PlaybackMode {
    PLAYBACK_PUSH,
    PLAYBACK_PULL
};

enum SeekFlags {
    SEEK_NONE = 0,
    SEEK_PUSH_AUDIO = 1,
    SEEK_FORCE_RECACHE = 1 << 1
};

class KRITAUI_EXPORT KisPlaybackEngine : public QObject, public KoCanvasObserverBase
{
    Q_OBJECT
public:
    explicit KisPlaybackEngine(QObject *parent = nullptr);
    ~KisPlaybackEngine();

Q_SIGNALS:
    void sigChangeActiveCanvasFrame(int p_frame);

public Q_SLOTS:
    void play();
    void pause();
    void playPause();
    void stop();

    void seek(int frameIndex, SeekFlags flags = SEEK_PUSH_AUDIO);
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

    void setPlaybackSpeedPercent(int value);
    void setPlaybackSpeedNormalized(double value);


protected Q_SLOTS:
    void setCanvas(KoCanvasBase* canvas) override;
    void unsetCanvas() override;

private:
    void setupPlaybackMode(PlaybackMode p_mode);
    void nextKeyframeWithColor(int color);
    void nextKeyframeWithColor(const QSet<int> &validColors);
    void previousKeyframeWithColor(int color);
    void previousKeyframeWithColor(const QSet<int> &validColors);

    void setupProducerFromFile(QFileInfo file);

private:
    struct Private;
    QScopedPointer<Private> m_d;
};

#endif // KISPLAYBACKENGINE_H
