#include "KisPlaybackEngine.h"

#include <QMap>

#include "kis_canvas2.h"
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
#include <mlt++/MltFilter.h>
#include <mlt-7/framework/mlt_service.h>

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
        , mute(false)
    {
        // Initialize Audio Libraries
        Mlt::Factory::init();

        profile.reset(new Mlt::Profile());
        profile->set_frame_rate(24, 1);


        std::function<void (int)> callback(std::bind(&Private::pushAudio, this, std::placeholders::_1));
        sigPushAudioCompressor.reset(
                    new KisSignalCompressorWithParam<int>(1000 * SCRUB_AUDIO_SECONDS, callback, KisSignalCompressor::FIRST_ACTIVE)
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

    KisCanvasAnimationState* activeCanvasAnimationState() {
        KIS_ASSERT_RECOVER_RETURN_VALUE(activeCanvas, nullptr);
        return activeCanvas->animationState();
    }

    PlaybackMode activePlaybackMode() {
        KIS_ASSERT_RECOVER_RETURN_VALUE(activeCanvas, PLAYBACK_PUSH);
        return activeCanvasAnimationState()->playbackState() == PlaybackState::PLAYING ? PLAYBACK_PULL : PLAYBACK_PUSH;
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

    // Map of handles to Mlt producers..
    QMap<KisCanvas2*, QSharedPointer<Mlt::Producer>> canvasProducers;

    QScopedPointer<KisSignalCompressorWithParam<int>> sigPushAudioCompressor;

    bool mute;
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
            m_d->activeProducer()->seek(m_d->activeCanvasAnimationState()->displayProxy()->activeFrame());
        }
    }

    ~StopAndResume() {
        KIS_ASSERT(m_d);
        if (!m_d->pushConsumer || !m_d->pullConsumer) {
            m_d->initializeConsumers();
        }

        if (m_d->activeCanvas) {
            if (m_d->activePlaybackMode() == PLAYBACK_PUSH) {
                m_d->pushConsumer->set("volume", m_d->mute ? 0.0 : m_d->activeCanvasAnimationState()->currentVolume());
                m_d->pushConsumer->start();
            } else {
                m_d->pullConsumer->connect_producer(*m_d->activeProducer());
                m_d->pullConsumer->set("volume", m_d->mute ? 0.0 : m_d->activeCanvasAnimationState()->currentVolume());
                m_d->pullConsumer->start();
            }

            {
                KisImageAnimationInterface* animInterface = m_d->activeCanvas->image()->animationInterface();
                m_d->activeProducer()->set("start_frame", animInterface->activePlaybackRange().start());
                m_d->activeProducer()->set("end_frame", animInterface->activePlaybackRange().end());
                const int shouldLimit = m_d->activePlaybackMode() == PLAYBACK_PUSH ? 0 : 1;
                m_d->activeProducer()->set("limit_enabled", shouldLimit);
            }

            if (m_d->canvasProducers.contains(m_d->activeCanvas)) {
                if ( m_d->activeCanvasAnimationState()->displayProxy()->activeFrame() >= 0) {
                    m_d->canvasProducers[m_d->activeCanvas]->seek(m_d->activeCanvasAnimationState()->displayProxy()->activeFrame());
                }
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
    m_d->activeCanvasAnimationState()->setPlaybackState(PLAYING);
}

void KisPlaybackEngine::pause()
{
    m_d->activeCanvasAnimationState()->setPlaybackState(PAUSED);
}

void KisPlaybackEngine::playPause()
{
    if (!m_d->activeCanvasAnimationState()) {
        return;
    }

    if (m_d->activeCanvasAnimationState()->playbackState() == PLAYING) {
        pause();
        seek(m_d->activeCanvasAnimationState()->displayProxy()->activeFrame(), SEEK_FINALIZE);
    } else {
        play();
    }
}

void KisPlaybackEngine::stop()
{
    if (m_d->activeCanvasAnimationState()->playbackState() != STOPPED) {
        const boost::optional<int> origin = m_d->activeCanvasAnimationState()->playbackOrigin();
        m_d->activeCanvasAnimationState()->setPlaybackState(STOPPED);
        if (origin.has_value()) {
            seek(origin.value(), SEEK_FINALIZE);
        }
    } else if (m_d->activeCanvasAnimationState()->displayProxy()->activeFrame() != 0) {
        const int firstFrame = m_d->activeCanvas->image()->animationInterface()->documentPlaybackRange().start();
        seek(firstFrame, SEEK_FINALIZE | SEEK_PUSH_AUDIO);
    }
}

void KisPlaybackEngine::seek(int frameIndex, SeekFlags flags)
{
    if (!m_d->activeCanvas || !m_d->activeCanvasAnimationState()) return;

    if (m_d->activePlaybackMode() == PLAYBACK_PUSH) {
        m_d->canvasProducers[m_d->activeCanvas]->seek(frameIndex);

        if (flags & SEEK_PUSH_AUDIO) {
            m_d->sigPushAudioCompressor->start(frameIndex);
        }

        m_d->activeCanvasAnimationState()->showFrame(frameIndex, (flags & SEEK_FINALIZE) > 0);
    }
}

void KisPlaybackEngine::previousFrame()
{
    if (!m_d->activeCanvas || !m_d->activeCanvasAnimationState()) return;

    KisImageAnimationInterface *animInterface = m_d->activeCanvas->image()->animationInterface();

    const int startFrame = animInterface->activePlaybackRange().start();
    const int endFrame = animInterface->activePlaybackRange().end();

    int frame = m_d->activeCanvasAnimationState()->displayProxy()->activeFrame() - 1;

    if (frame < startFrame || frame >  endFrame) {
        frame = endFrame;
    }

    if (frame >= 0) {
        if (m_d->activeCanvasAnimationState()->playbackState() != STOPPED) {
            stop();
        }

        seek(frame, SEEK_FINALIZE | SEEK_PUSH_AUDIO);
    }
}

void KisPlaybackEngine::nextFrame()
{
    if (!m_d->activeCanvas || !m_d->activeCanvasAnimationState()) return;
    KisImageAnimationInterface *animInterface = m_d->activeCanvas->image()->animationInterface();

    const int startFrame = animInterface->activePlaybackRange().start();
    const int endFrame = animInterface->activePlaybackRange().end();

    int frame = m_d->activeCanvasAnimationState()->displayProxy()->activeFrame() + 1;

    if (frame > endFrame || frame < startFrame ) {
        frame = startFrame;
    }

    if (frame >= 0) {
        if (m_d->activeCanvasAnimationState()->playbackState() != STOPPED) {
            stop();
        }

        seek(frame, SEEK_FINALIZE | SEEK_PUSH_AUDIO);
    }
}

void KisPlaybackEngine::previousKeyframe()
{
    if (!m_d->activeCanvas || !m_d->activeCanvasAnimationState()) return;

    KisNodeSP node = m_d->activeCanvas->viewManager()->activeNode();
    if (!node) return;

    KisKeyframeChannel *keyframes =
        node->getKeyframeChannel(KisKeyframeChannel::Raster.id());
    if (!keyframes) return;

    int currentFrame = m_d->activeCanvasAnimationState()->displayProxy()->activeFrame();

    int destinationTime = -1;
    if (!keyframes->keyframeAt(currentFrame)) {
        destinationTime = keyframes->activeKeyframeTime(currentFrame);
    } else {
        destinationTime = keyframes->previousKeyframeTime(currentFrame);
    }

    if (keyframes->keyframeAt(destinationTime)) {
        if (m_d->activeCanvasAnimationState()->playbackState() != STOPPED) {
            stop();
        }

        seek(destinationTime, SEEK_FINALIZE | SEEK_PUSH_AUDIO);
    }
}

void KisPlaybackEngine::nextKeyframe()
{
    if (!m_d->activeCanvas || !m_d->activeCanvasAnimationState()) return;

    KisNodeSP node = m_d->activeCanvas->viewManager()->activeNode();
    if (!node) return;

    KisKeyframeChannel *keyframes =
        node->getKeyframeChannel(KisKeyframeChannel::Raster.id());
    if (!keyframes) return;

    int currentTime = m_d->activeCanvasAnimationState()->displayProxy()->activeFrame();

    int destinationTime = -1;
    if (keyframes->activeKeyframeAt(currentTime)) {
        destinationTime = keyframes->nextKeyframeTime(currentTime);
    }

    if (keyframes->keyframeAt(destinationTime)) {
        // Jump to next key...
        if (m_d->activeCanvasAnimationState()->playbackState() != STOPPED) {
            stop();
        }

        seek(destinationTime, SEEK_FINALIZE | SEEK_PUSH_AUDIO);
    } else {
        // Jump ahead by estimated timing...
        const int activeKeyTime = keyframes->activeKeyframeTime(currentTime);
        const int previousKeyTime = keyframes->previousKeyframeTime(activeKeyTime);

        if (previousKeyTime != -1) {
            if (m_d->activeCanvasAnimationState()->playbackState() != STOPPED) {
                stop();
            }

            const int timing = activeKeyTime - previousKeyTime;
            seek(currentTime + timing, SEEK_FINALIZE | SEEK_PUSH_AUDIO);
        }
    }
}

void KisPlaybackEngine::previousMatchingKeyframe()
{
    if (!m_d->activeCanvas || !m_d->activeCanvasAnimationState()) return;

    KisNodeSP node = m_d->activeCanvas->viewManager()->activeNode();
    if (!node) return;

    KisKeyframeChannel *keyframes =
        node->getKeyframeChannel(KisKeyframeChannel::Raster.id());
    if (!keyframes) return;

    int time = m_d->activeCanvasAnimationState()->displayProxy()->activeFrame();

    KisKeyframeSP currentKeyframe = keyframes->keyframeAt(time);
    int destinationTime = keyframes->activeKeyframeTime(time);
    const int desiredColor = currentKeyframe ? currentKeyframe->colorLabel() : keyframes->keyframeAt(destinationTime)->colorLabel();
    previousKeyframeWithColor(desiredColor);
}

void KisPlaybackEngine::nextMatchingKeyframe()
{
    if (!m_d->activeCanvas || !m_d->activeCanvasAnimationState()) return;

    KisNodeSP node = m_d->activeCanvas->viewManager()->activeNode();
    if (!node) return;

    KisKeyframeChannel *keyframes =
        node->getKeyframeChannel(KisKeyframeChannel::Raster.id());
    if (!keyframes) return;

    int time = m_d->activeCanvasAnimationState()->displayProxy()->activeFrame();

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
    if (!m_d->activeCanvas || !m_d->activeCanvasAnimationState()) return;

    KisNodeSP node = m_d->activeCanvas->viewManager()->activeNode();
    if (!node) return;

    KisKeyframeChannel *keyframes =
        node->getKeyframeChannel(KisKeyframeChannel::Raster.id());
    if (!keyframes) return;

    int time = m_d->activeCanvasAnimationState()->displayProxy()->activeFrame();

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
        if (m_d->activeCanvasAnimationState()->playbackState() != STOPPED) {
            stop();
        }

        seek(destinationTime, SEEK_FINALIZE | SEEK_PUSH_AUDIO);
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
    if (!m_d->activeCanvas || !m_d->activeCanvasAnimationState()) return;

    KisNodeSP node = m_d->activeCanvas->viewManager()->activeNode();
    if (!node) return;

    KisKeyframeChannel *keyframes =
        node->getKeyframeChannel(KisKeyframeChannel::Raster.id());
    if (!keyframes) return;

    int time = m_d->activeCanvasAnimationState()->displayProxy()->activeFrame();

    int destinationTime = keyframes->activeKeyframeTime(time);
    while (keyframes->keyframeAt(destinationTime) &&
           ((destinationTime == time) ||
           !validColors.contains(keyframes->keyframeAt(destinationTime)->colorLabel()))) {

        destinationTime = keyframes->previousKeyframeTime(destinationTime);
    }

    if (keyframes->keyframeAt(destinationTime)) {
        if (m_d->activeCanvasAnimationState()->playbackState() != STOPPED) {
            stop();
        }

        seek(destinationTime, SEEK_FINALIZE | SEEK_PUSH_AUDIO);
    }
}

void KisPlaybackEngine::setupProducer(boost::optional<QFileInfo> file)
{
    if (file.has_value()) {
        QSharedPointer<Mlt::Producer> producer( new Mlt::Producer(*m_d->profile, "ranged", file->absoluteFilePath().toUtf8().data()));
        m_d->canvasProducers[m_d->activeCanvas] = producer;
    } else {
        m_d->canvasProducers[m_d->activeCanvas] = QSharedPointer<Mlt::Producer>(new Mlt::Producer(*m_d->profile, "ranged", "count"));
    }


    KisImageAnimationInterface *animInterface = m_d->activeCanvas->image()->animationInterface();
    QSharedPointer<Mlt::Producer> producer = m_d->canvasProducers[m_d->activeCanvas];
    KIS_ASSERT(producer->is_valid());
    KIS_ASSERT(animInterface);

    producer->set("start_frame", animInterface->documentPlaybackRange().start());
    producer->set("end_frame", animInterface->documentPlaybackRange().end());
    producer->set("limit_enabled", false);
}

void KisPlaybackEngine::setCanvas(KoCanvasBase *p_canvas)
{
    KisCanvas2* canvas = dynamic_cast<KisCanvas2*>(p_canvas);

    if (m_d->activeCanvas == canvas) {
        return;
    }

    if (m_d->activeCanvas) {
        // Disconnect old player, prepare for new one..
        if (m_d->activeCanvasAnimationState()) {
            this->disconnect(m_d->activeCanvasAnimationState());
            m_d->activeCanvasAnimationState()->disconnect(this);
        }

        // Disconnect old image, prepare for new one..
        auto image = m_d->activeCanvas->image();
        if (image && image->animationInterface()) {
            this->disconnect(image->animationInterface());
            image->animationInterface()->disconnect(this);
        }
    }

    StopAndResume stopResume(m_d.data(), true);

    m_d->activeCanvas = canvas;

    // Connect new player..
    if (m_d->activeCanvas) {
        KIS_ASSERT(m_d->activeCanvasAnimationState());

        connect(m_d->activeCanvasAnimationState(), &KisCanvasAnimationState::sigPlaybackStateChanged, this, [this](PlaybackState state){
            Q_UNUSED(state); // We don't need the state yet -- we just want to stop and resume playback according to new state info.
            QSharedPointer<Mlt::Producer> activeProducer = m_d->canvasProducers[m_d->activeCanvas];
            StopAndResume callbackStopResume(m_d.data());
        });

        connect(m_d->activeCanvasAnimationState(), &KisCanvasAnimationState::sigPlaybackMediaChanged, this, [this](){
            setupProducer(m_d->activeCanvasAnimationState()->mediaInfo());
        });

        connect(m_d->activeCanvasAnimationState(), &KisCanvasAnimationState::sigAudioLevelChanged, this, &KisPlaybackEngine::setAudioVolume);

        auto image = m_d->activeCanvas->image();

        KIS_ASSERT(image);

        // Connect new image..
        connect(image->animationInterface(), &KisImageAnimationInterface::sigFramerateChanged, this, [this](){
            StopAndResume callbackStopResume(m_d.data());
            m_d->profile->set_frame_rate(m_d->activeCanvas->image()->animationInterface()->framerate(), 1);
        });

        connect(image->animationInterface(), &KisImageAnimationInterface::sigPlaybackRangeChanged, this, [this](){
            QSharedPointer<Mlt::Producer> producer = m_d->canvasProducers[m_d->activeCanvas];
            auto image = m_d->activeCanvas->image();
            KIS_SAFE_ASSERT_RECOVER_RETURN(image);
            producer->set("start_frame", image->animationInterface()->activePlaybackRange().start());
            producer->set("end_frame", image->animationInterface()->activePlaybackRange().end());
        });

        setupProducer(m_d->activeCanvasAnimationState()->mediaInfo());
    }

}

void KisPlaybackEngine::unsetCanvas() {
    setCanvas(nullptr);
}

void KisPlaybackEngine::throttledShowFrame(const int frame)
{
    if (m_d->activeCanvas && m_d->activePlaybackMode() == PLAYBACK_PULL) {
        m_d->activeCanvasAnimationState()->showFrame(frame);
    }
}

void KisPlaybackEngine::setAudioVolume(qreal volumeNormalized)
{
    if (m_d->mute) {
        m_d->pullConsumer->set("volume", 0.0);
        m_d->pushConsumer->set("volume", 0.0);
    } else {
        m_d->pullConsumer->set("volume", volumeNormalized);
        m_d->pushConsumer->set("volume", volumeNormalized);
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

void KisPlaybackEngine::setMute(bool val)
{
    KIS_ASSERT_RECOVER_RETURN(m_d->activeCanvasAnimationState());
    qreal currentVolume = m_d->activeCanvasAnimationState()->currentVolume();
    m_d->mute = val;
    setAudioVolume(currentVolume);
}

bool KisPlaybackEngine::isMute()
{
    return m_d->mute;
}



