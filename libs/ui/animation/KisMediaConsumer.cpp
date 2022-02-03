#include "animation/KisMediaConsumer.h"

#include <mlt++/MltFactory.h>
#include <mlt++/MltConsumer.h>
#include <mlt++/MltPushConsumer.h>
#include <mlt++/MltProducer.h>
#include <mlt++/MltProfile.h>
#include <functional>

#include "kis_signal_compressor_with_param.h"

const int SCRUB_AUDIO_WINDOW = 5;

static void onConsumerFrameShow(mlt_consumer c, void* p_self, mlt_frame p_frame) {
    const KisMediaConsumer* self = static_cast<KisMediaConsumer*>(p_self);
    Mlt::Frame frame(p_frame);
    self->sigFrameShow(frame.get_position());
}

struct Private {
    Private(KisMediaConsumer* p_self)
    {
        Mlt::Factory::init();

        profile.reset(new Mlt::Profile());
        profile->set_frame_rate(24, 1);

        pullConsumer.reset(new Mlt::Consumer(*profile, "sdl2_audio"));
        pullConsumer->listen("consumer-frame-show", p_self, (mlt_listener)onConsumerFrameShow);

        pushConsumer.reset(new Mlt::PushConsumer(*profile, "sdl2_audio"));
        pushConsumer->start();

        std::function<void (int)> callback(std::bind(&Private::pushAudio, this, std::placeholders::_1));
        sigPushAudioCompressor.reset(
                    new KisSignalCompressorWithParam<int>(35 * SCRUB_AUDIO_WINDOW, callback, KisSignalCompressor::FIRST_ACTIVE_POSTPONE_NEXT)
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
        Mlt::Factory::close();
    }

    void pushAudio(int frame) {
        if (pushConsumer->is_stopped())
            return;

        if (mode == KisMediaConsumer::PUSH && producer) {
            for (int i = 0; i < SCRUB_AUDIO_WINDOW; i++ ) {
                Mlt::Frame* f = producer->get_frame(frame + i );
                pushConsumer->push(f);
            }
        }
    }

    KisMediaConsumer::Mode mode;

    QScopedPointer<Mlt::Profile> profile;

    QScopedPointer<Mlt::Consumer> pullConsumer;
    QScopedPointer<Mlt::PushConsumer> pushConsumer;

    //Filters...
    //QSharedPointer<Mlt::Filter> loopFilter
    //QSharedPointer<Mlt::Filter> timeWarpFilter

    QSharedPointer<Mlt::Producer> producer;

    QScopedPointer<KisSignalCompressorWithParam<int>> sigPushAudioCompressor;
};


KisMediaConsumer::KisMediaConsumer(QObject* parent)
    : QObject(parent)
    , m_d(new Private(this))
{
    QSharedPointer<Mlt::Producer> p( new Mlt::Producer(*m_d->profile, "count"));
    setProducer(p);
}

KisMediaConsumer::~KisMediaConsumer()
{
}

void KisMediaConsumer::seek(int p_frame)
{
    if (m_d->producer) {
        m_d->producer->seek(p_frame);
        m_d->sigPushAudioCompressor->start(p_frame);
    }
}

int KisMediaConsumer::playhead()
{
    if (m_d->producer) {
        return m_d->producer->position();
    } else {
        return -1;
    }
}

void KisMediaConsumer::setFrameRate(int fps)
{
    m_d->profile->set_frame_rate(fps, 1);
}

int KisMediaConsumer::getFrameRate()
{
    return m_d->profile->frame_rate_num();
}

void KisMediaConsumer::setMode(Mode setting)
{
    if (m_d->mode != setting) {
        m_d->mode = setting;
        if (m_d->mode == Mode::PUSH) {
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
}

KisMediaConsumer::Mode KisMediaConsumer::getMode()
{
    return m_d->mode;
}

Mlt::Profile* KisMediaConsumer::getProfile()
{
    return m_d->profile.data();
}

void KisMediaConsumer::setProducer(QSharedPointer<Mlt::Producer> p_producer)
{
    bool restartPullConsumer = false;
    if (m_d->mode == KisMediaConsumer::PULL && !m_d->pullConsumer->is_stopped()) {
        m_d->pullConsumer->stop();
        restartPullConsumer = true;
    }

    m_d->pullConsumer->disconnect_all_producers();
    m_d->producer = p_producer;
    m_d->producer->set_profile(*m_d->profile);
    m_d->pullConsumer->connect_producer(*m_d->producer);

    if (restartPullConsumer) {
        m_d->pullConsumer->start();
    } else {
        m_d->pushConsumer->stop();
        m_d->pushConsumer->start();
    }
}
