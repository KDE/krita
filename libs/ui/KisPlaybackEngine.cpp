#include "KisPlaybackEngine.h"

#include <QMap>

#include "kis_canvas2.h"
#include "animation/KisPlaybackHandle.h"
#include "kis_signal_compressor_with_param.h"
#include "kis_canvas_animation_state.h"
#include "kis_image_animation_interface.h"
#include "animation/KisFrameDisplayProxy.h"
#include "KisViewManager.h"
#include "kis_raster_keyframe_channel.h"
#include "kis_onion_skin_compositor.h"

#include <mlt++/Mlt.h>
#include <mlt++/MltConsumer.h>
#include <mlt++/MltFrame.h>

#include "kis_debug.h"

const float SCRUB_AUDIO_SECONDS = 0.25f;

/**
 *  This responds to MLT consumer requests for frames. This may
 *  continue to be called even when playback is stopped due to it running
 *  simultaneously in a separate thread.
 */
static void mltOnConsumerFrameShow(mlt_consumer c, void* p_self, mlt_frame p_frame) {
    KisPlaybackEngine* self = static_cast<KisPlaybackEngine*>(p_self);
    Mlt::Frame frame(p_frame);
    Mlt::Consumer consumer(c);
    const int position = frame.get_position();
    self->sigChangeActiveCanvasFrame(position);
}

struct KisPlaybackEngine::Private {

    Private( KisPlaybackEngine* p_self )
        : m_self(p_self)
        , activeCanvas(nullptr)
    {
        // Initialize Audio Libraries
        Mlt::Factory::init();

        profile.reset(new Mlt::Profile());
        profile->set_frame_rate(24, 1);

        std::function<void (int)> callback(std::bind(&Private::pushAudio, this, std::placeholders::_1));
        sigPushAudioCompressor.reset(
                    new KisSignalCompressorWithParam<int>(1000 * SCRUB_AUDIO_SECONDS, callback, KisSignalCompressor::FIRST_ACTIVE_POSTPONE_NEXT)
                    );

        initializeConsumers();
    }

    ~Private() {
        cleanupConsumers();
        Mlt::Factory::close();
    }

    void pushAudio(int frame) {
        if (pushConsumer->is_stopped())
            return;

        KIS_ASSERT(pullConsumer->is_stopped());

        QSharedPointer<Mlt::Producer> activeProducer = canvasProducers[activeCanvas];
        if (activePlaybackMode() == PLAYBACK_PUSH && activeProducer) {
            const int SCRUB_AUDIO_WINDOW = profile->frame_rate_num() * SCRUB_AUDIO_SECONDS;
            for (int i = 0; i < SCRUB_AUDIO_WINDOW; i++ ) {
                Mlt::Frame* f = activeProducer->get_frame(frame + i );
                pushConsumer->push(f);
            }

            // It turns out that get_frame actually seeks to the frame too,
            // Not having this last seek will cause unexpected "jumps" at
            // the beginning of playback...
            activeProducer->seek(frame);
        }
    }

    void initializeConsumers() {
        pushConsumer.reset(new Mlt::PushConsumer(*profile, "sdl2_audio"));
        pullConsumer.reset(new Mlt::Consumer(*profile, "sdl2_audio"));
        pullConsumer->listen("consumer-frame-show", m_self, (mlt_listener)mltOnConsumerFrameShow);
    }

    void cleanupConsumers() {
        if (pullConsumer && !pullConsumer->is_stopped()) {
            pullConsumer->stop();
        }

        if (pushConsumer && !pushConsumer->is_stopped()) {
            pushConsumer->stop();
        }

        pullConsumer.reset();
        pushConsumer.reset();
    }

    KisCanvasAnimationState* activeCanvasAnimationPlayer() {
        KIS_ASSERT_RECOVER_RETURN_VALUE(activeCanvas, nullptr);
        return activeCanvas->animationState();
    }

    PlaybackMode activePlaybackMode() {
        KIS_ASSERT_RECOVER_RETURN_VALUE(activeCanvas, PLAYBACK_PUSH);
        return activeCanvasAnimationPlayer()->playbackState() == PlaybackState::PLAYING ? PLAYBACK_PULL : PLAYBACK_PUSH;
    }

    QSharedPointer<Mlt::Producer> activeProducer() {
        KIS_ASSERT_RECOVER_RETURN_VALUE(activeCanvas, nullptr);
        KIS_ASSERT_RECOVER_RETURN_VALUE(canvasProducers.contains(activeCanvas), nullptr);
        return canvasProducers[activeCanvas];
    }

private:
    KisPlaybackEngine* m_self; //temp, we need a back pointer for callback binding. Maybe we can just use the private class itself instead?

public:
    QScopedPointer<Mlt::Profile> profile;

    //MLT PUSH CONSUMER
    QScopedPointer<Mlt::Consumer> pullConsumer;
    //MLT PULL CONSUMER
    QScopedPointer<Mlt::PushConsumer> pushConsumer;

    // Currently active canvas
    KisCanvas2* activeCanvas;

    //Filters...
    //QSharedPointer<Mlt::Filter> loopFilter;

    // Map of handles to Mlt producers..
    QMap<KisCanvas2*, QSharedPointer<Mlt::Producer>> canvasProducers;

    QScopedPointer<KisSignalCompressorWithParam<int>> sigPushAudioCompressor;
};

/**
 * @brief The StopAndResumeConsumer struct is used to encapsulate optional
 * stop-and-then-resume behavior of a consumer. Using RAII, we can stop
 * a consumer at construction and simply resume it when it exits scope.
 */
struct KisPlaybackEngine::StopAndResume {
public:
    explicit StopAndResume(KisPlaybackEngine::Private* p_d, bool requireFullRestart = false)
        : m_d(p_d)
    {
        KIS_ASSERT(p_d);

        m_d->pushConsumer->stop();
        m_d->pushConsumer->purge();
        m_d->pullConsumer->stop();
        m_d->pullConsumer->purge();
        m_d->pullConsumer->disconnect_all_producers();

        if (requireFullRestart) {
            m_d->cleanupConsumers();
        }

        if (m_d->activeCanvas) {
            m_d->activeProducer()->seek(m_d->activeCanvasAnimationPlayer()->displayProxy()->visibleFrame());
        }
    }

    ~StopAndResume() {
        KIS_ASSERT(m_d);
        if (!m_d->pushConsumer || !m_d->pullConsumer) {
            m_d->initializeConsumers();
        }

        if (m_d->activePlaybackMode() == PLAYBACK_PUSH) {
            m_d->pushConsumer->start();
        } else {
            m_d->pullConsumer->connect_producer(*m_d->canvasProducers[m_d->activeCanvas]);
            m_d->pullConsumer->start();
        }

        if (m_d->activeCanvas && m_d->canvasProducers.contains(m_d->activeCanvas)) {
            if ( m_d->activeCanvasAnimationPlayer()->displayProxy()->visibleFrame() > 0) {
                m_d->canvasProducers[m_d->activeCanvas]->seek(m_d->activeCanvasAnimationPlayer()->displayProxy()->visibleFrame());
                KIS_ASSERT(m_d->canvasProducers[m_d->activeCanvas]->frame() == m_d->activeCanvasAnimationPlayer()->displayProxy()->visibleFrame());
            }
        }

    }

private:
    Private* m_d;
};

KisPlaybackEngine::KisPlaybackEngine(QObject *parent)
    : QObject(parent)
    , m_d( new Private(this))
{
    connect(this, &KisPlaybackEngine::sigChangeActiveCanvasFrame, this, &KisPlaybackEngine::throttledShowFrame, Qt::UniqueConnection);
}

KisPlaybackEngine::~KisPlaybackEngine()
{
}

void KisPlaybackEngine::play()
{
    m_d->activeCanvasAnimationPlayer()->setPlaybackState(PLAYING);
}

void KisPlaybackEngine::pause()
{
    m_d->activeCanvasAnimationPlayer()->setPlaybackState(PAUSED);
}

void KisPlaybackEngine::playPause()
{
    if (!m_d->activeCanvasAnimationPlayer()) {
        return;
    }

    if (m_d->activeCanvasAnimationPlayer()->playbackState() == PLAYING) {
        pause();
    } else {
        play();
    }
}

void KisPlaybackEngine::stop()
{
    if (m_d->activeCanvasAnimationPlayer()->playbackState() != STOPPED) {
        const boost::optional<int> origin = m_d->activeCanvasAnimationPlayer()->playbackOrigin();
        m_d->activeCanvasAnimationPlayer()->setPlaybackState(STOPPED);
        if (origin.has_value()) {
            seek(origin.value());
        }
    } else if (m_d->activeCanvasAnimationPlayer()->displayProxy()->frame() != 0) {
        seek(0);
    }
}

void KisPlaybackEngine::seek(int frameIndex, SeekFlags flags)
{
    if (!m_d->activeCanvas || !m_d->activeCanvasAnimationPlayer()) return;

    if (m_d->activePlaybackMode() == PLAYBACK_PUSH) {
        m_d->canvasProducers[m_d->activeCanvas]->seek(frameIndex);

        if (flags & SEEK_PUSH_AUDIO) {
            m_d->sigPushAudioCompressor->start(frameIndex);
        }

        m_d->activeCanvasAnimationPlayer()->showFrame(frameIndex);
    } else {
        StopAndResume(m_d.data());
        m_d->canvasProducers[m_d->activeCanvas]->seek(frameIndex);
    }
}

void KisPlaybackEngine::previousFrame()
{
    if (!m_d->activeCanvas || !m_d->activeCanvasAnimationPlayer()) return;

    KisImageAnimationInterface *animInterface = m_d->activeCanvas->image()->animationInterface();

    const int startFrame = animInterface->playbackRange().start();
    const int endFrame = animInterface->playbackRange().end();

    int frame = m_d->activeCanvasAnimationPlayer()->displayProxy()->visibleFrame() - 1;

    if (frame < startFrame || frame >  endFrame) {
        frame = endFrame;
    }

    if (frame >= 0) {
        if (m_d->activeCanvasAnimationPlayer()->playbackState() != STOPPED) {
            stop();
        }
        seek(frame);
    }
}

void KisPlaybackEngine::nextFrame()
{
    if (!m_d->activeCanvas || !m_d->activeCanvasAnimationPlayer()) return;
    KisImageAnimationInterface *animInterface = m_d->activeCanvas->image()->animationInterface();

    const int startFrame = animInterface->playbackRange().start();
    const int endFrame = animInterface->playbackRange().end();

    int frame = m_d->activeCanvasAnimationPlayer()->displayProxy()->visibleFrame() + 1;

    if (frame > endFrame || frame < startFrame ) {
        frame = startFrame;
    }

    if (frame >= 0) {
        if (m_d->activeCanvasAnimationPlayer()->playbackState() != STOPPED) {
            stop();
        }

        seek(frame);
    }
}

void KisPlaybackEngine::previousKeyframe()
{
    if (!m_d->activeCanvas || !m_d->activeCanvasAnimationPlayer()) return;

    KisNodeSP node = m_d->activeCanvas->viewManager()->activeNode();
    if (!node) return;

    KisKeyframeChannel *keyframes =
        node->getKeyframeChannel(KisKeyframeChannel::Raster.id());
    if (!keyframes) return;

    int currentFrame = m_d->activeCanvasAnimationPlayer()->displayProxy()->visibleFrame();

    int destinationTime = -1;
    if (!keyframes->keyframeAt(currentFrame)) {
        destinationTime = keyframes->activeKeyframeTime(currentFrame);
    } else {
        destinationTime = keyframes->previousKeyframeTime(currentFrame);
    }

    if (keyframes->keyframeAt(destinationTime)) {
        if (m_d->activeCanvasAnimationPlayer()->playbackState() != STOPPED) {
            stop();
        }

        seek(destinationTime);
    }
}

void KisPlaybackEngine::nextKeyframe()
{
    if (!m_d->activeCanvas || !m_d->activeCanvasAnimationPlayer()) return;

    KisNodeSP node = m_d->activeCanvas->viewManager()->activeNode();
    if (!node) return;

    KisKeyframeChannel *keyframes =
        node->getKeyframeChannel(KisKeyframeChannel::Raster.id());
    if (!keyframes) return;

    int currentTime = m_d->activeCanvasAnimationPlayer()->displayProxy()->visibleFrame();

    int destinationTime = -1;
    if (keyframes->activeKeyframeAt(currentTime)) {
        destinationTime = keyframes->nextKeyframeTime(currentTime);
    }

    if (keyframes->keyframeAt(destinationTime)) {
        // Jump to next key...
        if (m_d->activeCanvasAnimationPlayer()->playbackState() != STOPPED) {
            stop();
        }

        seek(destinationTime);
    } else {
        // Jump ahead by estimated timing...
        const int activeKeyTime = keyframes->activeKeyframeTime(currentTime);
        const int previousKeyTime = keyframes->previousKeyframeTime(activeKeyTime);

        if (previousKeyTime != -1) {
            if (m_d->activeCanvasAnimationPlayer()->playbackState() != STOPPED) {
                stop();
            }

            const int timing = activeKeyTime - previousKeyTime;
            seek(currentTime + timing);
        }
    }
}

void KisPlaybackEngine::previousMatchingKeyframe()
{
    if (!m_d->activeCanvas || !m_d->activeCanvasAnimationPlayer()) return;

    KisNodeSP node = m_d->activeCanvas->viewManager()->activeNode();
    if (!node) return;

    KisKeyframeChannel *keyframes =
        node->getKeyframeChannel(KisKeyframeChannel::Raster.id());
    if (!keyframes) return;

    int time = m_d->activeCanvasAnimationPlayer()->displayProxy()->visibleFrame();

    KisKeyframeSP currentKeyframe = keyframes->keyframeAt(time);
    int destinationTime = keyframes->activeKeyframeTime(time);
    const int desiredColor = currentKeyframe ? currentKeyframe->colorLabel() : keyframes->keyframeAt(destinationTime)->colorLabel();
    previousKeyframeWithColor(desiredColor);
}

void KisPlaybackEngine::nextMatchingKeyframe()
{
    if (!m_d->activeCanvas || !m_d->activeCanvasAnimationPlayer()) return;

    KisNodeSP node = m_d->activeCanvas->viewManager()->activeNode();
    if (!node) return;

    KisKeyframeChannel *keyframes =
        node->getKeyframeChannel(KisKeyframeChannel::Raster.id());
    if (!keyframes) return;

    int time = m_d->activeCanvasAnimationPlayer()->displayProxy()->visibleFrame();

    if (!keyframes->activeKeyframeAt(time)) {
        return;
    }

    int destinationTime = keyframes->activeKeyframeTime(time);
    nextKeyframeWithColor(keyframes->keyframeAt(destinationTime)->colorLabel());
}

void KisPlaybackEngine::previousUnfilteredKeyframe()
{
    previousKeyframeWithColor(KisOnionSkinCompositor::instance()->colorLabelFilter());
}

void KisPlaybackEngine::nextUnfilteredKeyframe()
{
    nextKeyframeWithColor(KisOnionSkinCompositor::instance()->colorLabelFilter());
}

void KisPlaybackEngine::nextKeyframeWithColor(int color)
{
    QSet<int> validColors;
    validColors.insert(color);
    nextKeyframeWithColor(validColors);
}

void KisPlaybackEngine::nextKeyframeWithColor(const QSet<int> &validColors)
{
    if (!m_d->activeCanvas || !m_d->activeCanvasAnimationPlayer()) return;

    KisNodeSP node = m_d->activeCanvas->viewManager()->activeNode();
    if (!node) return;

    KisKeyframeChannel *keyframes =
        node->getKeyframeChannel(KisKeyframeChannel::Raster.id());
    if (!keyframes) return;

    int time = m_d->activeCanvasAnimationPlayer()->displayProxy()->visibleFrame();

    if (!keyframes->activeKeyframeAt(time)) {
        return;
    }

    int destinationTime = keyframes->activeKeyframeTime(time);
    while ( keyframes->keyframeAt(destinationTime) &&
            ((destinationTime == time) ||
            !validColors.contains(keyframes->keyframeAt(destinationTime)->colorLabel()))) {

        destinationTime = keyframes->nextKeyframeTime(destinationTime);
    }

    if (keyframes->keyframeAt(destinationTime)) {
        if (m_d->activeCanvasAnimationPlayer()->playbackState() != STOPPED) {
            stop();
        }

        seek(destinationTime);
    }
}

void KisPlaybackEngine::previousKeyframeWithColor(int color)
{
    QSet<int> validColors;
    validColors.insert(color);
    previousKeyframeWithColor(validColors);
}

void KisPlaybackEngine::previousKeyframeWithColor(const QSet<int> &validColors)
{
    if (!m_d->activeCanvas || !m_d->activeCanvasAnimationPlayer()) return;

    KisNodeSP node = m_d->activeCanvas->viewManager()->activeNode();
    if (!node) return;

    KisKeyframeChannel *keyframes =
        node->getKeyframeChannel(KisKeyframeChannel::Raster.id());
    if (!keyframes) return;

    int time = m_d->activeCanvasAnimationPlayer()->displayProxy()->visibleFrame();

    int destinationTime = keyframes->activeKeyframeTime(time);
    while (keyframes->keyframeAt(destinationTime) &&
           ((destinationTime == time) ||
           !validColors.contains(keyframes->keyframeAt(destinationTime)->colorLabel()))) {

        destinationTime = keyframes->previousKeyframeTime(destinationTime);
    }

    if (keyframes->keyframeAt(destinationTime)) {
        if (m_d->activeCanvasAnimationPlayer()->playbackState() != STOPPED) {
            stop();
        }

        seek(destinationTime);
    }
}

void KisPlaybackEngine::setupProducerFromFile(QFileInfo file)
{
    QSharedPointer<Mlt::Producer> producer( new Mlt::Producer(*m_d->profile, file.absoluteFilePath().toUtf8().data()));
    m_d->canvasProducers[m_d->activeCanvas] = producer;
}

void KisPlaybackEngine::setCanvas(KoCanvasBase *canvas)
{
    KisCanvas2* canvas2 = dynamic_cast<KisCanvas2*>(canvas);

    if (!canvas2 || m_d->activeCanvas == canvas2) {
        return;
    }

    // Disconnect player, prepare for new active player..
    if (m_d->activeCanvas && m_d->activeCanvasAnimationPlayer()) {
        this->disconnect(m_d->activeCanvasAnimationPlayer());
        m_d->activeCanvasAnimationPlayer()->disconnect(this);
    }

    StopAndResume sr(m_d.data(), true);

    m_d->activeCanvas = canvas2;

    if (m_d->activeCanvasAnimationPlayer()) {
        connect(m_d->activeCanvasAnimationPlayer(), &KisCanvasAnimationState::sigPlaybackStateChanged, this, [this](PlaybackState state){
            QSharedPointer<Mlt::Producer> activeProducer = m_d->canvasProducers[m_d->activeCanvas];

            StopAndResume stopResume(m_d.data());
        });

        connect(m_d->activeCanvasAnimationPlayer(), &KisCanvasAnimationState::sigPlaybackMediaChanged, this, &KisPlaybackEngine::setupProducerFromFile);

        m_d->profile->set_frame_rate(m_d->activeCanvas->image()->animationInterface()->framerate(), 1);

        if (m_d->activeCanvasAnimationPlayer()->mediaInfo().has_value()) {
            setupProducerFromFile(m_d->activeCanvasAnimationPlayer()->mediaInfo().value());
        } else {
            m_d->canvasProducers[m_d->activeCanvas] = QSharedPointer<Mlt::Producer>(new Mlt::Producer(*m_d->profile, "count"));
        }
    }
}

void KisPlaybackEngine::unsetCanvas() {
    // TODO: What do we **NEED** to do here?
}

void KisPlaybackEngine::throttledShowFrame(const int frame)
{
    if (m_d->activeCanvas && m_d->activePlaybackMode() == PLAYBACK_PULL) {
        m_d->activeCanvasAnimationPlayer()->showFrame(frame);
    }
}

void KisPlaybackEngine::setPlaybackSpeedPercent(int value)
{
    Q_UNIMPLEMENTED();
}

void KisPlaybackEngine::setPlaybackSpeedNormalized(double value)
{
    Q_UNIMPLEMENTED();
}

