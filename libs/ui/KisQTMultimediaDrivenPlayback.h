/*
 *  SPDX-License-Identifier: GPL-3.0-or-later
 */

#ifndef AUDIODRIVENPLAYBACK_H
#define AUDIODRIVENPLAYBACK_H

#include <kritaui_export.h>

#include "KisPlaybackEngineQT.h"

/** @brief Plays media using a QMediaPlayer while looping at a consistent rate behind the scenes.
 * Unlike old QMediaPlayer based solution, this one tries to simplify behavior by forcing
 * our frame display to match the audio position via conversion to frame time. In other words,
 * we let the audio position drive our frame logic. */
class KRITAUI_EXPORT AudioDrivenPlayback : public PlaybackDriver
{
    Q_OBJECT
public:
    AudioDrivenPlayback( KisPlaybackEngineQT* engine, QFileInfo fileinfo, QObject* parent = nullptr );
    ~AudioDrivenPlayback() override;

    virtual void setPlaybackState(PlaybackState state) override;
    virtual void setFrame(int frame) override;
    virtual void setFramerate(int) override;
    virtual boost::optional<int> desiredFrame() override;
    virtual void setVolume(qreal value) override;
    virtual void setSpeed(qreal speed) override;
    virtual double speed() override;

    bool deviceAvailability();

Q_SIGNALS:
    void error(const QString &filename, const QString &message);

private Q_SLOTS:
    void slotOnError();
    void timerElapsed();

protected:
    void updatePlaybackLoopInterval(const int &in_fps, const qreal &in_speed);

private:
    struct Private;
    QScopedPointer<Private> m_d;
};

#endif
