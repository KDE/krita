/* This file is part of the KDE project
   SPDX-FileCopyrightText: 2022 Emmet O'Neill <emmetoneill.pdx@gmail.com>
   SPDX-FileCopyrightText: 2022 Eoin O'Neill <eoinoneill1991@gmail.com>

   SPDX-License-Identifier: LGPL-2.0-or-later
*/

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

/**
 * @brief The KisPlaybackEngineMLT class is an implementation of KisPlaybackEngine
 * that uses MLT (Media Lovin' Toolkit) to drive image frame changes and animation audio
 * with (hopefully) close to frame-perfect synchronization.
 *
 * If MLT is unavailable or unwanted, Krita can instead use KisPlaybackEngineQT
 * which may be simpler but has different characteristics and is not designed with
 * audio-video synchronization in mind.
 */
class KRITAUI_EXPORT KisPlaybackEngineMLT : public KisPlaybackEngine
{
    Q_OBJECT
public:
    explicit KisPlaybackEngineMLT(QObject *parent = nullptr);
    ~KisPlaybackEngineMLT();

Q_SIGNALS:
    void sigChangeActiveCanvasFrame(int p_frame);

public Q_SLOTS:
    void seek(int frameIndex, SeekOptionFlags flags = SEEK_FINALIZE | SEEK_PUSH_AUDIO) override;

    void setMute(bool val) override;
    bool isMute() override;

    bool supportsAudio() override { return true; }
    bool supportsVariablePlaybackSpeed() override { return true; }

    void setDropFramesMode(bool value) override;

    PlaybackStats playbackStatistics() const override;

protected Q_SLOTS:
    void setCanvas(KoCanvasBase* canvas) override;
    void unsetCanvas() override;
    void canvasDestroyed(QObject *canvas);

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
     * @brief throttledSetSpeed
     * @param speed
     *
     * Because MLT needs to be stopped and restarted to change playback speed
     * we use this function to limit the frequency of speed change requests.
     */
    void throttledSetSpeed(const double speed);


    /**
     * @brief setAudioVolume
     * @param volume (normalized)
     */
    void setAudioVolume(qreal volumeNormalized);

public:
    struct FrameWaitingInterface;
    FrameWaitingInterface* frameWaitingInterface();

private:
    /**
     * @brief Sets up an MLT::Producer object in response to audio being
     * added to a Krita document or when canvas changes.
     * @param file: An optional file to be loaded by MLT.
     */
    void setupProducer(boost::optional<QFileInfo> file);

    struct Private;
    struct StopAndResume;
    QScopedPointer<Private> m_d;
};

#endif // KISPLAYBACKENGINEMLT_H
