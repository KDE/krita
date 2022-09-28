/* This file is part of the KDE project
   SPDX-FileCopyrightText: 2022 Emmet O'Neill <emmetoneill.pdx@gmail.com>
   SPDX-FileCopyrightText: 2022 Eoin O'Neill <eoinoneill1991@gmail.com>

   SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "KisPlaybackEngineQT.h"

#include "kis_debug.h"
#include "kis_canvas2.h"
#include "KisCanvasAnimationState.h"
#include "kis_image.h"
#include "kis_image_animation_interface.h"

#include <QTimer>
#include "animation/KisFrameDisplayProxy.h"



#include "KisSyncedAudioPlayback.h"

#include "config-qtmultimedia.h"
#include "KisPlaybackEngineQT.h"

#ifdef HAVE_QT_MULTIMEDIA
#include <QtMultimedia/QMediaPlayer>
//TODO find nicer way to encapsulate these macros so that "mocked" QMediaPlayer is isolated from PlaybackEngineQT
#else
class QIODevice;

#include <QUrl>

namespace {

    class QMediaPlayer : public QObject {
        Q_OBJECT
    public:

    enum Error
    {
        NoError,
        ResourceError,
        FormatError,
        NetworkError,
        AccessDeniedError,
        ServiceMissingError,
        MediaIsPlaylist
    };

    enum State
    {
        StoppedState,
        PlayingState,
        PausedState
    };

    State state() const { return StoppedState; }

    void play() {}
    void stop() {}

    qint64 position() const { return 0; }
    qreal playbackRate() const { return 1.0; }
    void setPosition(qint64) {}
    void setPlaybackRate(qreal) {}
    void setVolume(int) {}
    void setMedia(const QUrl&, QIODevice * device = 0) { Q_UNUSED(device);}
    QString errorString() const { return QString(); }

    Q_SIGNALS:
    void error(Error value);
    };
}

#endif



#include <QFileInfo>


// ====

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

// ======


PlaybackDriver::PlaybackDriver( KisPlaybackEngineQT* engine, QObject* parent )
    : QObject(parent)
    , m_engine(engine)
{
    KIS_ASSERT(engine);
}

PlaybackDriver::~PlaybackDriver()
{
}


// ======

class LoopDrivenPlayback : public PlaybackDriver {
    Q_OBJECT
public:
    LoopDrivenPlayback(KisPlaybackEngineQT* engine, QObject* parent = nullptr);
    ~LoopDrivenPlayback();

    virtual void setPlaybackState( PlaybackState newState ) override;

    virtual void setFramerate(int rate) override;
    virtual void setSpeed(qreal speed) override;

private:
    void updatePlaybackLoopInterval(const int& in_fps, const qreal& in_speed);

private:
    QTimer m_playbackLoop;
    double m_speed;
    int m_fps;
};

LoopDrivenPlayback::LoopDrivenPlayback(KisPlaybackEngineQT *engine, QObject *parent)
    : PlaybackDriver(engine, parent)
    , m_speed(1.0)
    , m_fps(24)
{
    connect( &m_playbackLoop, SIGNAL(timeout()), this, SIGNAL(throttledShowFrame()) );
}

LoopDrivenPlayback::~LoopDrivenPlayback()
{
}

void LoopDrivenPlayback::setPlaybackState(PlaybackState newState) {
    switch (newState) {
    case PlaybackState::PLAYING:
        m_playbackLoop.start();
        break;
    case PlaybackState::PAUSED:
    case PlaybackState::STOPPED:
    default:
        m_playbackLoop.stop();
        break;
    }
}

void LoopDrivenPlayback::setFramerate(int rate) {
    KIS_SAFE_ASSERT_RECOVER_RETURN(rate > 0);
    m_fps = rate;
    updatePlaybackLoopInterval(m_fps, m_speed);
}

void LoopDrivenPlayback::setSpeed(qreal speed) {
    KIS_SAFE_ASSERT_RECOVER_RETURN(speed > 0.f);
    m_speed = speed;
    updatePlaybackLoopInterval(m_fps, m_speed);
}

void LoopDrivenPlayback::updatePlaybackLoopInterval(const int &in_fps, const qreal &in_speed) {
    int loopMS = qRound( 1000.f / (qreal(in_fps) * in_speed));
    m_playbackLoop.setInterval(loopMS);
}

// ======

/** @brief Plays media using a QMediaPlayer while looping at a consistent rate behind the scenes.
 * Unlike old QMediaPlayer based solution, this one tries to simplify behavior by forcing
 * our frame display to match the audio position via conversion to frame time. In other words,
 * we let the audio position drive our frame logic. */
class AudioDrivenPlayback : public PlaybackDriver
{
    Q_OBJECT
public:
    AudioDrivenPlayback( KisPlaybackEngineQT* engine, QFileInfo fileinfo, QObject* parent = nullptr );
    ~AudioDrivenPlayback() override;

    virtual void setPlaybackState(PlaybackState state) override;
    virtual void setFrame(int frame) override;
    virtual void setFramerate(int) override;
    virtual boost::optional<int> getDesiredFrame() override;
    virtual void setVolume(qreal value) override;
    virtual void setSpeed(qreal speed) override;

Q_SIGNALS:
    void error(const QString &filename, const QString &message);


private Q_SLOTS:
    void slotOnError();
    void timerElapsed();

protected:
    void updatePlaybackLoopInterval(const int &in_fps, const qreal &in_speed);

private:
    struct Private;
    const QScopedPointer<Private> m_d;
};

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
    QScopedPointer<AudioPlaybackMemory> playbackMemory;
    QTimer playbackLoop;
};


AudioDrivenPlayback::AudioDrivenPlayback(KisPlaybackEngineQT* engine, QFileInfo fileinfo, QObject* parent )
    : PlaybackDriver(engine, parent),
      m_d(new AudioDrivenPlayback::Private)
{

    connect(&m_d->player, SIGNAL(error(QMediaPlayer::Error)), SLOT(slotOnError()));
    connect(&m_d->playbackLoop, SIGNAL(timeout()), SLOT(timerElapsed()));

    KIS_ASSERT(fileinfo.isFile());
    QString mimetype = QMimeDatabase().mimeTypeForFile( fileinfo.absoluteFilePath() ).name();

    if (m_d->player.hasSupport(mimetype)) {
        // Buffer file immediately, do not wait for playback to begin. This should help fix
        // "lag" caused by loading the file when requesting to play the media.
        // This might result in small slowdown on initial file loading, but that should beat
        // the alternative.

        m_d->player.setMedia(QUrl::fromLocalFile(fileinfo.absoluteFilePath()));
    } else {
        error(fileinfo.absoluteFilePath(), i18nc("Error callback when file is unsupported by QMediaPlayer. String is followed by file name fed to mediaplayer",
                                         "QMediaPlayer either does not support this file or is missing the appropriate codecs:"));
    }

    m_d->player.setVolume(100);
    m_d->player.setPlaybackRate(1.0);

}

AudioDrivenPlayback::~AudioDrivenPlayback()
{
}

void AudioDrivenPlayback::setPlaybackState(PlaybackState newState)
{
    if (convertToMediaPlayerState(newState) != m_d->player.state()) {
        switch(newState) {
            case PLAYING:
                m_d->player.play();
                m_d->playbackLoop.start();
            break;
            case PAUSED:
                m_d->player.pause();
                m_d->playbackLoop.stop();
                m_d->playbackMemory.reset();
            break;
            case STOPPED:
                m_d->player.stop();
                m_d->playbackLoop.stop();
                m_d->playbackMemory.reset();
            break;
        }
    }
}

void AudioDrivenPlayback::setFrame(int frame)
{
    int framerate = engine()->activeFramesPerSecond().get_value_or(24);
    qint64 msec = framesToMSec(frame, framerate);
    m_d->player.setPosition(msec);
}

void AudioDrivenPlayback::setFramerate(int)
{
    updatePlaybackLoopInterval(engine()->activeFramesPerSecond().get_value_or(24), m_d->player.playbackRate());
}

boost::optional<int> AudioDrivenPlayback::getDesiredFrame()
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
        m_d->player.stop();
        m_d->player.setPlaybackRate(value);
        m_d->player.setPosition(oldPosition);
        m_d->player.play();
    } else if (m_d->player.state() == QMediaPlayer::PausedState) {
        m_d->player.stop();
        m_d->player.setPlaybackRate(value);
        m_d->player.setPosition(oldPosition);
    } else {
        m_d->player.setPlaybackRate(value);
    }

    updatePlaybackLoopInterval(engine()->activeFramesPerSecond().get_value_or(24), m_d->player.playbackRate());
}

void AudioDrivenPlayback::slotOnError()
{
#ifdef HAVE_QT_MULTIMEDIA
#if QT_VERSION >= QT_VERSION_CHECK(5,14,0)
    emit error(m_d->player.media().request().url().toLocalFile(), m_d->player.errorString());
#else
    emit error(m_d->player.media().canonicalUrl().toLocalFile(), m_d->player.errorString());
#endif
#endif
}

void AudioDrivenPlayback::timerElapsed()
{
    const int pos = m_d->player.position();
    const int currentFrame = msecToFrames(pos, engine()->activeFramesPerSecond().get_value_or(24));

    bool needsNotify = false;
    if (!m_d->playbackMemory) {
        m_d->playbackMemory.reset(new AudioPlaybackMemory);
        needsNotify = true;
    } else {
        const int lastFrame = msecToFrames(m_d->playbackMemory->lastTimeMSec, engine()->activeFramesPerSecond().get_value_or(24));
        needsNotify = lastFrame != currentFrame;
    }

    if (needsNotify) {
        throttledShowFrame();
    }

    m_d->playbackMemory->lastTimeMSec = pos;
}

void AudioDrivenPlayback::updatePlaybackLoopInterval(const int &in_fps, const qreal &in_speed) {
    int loopMS = qRound( 1000.f / (qreal(in_fps) * in_speed));
    m_d->playbackLoop.setInterval(loopMS);
}


struct KisPlaybackEngineQT::Private {
public:
    Private(KisPlaybackEngineQT* p_self)
        : driver(nullptr)
    {
    }

    ~Private() {
        driver.reset();
    }

    QScopedPointer< PlaybackDriver > driver;

private:
    KisPlaybackEngineQT* self;
};

// ======

KisPlaybackEngineQT::KisPlaybackEngineQT(QObject *parent)
    : KisPlaybackEngine(parent)
    , m_d(new Private(this))
{
}

KisPlaybackEngineQT::~KisPlaybackEngineQT()
{

}

void KisPlaybackEngineQT::seek(int frameIndex, SeekOptionFlags flags)
{
    if (!activeCanvas())
        return;

    KIS_SAFE_ASSERT_RECOVER_RETURN(activeCanvas()->animationState());
    KisFrameDisplayProxy* displayProxy = activeCanvas()->animationState()->displayProxy();
    KIS_SAFE_ASSERT_RECOVER_RETURN(displayProxy);

    m_d->driver->setFrame(frameIndex);
    if (displayProxy->activeFrame() != frameIndex) {
        displayProxy->displayFrame(frameIndex, flags & SEEK_FINALIZE);
    }
}

void KisPlaybackEngineQT::setPlaybackSpeedPercent(int percentage)
{
    setPlaybackSpeedNormalized(qreal(percentage) / 100.f);
}

void KisPlaybackEngineQT::setPlaybackSpeedNormalized(double value)
{
    KIS_SAFE_ASSERT_RECOVER_RETURN(m_d->driver);
    m_d->driver->setSpeed(value);
}

boost::optional<int64_t> KisPlaybackEngineQT::activeFramesPerSecond()
{
    if (activeCanvas()) {
        return activeCanvas()->image()->animationInterface()->framerate();
    } else {
        return boost::none;
    }
}

void KisPlaybackEngineQT::throttledDriverCallback()
{
    if (!m_d->driver)
        return;

    KIS_SAFE_ASSERT_RECOVER_RETURN(activeCanvas()->animationState());
    KisFrameDisplayProxy* displayProxy = activeCanvas()->animationState()->displayProxy();
    KIS_SAFE_ASSERT_RECOVER_RETURN(displayProxy);

    KIS_SAFE_ASSERT_RECOVER_RETURN(activeCanvas()->image());
    KisImageAnimationInterface *animInterface = activeCanvas()->image()->animationInterface();
    KIS_SAFE_ASSERT_RECOVER_RETURN(animInterface);

    const int currentFrame = displayProxy->activeFrame();
    const int startFrame = animInterface->activePlaybackRange().start();
    const int endFrame = animInterface->activePlaybackRange().end();

    // If we have an audio playback engine, we will only go to what the audio determines to be the desired frame...
    if (m_d->driver->getDesiredFrame()) {
        const int desiredFrame = m_d->driver->getDesiredFrame().get();

        const int targetFrame = frameWrap(desiredFrame, startFrame, endFrame );

        if (currentFrame != targetFrame) {
            displayProxy->displayFrame(targetFrame, false);
        }

        // We've wrapped, let's do whatever correction we can...
        if (targetFrame != desiredFrame) {
            m_d->driver->setFrame(targetFrame);
        }

    } else { // Otherwise, we just advance the frame ourselves based on the displayProxy's active frame.
        int targetFrame = currentFrame + 1;

        targetFrame = frameWrap(targetFrame, startFrame, endFrame);

        if (currentFrame != targetFrame) {
            displayProxy->displayFrame(targetFrame, false);
        }
    }
}

void KisPlaybackEngineQT::setCanvas(KoCanvasBase *p_canvas)
{
    KisCanvas2* canvas = dynamic_cast<KisCanvas2*>(p_canvas);

    struct StopAndResume {
        StopAndResume(KisPlaybackEngineQT* p_self)
            : m_self(p_self) {
            if (m_self->m_d->driver) {
                m_self->m_d->driver->setPlaybackState(PlaybackState::STOPPED);
            }
        }

        ~StopAndResume() {
            if (m_self->activeCanvas() &&  m_self->m_d->driver) {
                m_self->m_d->driver->setPlaybackState(m_self->activeCanvas()->animationState()->playbackState());
            }
        }

    private:
        KisPlaybackEngineQT* m_self;
    };

    if (activeCanvas() == canvas) {
        return;
    }

    if (activeCanvas()) {
        KisCanvasAnimationState* animationState = activeCanvas()->animationState();

        KIS_SAFE_ASSERT_RECOVER_RETURN(m_d->driver);

        m_d->driver.data()->disconnect(this);

        // Disconnect old player, prepare for new one..
        if (animationState) {
            this->disconnect(animationState);
            animationState->disconnect(this);
        }

        // Disconnect old image, prepare for new one..
        auto image = activeCanvas()->image();
        if (image && image->animationInterface()) {
            this->disconnect(image->animationInterface());
            image->animationInterface()->disconnect(this);
        }
    }

    StopAndResume stopResume(this);

    KisPlaybackEngine::setCanvas(canvas);

    if (activeCanvas()) {
        KisCanvasAnimationState* animationState = activeCanvas()->animationState();
        KIS_ASSERT(animationState);

        recreateDriver(animationState->mediaInfo());

        KIS_SAFE_ASSERT_RECOVER_RETURN(m_d->driver);

        connect(animationState, &KisCanvasAnimationState::sigPlaybackMediaChanged, this, [this]() {
            KisCanvasAnimationState* animationState2 = activeCanvas()->animationState();
            if (animationState2) {
                recreateDriver(animationState2->mediaInfo());
            }
        });

        connect(m_d->driver.data(), SIGNAL(throttledShowFrame()), this, SLOT(throttledDriverCallback()));

        connect(animationState, &KisCanvasAnimationState::sigPlaybackStateChanged, this, [this](PlaybackState state){
            if (!m_d->driver)
                return;

            m_d->driver->setPlaybackState(state);
        });

        auto image = activeCanvas()->image();
        KIS_ASSERT(image);
        KisImageAnimationInterface* aniInterface = image->animationInterface();
        KIS_ASSERT(aniInterface);

        connect(aniInterface, &KisImageAnimationInterface::sigFramerateChanged, this, [this](){
            if (!activeCanvas())
                return;

            KisImageWSP img = activeCanvas()->image();
            KIS_SAFE_ASSERT_RECOVER_RETURN(img);
            KisImageAnimationInterface* aniInterface = img->animationInterface();
            KIS_SAFE_ASSERT_RECOVER_RETURN(aniInterface);

            m_d->driver->setFramerate(aniInterface->framerate());
        });

        m_d->driver->setFramerate(aniInterface->framerate());
    }
    else {
        recreateDriver(boost::none);
    }
}

void KisPlaybackEngineQT::unsetCanvas()
{
    setCanvas(nullptr);
}

void KisPlaybackEngineQT::recreateDriver(boost::optional<QFileInfo> file)
{
    m_d->driver.reset();

    if (!activeCanvas())
        return;

    if (file) {
        m_d->driver.reset(new AudioDrivenPlayback(this, file.get()));
    } else {
        m_d->driver.reset(new LoopDrivenPlayback(this));
    }
}


#include "KisPlaybackEngineQT.moc"
