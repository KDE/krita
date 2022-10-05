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
    virtual double speed() override;
    virtual void setDropFrames(bool drop) override;
    virtual bool dropFrames() override;
    virtual void setTimerDuration(int timeMS) override;
    virtual int timerDuration() override;

private:
    void updatePlaybackLoopInterval(const int& in_fps, const qreal& in_speed);

private:
    QTimer m_playbackLoop;
    double m_speed;
    int m_fps;
    bool m_dropFrames;
};

LoopDrivenPlayback::LoopDrivenPlayback(KisPlaybackEngineQT *engine, QObject *parent)
    : PlaybackDriver(engine, parent)
    , m_speed(1.0)
    , m_fps(24)
    , m_dropFrames(true)
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

double LoopDrivenPlayback::speed()
{
    return m_speed;
}

void LoopDrivenPlayback::setDropFrames(bool drop) {
    m_dropFrames = drop;
}

bool LoopDrivenPlayback::dropFrames() {
    return m_dropFrames;
}

void LoopDrivenPlayback::setTimerDuration(int timeMS)
{
    KIS_ASSERT(timeMS > 0);
    m_playbackLoop.setInterval(timeMS);
}

int LoopDrivenPlayback::timerDuration()
{
    return m_playbackLoop.interval();
}

void LoopDrivenPlayback::updatePlaybackLoopInterval(const int &in_fps, const qreal &in_speed) {
    int loopMS = qRound( 1000.f / (qreal(in_fps) * in_speed));
    m_playbackLoop.setInterval(loopMS);
}

// ======

/** @brief Struct used to keep track of all frame time variance
 * and acommodate for skipped frames. Also tracks whether a frame
 * is still being loaded by the display proxy.
 *
 * Only allocated when playback begins.
 */
struct FrameMeasure {
    FrameMeasure()
        : remainder(0)
        , averageDriverCallbackTime(0)
        , driverCallbackCalls(0)
        , waitingForFrame(false) {
        timeSinceLastFrame.start();
    }

    QElapsedTimer timeSinceLastFrame;
    int remainder;
    qreal averageDriverCallbackTime;
    int driverCallbackCalls;
    bool waitingForFrame;
};


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
    QScopedPointer< FrameMeasure > measure;

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

    KIS_ASSERT(m_d->measure);

    // If we're waiting for each frame, then we delay our callback.
    if (m_d->measure && m_d->measure->waitingForFrame) {
        // Without drop frames on, we need to factor out time that we're waiting
        // for a frame from our time
        return;
    }

    const int currentFrame = displayProxy->activeFrame();
    const int startFrame = animInterface->activePlaybackRange().start();
    const int endFrame = animInterface->activePlaybackRange().end();

    const int timeSinceLastFrame =  m_d->measure->timeSinceLastFrame.elapsed();
    m_d->measure->timeSinceLastFrame.restart();
    const int timePerFrame = qRound(1000.0 / qreal(activeFramesPerSecond().get_value_or(24)) / m_d->driver->speed());

    // Drop frames logic...
    int extraFrames = 0;
    if (m_d->driver->dropFrames()) {
        KIS_ASSERT(m_d->measure);

        int offset = timeSinceLastFrame - timePerFrame;
        m_d->measure->remainder += offset;
        extraFrames = (m_d->measure->remainder) / timePerFrame;
        m_d->measure->remainder = m_d->measure->remainder % timePerFrame;
    }


    // We need to update the timing interval of the internal loop to be as accurate as possible...
    // This is most important for drop frames mode, where frame timing should be as consistent as possible.
    // So...
    //      - Calculate our running average of msec-per-frame
    //      - Based on the error, we adjust our loop timer to offset toward a target time per frame
    //      - We clip this value to a minimum and maximum possible time per frame -- to prevent locking the system.
    // The only exception is the first driver callback -- here we'll just initialize our average and
    // continue.
    // Also another consideration is whether this needs to happen at all for non-dropframes mode or if the logic should
    // simply be different.
    if (m_d->measure->averageDriverCallbackTime == 0) {
        m_d->measure->averageDriverCallbackTime = timeSinceLastFrame;
    } else {
        static const uint8_t SLOPE_CONSTANT = 5; // Used to control how radically we will change timer duration based on latency.
        m_d->measure->averageDriverCallbackTime += qreal(timeSinceLastFrame - m_d->measure->averageDriverCallbackTime) / SLOPE_CONSTANT;
        const int delay = qRound(m_d->measure->averageDriverCallbackTime) - timePerFrame;
        const int newTargetTimerTime = qMax( timePerFrame / 2, timePerFrame - delay );
        m_d->driver->setTimerDuration(newTargetTimerTime);
    }

    // If we have an qtmultimedia playback driver: we will only go to what the audio determines to be the desired frame...
    if (m_d->driver->desiredFrame()) {
        const int desiredFrame = m_d->driver->desiredFrame().get();
        const int targetFrame = frameWrap(desiredFrame, startFrame, endFrame );

        if (currentFrame != targetFrame) {
            displayProxy->displayFrame(targetFrame, false);
        }

        // We've wrapped, let's do whatever correction we can...
        if (targetFrame != desiredFrame) {
            m_d->driver->setFrame(targetFrame);
        }

    } else { // Otherwise, we just advance the frame ourselves based on the displayProxy's active frame.
        int targetFrame = currentFrame + 1 + extraFrames;

        targetFrame = frameWrap(targetFrame, startFrame, endFrame);

        if (currentFrame != targetFrame) {
            // We only wait when drop frames is enabled.
            m_d->measure->waitingForFrame = !m_d->driver->dropFrames();

            bool neededRefresh = displayProxy->displayFrame(targetFrame, false);

            // If we didn't need to refresh, we just continue as usual.
            m_d->measure->waitingForFrame = m_d->measure->waitingForFrame && neededRefresh;
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

        // Disconnect internal..
        m_d->driver.data()->disconnect(this);

        { // Disconnect old Image Anim Interface, prepare for new one..
            auto image = activeCanvas()->image();
            KisImageAnimationInterface* aniInterface = image ? image->animationInterface() : nullptr;
            if (aniInterface) {
                this->disconnect(image->animationInterface());
                image->animationInterface()->disconnect(this);
            }
        }

        { // Disconnect old display proxy, prepare for new one.
            KisFrameDisplayProxy* displayProxy = animationState->displayProxy();

            if (displayProxy) {
                displayProxy->disconnect(this);
            }
        }

        { // Disconnect old animation state, prepare for new one..
            if (animationState) {
                this->disconnect(animationState);
                animationState->disconnect(this);
            }
        }
    }

    StopAndResume stopResume(this);

    KisPlaybackEngine::setCanvas(canvas);

    if (activeCanvas()) {
        KisCanvasAnimationState* animationState = activeCanvas()->animationState();
        KIS_ASSERT(animationState);

        recreateDriver(animationState->mediaInfo());

        KIS_SAFE_ASSERT_RECOVER_RETURN(m_d->driver);

        { // Animation State Connections
            connect(animationState, &KisCanvasAnimationState::sigPlaybackMediaChanged, this, [this]() {
                KisCanvasAnimationState* animationState2 = activeCanvas()->animationState();
                if (animationState2) {
                    recreateDriver(animationState2->mediaInfo());
                }
            });

            connect(animationState, &KisCanvasAnimationState::sigPlaybackStateChanged, this, [this](PlaybackState state){
                if (!m_d->driver)
                    return;

                if (state == PLAYING) {
                    m_d->measure.reset(new FrameMeasure);
                } else {
                    m_d->measure.reset();
                }

                m_d->driver->setPlaybackState(state);
            });
        }

        { // Display proxy connections
            KisFrameDisplayProxy* displayProxy = animationState->displayProxy();
            KIS_ASSERT(displayProxy);
            connect(displayProxy, &KisFrameDisplayProxy::sigFrameDisplayRefreshed, this, [this](){
                if (m_d->measure && m_d->measure->waitingForFrame) {
                    m_d->measure->waitingForFrame = false;
                }
            });

            connect(displayProxy, &KisFrameDisplayProxy::sigFrameRefreshSkipped, this, [this](){
                if (m_d->measure && m_d->measure->waitingForFrame) {
                    m_d->measure->waitingForFrame = false;
                }
            });
        }


        {   // Animation Interface Connections
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

        // Internal connections
        connect(m_d->driver.data(), SIGNAL(throttledShowFrame()), this, SLOT(throttledDriverCallback()));

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
    bool hasQTMultimediaEnabledInConfig = true; // TODO -- Add proper configuration entry w/ default to on. Used for debugging.
    if (file && hasQTMultimediaEnabledInConfig) {
        QScopedPointer<AudioDrivenPlayback> audioPlayback(new AudioDrivenPlayback(this, file.get()));
        if (audioPlayback->deviceAvailability()) {
            m_d->driver.reset(audioPlayback.take());
        }
    }
#endif
}


#include "KisPlaybackEngineQT.moc"
