#include "KisPlaybackEngine.h"

#include <QMap>

#include "animation/KisMediaConsumer.h"
#include "kis_signal_compressor_with_param.h"

#include <mlt++/Mlt.h>
#include <mlt++/MltConsumer.h>
#include <mlt++/MltFrame.h>

#include "kis_debug.h"

const float SCRUB_AUDIO_SECONDS = 0.25f;

static void onConsumerFrameShow(mlt_consumer c, void* p_self, mlt_frame p_frame) {
    KisPlaybackEngine* self = static_cast<KisPlaybackEngine*>(p_self);
    Mlt::Frame frame(p_frame);
    Mlt::Consumer consumer(c);
    const int position = frame.get_position();
    self->sigChangeActiveCanvasFrame(position);
}

struct KisPlaybackEngine::Private {

    Private( KisPlaybackEngine* p_self ) {
        // Initialize Audio Libraries
        Mlt::Factory::init();

        profile.reset(new Mlt::Profile());
        profile->set_frame_rate(24, 1);

        std::function<void (int)> callback(std::bind(&Private::pushAudio, this, std::placeholders::_1));
        sigPushAudioCompressor.reset(
                    new KisSignalCompressorWithParam<int>(1000 * SCRUB_AUDIO_SECONDS, callback, KisSignalCompressor::FIRST_ACTIVE_POSTPONE_NEXT)
                    );

        initializeConsumers(p_self);
    }

    ~Private() {
        cleanupConsumers();
        Mlt::Factory::close();
    }

    void pushAudio(int frame) {
        if (pushConsumer->is_stopped())
            return;

        KIS_ASSERT(pullConsumer->is_stopped());

        QSharedPointer<KisPlaybackHandle> activeHandle = canvasHandles[activeCanvas];
        QSharedPointer<Mlt::Producer> activeProducer = handleProducers[activeHandle.data()];

        if (activeHandle->getMode() == PlaybackMode::PUSH && activeProducer) {
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

    void initializeConsumers(KisPlaybackEngine* p_self) {
        pullConsumer.reset(new Mlt::Consumer(*profile, "sdl2_audio"));
        pullConsumer->listen("consumer-frame-show", p_self, (mlt_listener)onConsumerFrameShow);

        pushConsumer.reset(new Mlt::PushConsumer(*profile, "sdl2_audio"));
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

    //MLT PUSH CONSUMER
    //MLT PULL CONSUMER
    QScopedPointer<Mlt::Profile> profile;

    QScopedPointer<Mlt::Consumer> pullConsumer;
    QScopedPointer<Mlt::PushConsumer> pushConsumer;

    // Currently active canvas
    KoCanvasBase* activeCanvas;

    // Map of canvases to playback handles
    QMap<KoCanvasBase*, QSharedPointer<KisPlaybackHandle>> canvasHandles;

    //Filters...
    //QSharedPointer<Mlt::Filter> loopFilter;

    // Map of handles to Mlt producers..
    QMap<KisPlaybackHandle*, QSharedPointer<Mlt::Producer>> handleProducers; // TODO: Maybe we can just store a producer in the handle? Should handles remain as abstract as they are now?

    QScopedPointer<KisSignalCompressorWithParam<int>> sigPushAudioCompressor;
};

/**
 * @brief The StopAndResumeConsumer struct is used to encapsulate optional
 * stop-and-then-resume behavior of a consumer. Using RAII, we can stop
 * a consumer at construction and simply resume it when it exits scope.
 */
struct StopAndResumeConsumer {
public:
    explicit StopAndResumeConsumer(Mlt::Consumer* p_consumer)
        : consumer(p_consumer) {
        KIS_ASSERT(consumer);
        wasRunning = !consumer->is_stopped();
        if (wasRunning) {
            consumer->stop();
        }
    }

    ~StopAndResumeConsumer() {
        if (wasRunning) {
            consumer->start();
        }
    }

private:
    Mlt::Consumer* consumer;
    bool wasRunning = false;
};

KisPlaybackEngine::KisPlaybackEngine(QObject *parent)
    : QObject(parent)
    , m_d( new Private(this))
{
}

KisPlaybackEngine::~KisPlaybackEngine()
{
}

QSharedPointer<KisPlaybackHandle> KisPlaybackEngine::leaseHandle(KoCanvasBase* canvas)
{
    QSharedPointer<KisPlaybackHandle> handle(new KisPlaybackHandle);
    m_d->canvasHandles.insert(canvas, handle);

    connect(handle.data(), &KisPlaybackHandle::sigDesiredFrameChanged, this, [this, handle](int p_frame){
        if (m_d->handleProducers.contains(handle.data())) {
            QSharedPointer<Mlt::Producer> producer = m_d->handleProducers[handle.data()];
            producer->seek(p_frame);
        }
    });

    connect(handle.data(), &KisPlaybackHandle::sigPlaybackMediaChanged, this, [this, handle](QFileInfo file){
        QSharedPointer<Mlt::Producer> producer( new Mlt::Producer(*m_d->profile, file.absoluteFilePath().toUtf8().data()));
        if (m_d->canvasHandles[m_d->activeCanvas] == handle.data()) {
            StopAndResumeConsumer(m_d->pushConsumer.data());
            StopAndResumeConsumer(m_d->pullConsumer.data());
            m_d->pullConsumer->disconnect_all_producers();
            m_d->handleProducers[handle.data()] = producer;
            m_d->pullConsumer->connect_producer(*m_d->handleProducers[handle.data()].data());
        } else {
            m_d->handleProducers[handle.data()] = producer;
        }
    });

    QSharedPointer<Mlt::Producer> producer( new Mlt::Producer(*m_d->profile, "count"));
    m_d->handleProducers.insert(handle.data(), producer );

    return handle;
}

void KisPlaybackEngine::returnHandle(KoCanvasBase *canvas)
{
    if (m_d->canvasHandles.contains(canvas)) {
        m_d->canvasHandles[canvas]->disconnect(this);
        m_d->canvasHandles.remove(canvas);
    }
}


void KisPlaybackEngine::setCanvas(KoCanvasBase *canvas)
{
    if (m_d->activeCanvas == canvas) {
        return;
    }

    m_d->pullConsumer->stop();
    m_d->pullConsumer->disconnect_all_producers();

    // Destroy existing consumer setup..
    m_d->cleanupConsumers();

    // Disconnect previously active handle..
    if (m_d->activeCanvas && m_d->canvasHandles.contains(m_d->activeCanvas)) {
        QSharedPointer<KisPlaybackHandle> handle = m_d->canvasHandles[m_d->activeCanvas];

        disconnect(handle.data(), &KisPlaybackHandle::sigRequestPushAudio, this, nullptr);
        disconnect(handle.data(), &KisPlaybackHandle::sigModeChange, this, nullptr);
        disconnect(handle.data(), &KisPlaybackHandle::sigFrameRateChanged, this, nullptr);

        disconnect(this, &KisPlaybackEngine::sigChangeActiveCanvasFrame, this, nullptr);
    }

    m_d->activeCanvas = canvas;

    // Connect newly active handle..
    if (m_d->activeCanvas && m_d->canvasHandles.contains(m_d->activeCanvas)) {
        QSharedPointer<KisPlaybackHandle> handle = m_d->canvasHandles[m_d->activeCanvas];

        connect(handle.data(), &KisPlaybackHandle::sigRequestPushAudio, this, [this](){
            KIS_ASSERT_RECOVER_RETURN(m_d->activeCanvas);
            QSharedPointer<KisPlaybackHandle> handle = m_d->canvasHandles[m_d->activeCanvas];
            QSharedPointer<Mlt::Producer> producer = m_d->handleProducers[handle.data()];
            m_d->sigPushAudioCompressor->start(producer->position());
        });

        connect(handle.data(), &KisPlaybackHandle::sigModeChange, this, &KisPlaybackEngine::setupPlaybackMode);

        connect(handle.data(), &KisPlaybackHandle::sigFrameRateChanged, this, [this](int p_frameRate){
            StopAndResumeConsumer pushStopResume(m_d->pushConsumer.data());
            StopAndResumeConsumer pullStopResume(m_d->pullConsumer.data());

            m_d->profile->set_frame_rate(p_frameRate, 1);
        });

        connect(this, &KisPlaybackEngine::sigChangeActiveCanvasFrame, handle.data(), &KisPlaybackHandle::sigFrameShow);

        // Update MLT Profile to reflect newly active canvas..
        m_d->profile->set_frame_rate(handle->frameRate(), 1);
    }

    // Initialize Consumers w/ New Profile..
    m_d->initializeConsumers(this);

    m_d->pullConsumer->set_profile(*m_d->profile);
    m_d->pushConsumer->set_profile(*m_d->profile);

    // Redo producer connections
    m_d->pullConsumer->connect_producer(*m_d->handleProducers[m_d->canvasHandles[m_d->activeCanvas].data()]);

}

void KisPlaybackEngine::unsetCanvas() {

}

void KisPlaybackEngine::setupPlaybackMode(PlaybackMode p_mode)
{
    if (p_mode == PlaybackMode::PUSH) {
        if (!m_d->pullConsumer->is_stopped()) {
            m_d->pullConsumer->stop();
            m_d->pullConsumer->purge();
        }

        m_d->pushConsumer->start();
    } else {
        if (!m_d->pushConsumer->is_stopped()) {
            m_d->pushConsumer->stop();
            m_d->pushConsumer->purge();
        }

        m_d->pullConsumer->start();
    }
}
