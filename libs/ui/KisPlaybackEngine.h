#ifndef KISPLAYBACKENGINEMLT_H
#define KISPLAYBACKENGINEMLT_H

#include <QObject>
#include "KoCanvasObserverBase.h"
#include "KisPlaybackEngineBase.h"
#include <kritaui_export.h>

#include <QScopedPointer>
#include <QFileInfo>
#include <boost/optional.hpp>


enum PlaybackMode {
    PLAYBACK_PUSH, // MLT is being pushed to, used during pause and stop state for scrubbing.
    PLAYBACK_PULL // MLT is updating itself, we are getting regular updates from it about when we need to show our next frame.
};


class KRITAUI_EXPORT KisPlaybackEngineMLT : public KisPlaybackEngineBase
{
    Q_OBJECT
public:
    explicit KisPlaybackEngineMLT(QObject *parent = nullptr);
    ~KisPlaybackEngineMLT();

Q_SIGNALS:
    void sigChangeActiveCanvasFrame(int p_frame);

public Q_SLOTS:
    virtual void play() override;
    virtual void pause() override;
    virtual void playPause() override;
    virtual void stop() override;

    virtual void seek(int frameIndex, SeekOptionFlags flags = SEEK_FINALIZE | SEEK_PUSH_AUDIO) override;
    virtual void previousFrame() override;
    virtual void nextFrame() override;
    virtual void previousKeyframe() override;
    virtual void nextKeyframe() override;

    /**
     * @brief previousMatchingKeyframe && nextMatchingKeyframe
     * Navigate to the next keyframe that has the same color-label
     * as the current keyframe. Useful to quickly navigate to user-specified
     * 'similar' keyframes. E.g. Contact points in an animation might have
     * a specific color to specify importance and be quickly swapped between.
     */
    virtual void previousMatchingKeyframe() override;
    virtual void nextMatchingKeyframe() override;

    /**
     * @brief previousUnfilteredKeyframe && nextUnfilteredKeyframe
     * Navigate to keyframes based on the current onion skin filtration.
     * This lets users easily navigate to the next visible "onion-skinned"
     * keyframe on the active layer.
     */
    virtual void previousUnfilteredKeyframe() override;
    virtual void nextUnfilteredKeyframe() override;

    virtual void setPlaybackSpeedPercent(int value) override;
    virtual void setPlaybackSpeedNormalized(double value) override;

    virtual void setMute(bool val) override;
    virtual bool isMute() override;

protected Q_SLOTS:
    virtual void setCanvas(KoCanvasBase* canvas) override;
    virtual void unsetCanvas() override;

    /**
     * @brief throttledShowFrame
     * @param frame
     *
     * In order to throttle calls from MLT to respect our
     * playback mode, we need to redirect `showFrame` calls
     * to this thread and enforce that we only allow MLT to
     * show frames when we are in PULL mode.
     */
    void throttledShowFrame(const int frame);

    /**
     * @brief setAudioVolume
     * @param volume (normalized)
     */
    void setAudioVolume(qreal volumeNormalized);

private:
    void nextKeyframeWithColor(int color);
    void nextKeyframeWithColor(const QSet<int> &validColors);
    void previousKeyframeWithColor(int color);
    void previousKeyframeWithColor(const QSet<int> &validColors);

    void setupProducer(boost::optional<QFileInfo> file);

private:
    struct Private;
    struct StopAndResume;
    QScopedPointer<Private> m_d;
};

#endif // KISPLAYBACKENGINEMLT_H
