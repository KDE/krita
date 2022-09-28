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


class PlaybackDriver : public QObject {
    Q_OBJECT
public:
    PlaybackDriver( class KisPlaybackEngineQT* engine, QObject* parent = nullptr );
    ~PlaybackDriver();

    virtual void setPlaybackState(PlaybackState state) = 0;
    virtual void setFrame(int frame) {}
    virtual boost::optional<int> getDesiredFrame() { return boost::none; }
    virtual void setVolume(qreal volume) {}
    virtual void setSpeed(qreal speed) {}
    virtual void setFramerate(int rate) {}

    KisPlaybackEngineQT* engine() { return m_engine; }

Q_SIGNALS:
    void throttledShowFrame();

private:
    KisPlaybackEngineQT* m_engine;

};

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

    virtual bool supportsAudio() override { return false; }
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

#endif // KISPLAYBACKENGINEQT_H
