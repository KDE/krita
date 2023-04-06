/* This file is part of the KDE project
   SPDX-FileCopyrightText: 2022 Emmet O'Neill <emmetoneill.pdx@gmail.com>
   SPDX-FileCopyrightText: 2022 Eoin O'Neill <eoinoneill1991@gmail.com>

   SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KISPLAYBACKENGINEQT_H
#define KISPLAYBACKENGINEQT_H

#include "KisPlaybackEngine.h"

#include <kritaui_export.h>


#include "canvas/KisCanvasAnimationState.h"
#include <boost/optional.hpp>


#include <QElapsedTimer>

/**
 * @brief The KisPlaybackEngineQT class is an implementation of KisPlaybackEngine
 * that drives animation playback using simple Qt functionality alone.
 *
 * As of right now, this playback engine is used as a fallback for when KisPlaybackEngineMLT is unavailable,
 * invalid, or otherwise unwanted.
 */
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

    virtual bool supportsAudio() override;
    virtual bool supportsVariablePlaybackSpeed() override { return true; }

    boost::optional<int64_t> activeFramesPerSecond();

protected Q_SLOTS:
    /**
     * @brief throttledDriverCallback handles signals from the internal driver
     * that drives playback within this engine. It will either increment frame time,
     * wrapping within bounds, and communicate with KisFrameDisplayProxy or use the
     * driver's desired time to control which frame is visible...
     */
    void throttledDriverCallback();


protected:
    virtual void setCanvas(KoCanvasBase* canvas) override;
    virtual void unsetCanvas() override;

private:
    void recreateDriver(boost::optional<QFileInfo> file);

private:
    struct Private;
    QScopedPointer<Private> m_d;
};

class PlaybackDriver : public QObject {
    Q_OBJECT
public:
    PlaybackDriver( class KisPlaybackEngineQT* engine, QObject* parent = nullptr );
    ~PlaybackDriver();

    virtual void setPlaybackState(PlaybackState state) = 0;

    virtual void setFrame(int) {}
    virtual boost::optional<int> desiredFrame() { return boost::none; }

    virtual void setVolume(qreal) {}

    virtual void setSpeed(qreal) {}
    virtual double speed() = 0;

    virtual void setFramerate(int rate) {}

    virtual void setDropFrames(bool) {}
    virtual bool dropFrames() { return true; }

    virtual void setTimerDuration(int) {}
    virtual int timerDuration() { return 1000 / 24; }

    KisPlaybackEngineQT* engine() { return m_engine; }


Q_SIGNALS:
    void throttledShowFrame();

private:
    KisPlaybackEngineQT* m_engine;
    QElapsedTimer time;
    int m_measureRemainder = 0;

};

#endif // KISPLAYBACKENGINEQT_H
