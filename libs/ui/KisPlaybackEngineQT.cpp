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

#include "config-qtmultimedia.h"
#include "KisQTMultimediaDrivenPlayback.h"

#include "KisPlaybackEngineQT.h"

#include <QFileInfo>

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

    // If we have an qtmultimedia playback driver: we will only go to what the audio determines to be the desired frame...
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

    m_d->driver.reset(new LoopDrivenPlayback(this));

#ifdef HAVE_QT_MULTIMEDIA
    if (file) {
        m_d->driver.reset(new AudioDrivenPlayback(this, file.get()));
    }
#endif
}


#include "KisPlaybackEngineQT.moc"
