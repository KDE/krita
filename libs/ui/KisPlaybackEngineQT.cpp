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

#include <QElapsedTimer>
#include <QTimer>
#include "animation/KisFrameDisplayProxy.h"
#include "KisRollingMeanAccumulatorWrapper.h"
#include "KisRollingSumAccumulatorWrapper.h"

#include "KisPlaybackEngineQT.h"


namespace {

/** @brief A simple QTimer-based playback method for situations when audio is not 
 * used (and thus audio-video playback synchronization is not a concern).
 */
class PlaybackDriver : public QObject
{
    Q_OBJECT
public:
    PlaybackDriver(QObject* parent = nullptr);
    ~PlaybackDriver();

    void setPlaybackState(PlaybackState newState);

    void setFramerate(int rate);
    void setSpeed(qreal speed);
    double speed();
    void setDropFrames(bool drop);
    bool dropFrames();

Q_SIGNALS:
    void throttledShowFrame();

private:
    void updatePlaybackLoopInterval(const int& in_fps, const qreal& in_speed);

private:
    QTimer m_playbackLoop;
    double m_speed;
    int m_fps;
    bool m_dropFrames;
};

PlaybackDriver::PlaybackDriver(QObject *parent)
    : QObject(parent)
    , m_speed(1.0)
    , m_fps(24)
    , m_dropFrames(true)
{
    m_playbackLoop.setTimerType(Qt::PreciseTimer);
    connect( &m_playbackLoop, SIGNAL(timeout()), this, SIGNAL(throttledShowFrame()) );
}

PlaybackDriver::~PlaybackDriver()
{
}

void PlaybackDriver::setPlaybackState(PlaybackState newState) {
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

void PlaybackDriver::setFramerate(int rate) {
    KIS_SAFE_ASSERT_RECOVER_RETURN(rate > 0);
    m_fps = rate;
    updatePlaybackLoopInterval(m_fps, m_speed);
}

void PlaybackDriver::setSpeed(qreal speed) {
    KIS_SAFE_ASSERT_RECOVER_RETURN(speed > 0.f);
    m_speed = speed;
    updatePlaybackLoopInterval(m_fps, m_speed);
}

double PlaybackDriver::speed()
{
    return m_speed;
}

void PlaybackDriver::setDropFrames(bool drop) {
    m_dropFrames = drop;
}

bool PlaybackDriver::dropFrames() {
    return m_dropFrames;
}

void PlaybackDriver::updatePlaybackLoopInterval(const int &in_fps, const qreal &in_speed) {
    int loopMS = qRound( 1000.f / (qreal(in_fps) * in_speed));
    m_playbackLoop.setInterval(loopMS);
}

}

// ======

/** @brief Struct used to keep track of all frame time variance
 * and accommodate for skipped frames. Also tracks whether a frame
 * is still being loaded by the display proxy.
 *
 * Only allocated when playback begins.
 */
struct FrameMeasure {
    static constexpr int frameStatsWindow = 50;

    FrameMeasure()
        : averageTimePerFrame(frameStatsWindow)
        , waitingForFrame(false)
        , droppedFramesStat(frameStatsWindow)

    {
        timeSinceLastFrame.start();
    }

    void reset() {
        timeSinceLastFrame.start();
        averageTimePerFrame.reset(frameStatsWindow);
        waitingForFrame = false;
        droppedFramesStat.reset(frameStatsWindow);
    }

    QElapsedTimer timeSinceLastFrame;
    KisRollingMeanAccumulatorWrapper averageTimePerFrame;
    bool waitingForFrame;

    KisRollingSumAccumulatorWrapper droppedFramesStat;
};

// ====== KisPlaybackEngineQT ======

struct KisPlaybackEngineQT::Private {
public:
    Private()
        : driver(new PlaybackDriver())
    {
    }

    ~Private() {
    }

    QScopedPointer<PlaybackDriver> driver;
    FrameMeasure measure;
};

KisPlaybackEngineQT::KisPlaybackEngineQT(QObject *parent)
    : KisPlaybackEngine(parent)
    , m_d(new Private())
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

    KIS_SAFE_ASSERT_RECOVER_RETURN(frameIndex >= 0);

    if (displayProxy->activeFrame() != frameIndex || flags & SEEK_FINALIZE) {
        displayProxy->displayFrame(frameIndex, flags & SEEK_FINALIZE);
    }
}

void KisPlaybackEngineQT::setDropFramesMode(bool value)
{
    KisPlaybackEngine::setDropFramesMode(value);
    KIS_SAFE_ASSERT_RECOVER_RETURN(m_d->driver);
    m_d->driver->setDropFrames(value);
}

boost::optional<int64_t> KisPlaybackEngineQT::activeFramesPerSecond() const
{
    if (activeCanvas()) {
        return activeCanvas()->image()->animationInterface()->framerate();
    } else {
        return boost::none;
    }
}

KisPlaybackEngine::PlaybackStats KisPlaybackEngineQT::playbackStatistics() const
{
    KisPlaybackEngine::PlaybackStats stats;

    if (activeCanvas()->animationState()->playbackState() == PLAYING) {
        const int droppedFrames = m_d->measure.droppedFramesStat.rollingSum();
        const int totalFrames =
            m_d->measure.droppedFramesStat.rollingCount() +
            droppedFrames;

        stats.droppedFramesPortion = qreal(droppedFrames) / totalFrames;
        stats.expectedFps = qreal(activeFramesPerSecond().get_value_or(24)) * m_d->driver->speed();

        const qreal avgTimePerFrame = m_d->measure.averageTimePerFrame.rollingMeanSafe();
        stats.realFps = !qFuzzyIsNull(avgTimePerFrame) ? 1000.0 / avgTimePerFrame : 0.0;
    }

    return stats;
}

void KisPlaybackEngineQT::throttledDriverCallback()
{
    KIS_SAFE_ASSERT_RECOVER_RETURN(m_d->driver);

    KIS_SAFE_ASSERT_RECOVER_RETURN(activeCanvas()->animationState());
    KIS_SAFE_ASSERT_RECOVER_RETURN(activeCanvas()->animationState()->playbackState() == PLAYING);

    KisFrameDisplayProxy* displayProxy = activeCanvas()->animationState()->displayProxy();
    KIS_SAFE_ASSERT_RECOVER_RETURN(displayProxy);

    KIS_SAFE_ASSERT_RECOVER_RETURN(activeCanvas()->image());
    KisImageAnimationInterface *animInterface = activeCanvas()->image()->animationInterface();
    KIS_SAFE_ASSERT_RECOVER_RETURN(animInterface);

    // If we're waiting for each frame, then we delay our callback.
    if (m_d->measure.waitingForFrame) {
        // Without drop frames on, we need to factor out time that we're waiting
        // for a frame from our time
        return;
    }

    const int currentFrame = displayProxy->activeFrame();
    const int startFrame = animInterface->activePlaybackRange().start();
    const int endFrame = animInterface->activePlaybackRange().end();

    const int timeSinceLastFrame =  m_d->measure.timeSinceLastFrame.restart();
    const int timePerFrame = qRound(1000.0 / qreal(activeFramesPerSecond().get_value_or(24)) / m_d->driver->speed());
    m_d->measure.averageTimePerFrame(timeSinceLastFrame);


    // Drop frames logic...
    int extraFrames = 0;
    if (m_d->driver->dropFrames()) {
        const int offset = timeSinceLastFrame - timePerFrame;
        extraFrames = qMax(0, offset) / timePerFrame;
    }

    m_d->measure.droppedFramesStat(extraFrames);

    { // just advance the frame ourselves based on the displayProxy's active frame.
        int targetFrame = currentFrame + 1 + extraFrames;

        targetFrame = frameWrap(targetFrame, startFrame, endFrame);

        if (currentFrame != targetFrame) {
            // We only wait when drop frames is enabled.
            m_d->measure.waitingForFrame = !m_d->driver->dropFrames();

            bool neededRefresh = displayProxy->displayFrame(targetFrame, false);

            // If we didn't need to refresh, we just continue as usual.
            m_d->measure.waitingForFrame &= neededRefresh;
        }
    }
}

void KisPlaybackEngineQT::setCanvas(KoCanvasBase *p_canvas)
{
    KisCanvas2* canvas = dynamic_cast<KisCanvas2*>(p_canvas);

    struct StopAndResume {
        StopAndResume(KisPlaybackEngineQT* p_self)
            : m_self(p_self) {
            KIS_SAFE_ASSERT_RECOVER_RETURN(m_self->m_d->driver);

            m_self->m_d->driver->setPlaybackState(PlaybackState::STOPPED);
        }

        ~StopAndResume() {
            KIS_SAFE_ASSERT_RECOVER_RETURN(m_self->m_d->driver);

            if (m_self->activeCanvas()) {
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

        KIS_SAFE_ASSERT_RECOVER_RETURN(m_d->driver);

        { // Animation State Connections
            connect(animationState, &KisCanvasAnimationState::sigPlaybackStateChanged, this, [this](PlaybackState state){
                KIS_SAFE_ASSERT_RECOVER_RETURN(m_d->driver);

                if (state == PLAYING) {
                    m_d->measure.reset();
                }

                m_d->driver->setPlaybackState(state);
            });

            connect(animationState, &KisCanvasAnimationState::sigPlaybackSpeedChanged, this, [this](qreal value){
                KIS_SAFE_ASSERT_RECOVER_RETURN(m_d->driver);
                m_d->driver->setSpeed(value);
            });
            m_d->driver->setSpeed(animationState->playbackSpeed());
        }

        { // Display proxy connections
            KisFrameDisplayProxy* displayProxy = animationState->displayProxy();
            KIS_ASSERT(displayProxy);
            connect(displayProxy, &KisFrameDisplayProxy::sigFrameDisplayRefreshed, this, [this](){
                m_d->measure.waitingForFrame = false;
            });

            connect(displayProxy, &KisFrameDisplayProxy::sigFrameRefreshSkipped, this, [this](){
                m_d->measure.waitingForFrame = false;
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
}

void KisPlaybackEngineQT::unsetCanvas()
{
    setCanvas(nullptr);
}

#include "KisPlaybackEngineQT.moc"
