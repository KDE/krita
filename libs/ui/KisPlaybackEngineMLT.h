#ifndef KISPLAYBACKENGINEMLT_H
#define KISPLAYBACKENGINEMLT_H

#include <QObject>
#include "KoCanvasObserverBase.h"
#include "KisPlaybackEngine.h"
#include <kritaui_export.h>

#include <QScopedPointer>
#include <QFileInfo>
#include <boost/optional.hpp>


enum PlaybackMode {
    PLAYBACK_PUSH, // MLT is being pushed to, used during pause and stop state for scrubbing.
    PLAYBACK_PULL // MLT is updating itself, we are getting regular updates from it about when we need to show our next frame.
};


class KRITAUI_EXPORT KisPlaybackEngineMLT : public KisPlaybackEngine
{
    Q_OBJECT
public:
    explicit KisPlaybackEngineMLT(QObject *parent = nullptr);
    ~KisPlaybackEngineMLT();

Q_SIGNALS:
    void sigChangeActiveCanvasFrame(int p_frame);

public Q_SLOTS:
    virtual void seek(int frameIndex, SeekOptionFlags flags = SEEK_FINALIZE | SEEK_PUSH_AUDIO) override;

    virtual void setPlaybackSpeedPercent(int value) override;
    virtual void setPlaybackSpeedNormalized(double value) override;

    virtual void setMute(bool val) override;
    virtual bool isMute() override;

    virtual bool supportsAudio() override { return true; }
    virtual bool supportsVariablePlaybackSpeed() override { return true; }

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

    void throttledSetSpeed(const double speed);


    /**
     * @brief setAudioVolume
     * @param volume (normalized)
     */
    void setAudioVolume(qreal volumeNormalized);

private:
    void setupProducer(boost::optional<QFileInfo> file);

    struct Private;
    struct StopAndResume;
    QScopedPointer<Private> m_d;
};

#endif // KISPLAYBACKENGINEMLT_H
