/*
 *  SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "KisQTMultimediaDrivenPlayback.h"
#include "config-qtmultimedia.h"

#ifdef HAVE_QT_MULTIMEDIA

#include <QMediaPlayer>
#include <QMediaService>
#include <QAudioDeviceInfo>
#include <QMimeDatabase>

qint64 framesToMSec(qreal value, int fps) {
    return qRound((value / fps) * 1000.0);
}

qreal msecToFrames(qint64 value, int fps) {
    return qreal(value) * fps / 1000.0;
}

int framesToScaledTimeMS(qreal frame, int fps, qreal playbackSpeed) {
    return qRound(framesToMSec(frame, fps) / playbackSpeed);
}

qreal scaledTimeToFrames(qint64 time, int fps, qreal playbackSpeed) {
    return msecToFrames(time, fps) * playbackSpeed;
}

/**
 * @brief The memory of the last updates recieved from the QMediaPlayer.
 *
 * QMediaPlayer will update us spontaneously on a new position. We need to
 * keep track of the last position if available on each update to determine
 * whether or not we should or should not be updating the playback position
 * on a frame change.
 */
struct AudioPlaybackMemory {
    qint64 lastTimeMSec = 0;
};

struct AudioDrivenPlayback::Private {
    QMediaPlayer player;
    QTimer playbackLoop;
    QScopedPointer<AudioPlaybackMemory> playbackMemory;
};

AudioDrivenPlayback::AudioDrivenPlayback(KisPlaybackEngineQT* engine, QFileInfo fileinfo, QObject* parent )
    : PlaybackDriver(engine, parent),
      m_d(new AudioDrivenPlayback::Private)
{

    connect(&m_d->player, SIGNAL(error(QMediaPlayer::Error)), SLOT(slotOnError()));
    connect(&m_d->playbackLoop, SIGNAL(timeout()), SLOT(timerElapsed()));

    KIS_ASSERT(fileinfo.isFile());
    QString mimetype = QMimeDatabase().mimeTypeForFile(fileinfo.absoluteFilePath()).name();

    if (m_d->player.hasSupport(mimetype)) {
        // Buffer file immediately, do not wait for playback to begin. This should help fix
        // "lag" caused by loading the file when requesting to play the media.
        // This might result in small slowdown on initial file loading, but that should beat
        // the alternative.
        m_d->player.setMedia(QUrl::fromLocalFile(fileinfo.absoluteFilePath()));
    } else {
        emit error(fileinfo.absoluteFilePath(),
                   i18nc("Error callback when file is unsupported by QMediaPlayer. String is followed by file name fed to mediaplayer",
                                         "QMediaPlayer either does not support this file or is missing the appropriate codecs:"));
    }

    m_d->player.setVolume(100);
    m_d->player.setPlaybackRate(1.0);

}


#include <QtMultimedia/QMediaPlayer>

QMediaPlayer::State convertToMediaPlayerState(PlaybackState canvasState) {
    switch(canvasState) {
        case PLAYING:
            return QMediaPlayer::PlayingState;
        case PAUSED:
            return QMediaPlayer::PausedState;
        case STOPPED:
        default:
            return QMediaPlayer::StoppedState;
    }
}

AudioDrivenPlayback::~AudioDrivenPlayback()
{
}

void AudioDrivenPlayback::setPlaybackState(PlaybackState newState)
{
    if (convertToMediaPlayerState(newState) != m_d->player.state()) {
        switch(newState) {
            case PLAYING:
                m_d->playbackLoop.start();
                m_d->player.play();
                m_d->playbackMemory.reset(new AudioPlaybackMemory);
            break;
            case PAUSED:
                m_d->playbackLoop.stop();
                m_d->player.stop();
                m_d->playbackMemory.reset();
            break;
            case STOPPED:
                m_d->playbackLoop.stop();
                m_d->player.stop();
                m_d->playbackMemory.reset();
            break;
        }
    }
}

void AudioDrivenPlayback::setFrame(int frame)
{
    const int framerate = engine()->activeFramesPerSecond().get_value_or(24);
    qint64 msec = framesToMSec(frame, framerate);
    m_d->player.setPosition(msec);
}

void AudioDrivenPlayback::setFramerate(int)
{
    const int framerate = engine()->activeFramesPerSecond().get_value_or(24);
    updatePlaybackLoopInterval(framerate, m_d->player.playbackRate());
}

boost::optional<int> AudioDrivenPlayback::desiredFrame()
{
    int framerate = engine()->activeFramesPerSecond().get_value_or(24);
    return msecToFrames(m_d->player.position(), framerate);
}

void AudioDrivenPlayback::setVolume(qreal value)
{
    m_d->player.setVolume(qRound(100.0 * value));
}

void AudioDrivenPlayback::setSpeed(qreal value)
{
    if (qFuzzyCompare(value, m_d->player.playbackRate())) return;

    const qint64 oldPosition = m_d->player.position();
    if (m_d->player.state() == QMediaPlayer::PlayingState) {
        m_d->player.pause();
        m_d->player.setPlaybackRate(value);
        m_d->player.setPosition(oldPosition);
        m_d->player.play();
    } else if (m_d->player.state() == QMediaPlayer::PausedState) {
        m_d->player.setPlaybackRate(value);
        m_d->player.setPosition(oldPosition);
    } else {
        m_d->player.setPlaybackRate(value);
    }

    updatePlaybackLoopInterval(engine()->activeFramesPerSecond().get_value_or(24), m_d->player.playbackRate());
}

double AudioDrivenPlayback::speed()
{
    return m_d->player.playbackRate();
}

bool AudioDrivenPlayback::deviceAvailability()
{
    const QList<QAudioDeviceInfo> deviceDetails = QAudioDeviceInfo::availableDevices(QAudio::AudioOutput);
    return deviceDetails.size() > 0;
}

void AudioDrivenPlayback::slotOnError()
{
#if QT_VERSION >= QT_VERSION_CHECK(5,14,0)
    emit error(m_d->player.media().request().url().toLocalFile(), m_d->player.errorString());
#else // QT_VERSION < QT_VERSION_CHECK(5, 14, 0)
    emit error(m_d->player.media().canonicalUrl().toLocalFile(), m_d->player.errorString());
#endif // QT_VERSION
}

void AudioDrivenPlayback::timerElapsed()
{
    KIS_SAFE_ASSERT_RECOVER_RETURN(m_d->playbackMemory);
    const int pos = m_d->player.position();
    const int currentFrame = msecToFrames(pos, engine()->activeFramesPerSecond().get_value_or(24));

    const int lastFrame = msecToFrames(m_d->playbackMemory->lastTimeMSec, engine()->activeFramesPerSecond().get_value_or(24));
    bool needsNotify = lastFrame != currentFrame;

    if (needsNotify) {
        throttledShowFrame();
    }
}

void AudioDrivenPlayback::updatePlaybackLoopInterval(const int &in_fps, const qreal &in_speed) {
    int loopMS = qRound( 1000.f / (qreal(in_fps) * in_speed));
    m_d->playbackLoop.setInterval(loopMS);
}

#endif // HAVE_QT_MULTIMEDIA
