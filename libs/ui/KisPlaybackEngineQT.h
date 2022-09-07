#ifndef KISPLAYBACKENGINEQT_H
#define KISPLAYBACKENGINEQT_H

#include "KisPlaybackEngine.h"

#include <kritaui_export.h>

class KRITAUI_EXPORT KisPlaybackEngineQT : public KisPlaybackEngine
{
    Q_OBJECT
public:
    explicit KisPlaybackEngineQT(QObject *parent = nullptr);
    ~KisPlaybackEngineQT();

    virtual void seek(int frameIndex, SeekOptionFlags flags = SEEK_FINALIZE | SEEK_PUSH_AUDIO) override;

    virtual void setPlaybackSpeedPercent(int percentage) override;
    virtual void setPlaybackSpeedNormalized(double value) override;

    virtual void setMute(bool) override {}
    virtual bool isMute() override { return true; }

    virtual bool supportsAudio() override { return false; }
    virtual bool supportsVariablePlaybackSpeed() override { return true; }

protected Q_SLOTS:
    void throttledQtFrameTimeExpired();

protected:
    virtual void setCanvas(KoCanvasBase* canvas) override;
    virtual void unsetCanvas() override;

private:
    struct Private;
    QScopedPointer<Private> m_d;
};

#endif // KISPLAYBACKENGINEQT_H
