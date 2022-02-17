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
    //KisPlaybackEngine* self = static_cast<KisPlaybackEngine*>(p_self);
    Mlt::Frame frame(p_frame);
    Mlt::Consumer consumer(c);
    const int position = frame.get_position();
    ENTER_FUNCTION() << ppVar(position);
    //self->sigFrameShow(consumerPosition);
}

struct KisPlaybackEngine::Private {

    Private( KisPlaybackEngine* p_self ) {
        profile.reset(new Mlt::Profile());
        profile->set_frame_rate(24, 1);

        pullConsumer.reset(new Mlt::Consumer(*profile, "sdl2_audio"));
        pullConsumer->listen("consumer-frame-show", p_self, (mlt_listener)onConsumerFrameShow);

        pushConsumer.reset(new Mlt::PushConsumer(*profile, "sdl2_audio"));
        pushConsumer->start();

        std::function<void (int)> callback(std::bind(&Private::pushAudio, this, std::placeholders::_1));
        sigPushAudioCompressor.reset(
                    new KisSignalCompressorWithParam<int>(1000 * SCRUB_AUDIO_SECONDS, callback, KisSignalCompressor::FIRST_ACTIVE_POSTPONE_NEXT)
                    );

    }

    ~Private() {
        if (!pullConsumer->is_stopped()) {
            pullConsumer->stop();
        }

        if (!pushConsumer->is_stopped()) {
            pushConsumer->stop();
        }

        pullConsumer.reset();
        pushConsumer.reset();
    }

    void pushAudio(int frame) {
        if (pushConsumer->is_stopped())
            return;

        KIS_ASSERT(pullConsumer->is_stopped());

        QSharedPointer<KisPlaybackHandle> activeHandle = canvasHandles[activeCanvas];
        QSharedPointer<Mlt::Producer> activeProducer = handleProducers[activeHandle.data()];

        if (activeHandle->getMode() == KisPlaybackHandle::PUSH && activeProducer) {
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
    QMap<KisPlaybackHandle*, QSharedPointer<Mlt::Producer>> handleProducers;

    QScopedPointer<KisSignalCompressorWithParam<int>> sigPushAudioCompressor;
};

/**
 * @brief The StopAndResumeConsumer struct is used to encapsulate optional
 * stop-and-then-resume behavior of a consumer. Using RAII, we can stop
 * a consumer at construction and simply resume it when it exits scope.
 */
struct StopAndResumeConsumer {
public:
    StopAndResumeConsumer(Mlt::Consumer* p_consumer)
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
}

void KisPlaybackEngine::unsetCanvas() {

}
