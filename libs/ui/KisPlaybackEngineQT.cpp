#include "KisPlaybackEngineQT.h"

#include "kis_debug.h"
#include "kis_canvas2.h"
#include "kis_canvas_animation_state.h"
#include "kis_image.h"
#include "kis_image_animation_interface.h"

#include <QTimer>
#include "animation/KisFrameDisplayProxy.h"

struct KisPlaybackEngineQT::Private {
public:
    Private(KisPlaybackEngineQT* p_self)
        : self(p_self) {
        setFPS(24);
        setNormalizedSpeed(1.0f);
    }

    ~Private() {
    }

    const QTimer* getPlaybackLoop() const {
        return &m_playbackLoop;
    }

    void respondToStateChange( PlaybackState newState ) {
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

    void setFPS(const int& p_fpsValue) {
        KIS_SAFE_ASSERT_RECOVER_RETURN(p_fpsValue > 0);
        m_fps = p_fpsValue;
        updatePlaybackLoopInterval(m_fps, m_speed);
    }

    void setNormalizedSpeed(const qreal& p_speed) {
        KIS_SAFE_ASSERT_RECOVER_RETURN(p_speed > 0.f);
        m_speed = p_speed;
        updatePlaybackLoopInterval(m_fps, m_speed);
    }

private:
    void updatePlaybackLoopInterval(const int& in_fps, const qreal& in_speed) {
        int loopMS = qRound( 1000.f / (qreal(in_fps) * in_speed));
        m_playbackLoop.setInterval(loopMS);
    }

private:
    QTimer m_playbackLoop;
    int m_fps;
    qreal m_speed;

    KisPlaybackEngineQT* self;

};

KisPlaybackEngineQT::KisPlaybackEngineQT(QObject *parent)
    : KisPlaybackEngine(parent)
    , m_d(new Private(this))
{
    connect(m_d->getPlaybackLoop(), SIGNAL(timeout()), this, SLOT(throttledQtFrameTimeExpired()));
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
    m_d->setNormalizedSpeed(value);
}


void KisPlaybackEngineQT::throttledQtFrameTimeExpired()
{
    if (!activeCanvas())
        return;

    KIS_SAFE_ASSERT_RECOVER_RETURN(activeCanvas()->animationState());
    KisFrameDisplayProxy* displayProxy = activeCanvas()->animationState()->displayProxy();
    KIS_SAFE_ASSERT_RECOVER_RETURN(displayProxy);

    KIS_SAFE_ASSERT_RECOVER_RETURN(activeCanvas()->image());
    KisImageAnimationInterface *animInterface = activeCanvas()->image()->animationInterface();
    KIS_SAFE_ASSERT_RECOVER_RETURN(animInterface);

    const int currentFrame = displayProxy->activeFrame();
    int targetFrame = currentFrame + 1;

    const int startFrame = animInterface->activePlaybackRange().start();
    const int endFrame = animInterface->activePlaybackRange().end();

    targetFrame = frameWrap(targetFrame, startFrame, endFrame);

    if (displayProxy->activeFrame() != targetFrame) {
        displayProxy->displayFrame(targetFrame, false);
    }

}

void KisPlaybackEngineQT::setCanvas(KoCanvasBase *p_canvas)
{
    KisCanvas2* canvas = dynamic_cast<KisCanvas2*>(p_canvas);

    if (activeCanvas() == canvas) {
        return;
    }

    if (activeCanvas()) {
        KisCanvasAnimationState* animationState = activeCanvas()->animationState();

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

    KisPlaybackEngine::setCanvas(canvas);

    if (activeCanvas()) {
        KisCanvasAnimationState* animationState = activeCanvas()->animationState();
        KIS_ASSERT(animationState);

        connect(animationState, &KisCanvasAnimationState::sigPlaybackStateChanged, this, [this](PlaybackState state){
            m_d->respondToStateChange(state);
        });

        auto image = activeCanvas()->image();
        KIS_ASSERT(image);

        m_d->setFPS(image->animationInterface()->framerate());
    }
}

void KisPlaybackEngineQT::unsetCanvas()
{
    setCanvas(nullptr);
}
