/*
 *  SPDX-FileCopyrightText: 2022 Eoin O'Neill <eoinoneill1991@gmail.com>
 *  SPDX-FileCopyrightText: 2022 Emmet O'Neill <emmetoneill.pdx@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "animation/KisMediaConsumer.h"

#include <mlt++/MltFactory.h>
#include <mlt++/MltConsumer.h>
#include <mlt++/MltPushConsumer.h>
#include <mlt++/MltProducer.h>
#include <mlt++/MltProfile.h>
#include <functional>

#include "kis_signal_compressor_with_param.h"
#include "kis_debug.h"


const float SCRUB_AUDIO_SECONDS = 0.25f;

static void onConsumerFrameShow(mlt_consumer c, void* p_self, mlt_frame p_frame) {
    KisMediaConsumer* self = static_cast<KisMediaConsumer*>(p_self);
    Mlt::Frame frame(p_frame);
    Mlt::Consumer consumer(c);
    //const float speed = consumer.producer()->get_double("warp_speed");
    //const int actualPosition = consumerPosition * speed;
    const int consumerPosition = frame.get_position();
    ENTER_FUNCTION() << ppVar(consumer.producer()->profile()->frame_rate_num()) << ppVar(consumer.profile()->frame_rate_num()) << ppVar(self->temp_stopWatch());
    self->sigFrameShow(consumerPosition);
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
                    new KisSignalCompressorWithParam<int>(1000 * SCRUB_AUDIO_SECONDS, callback, KisSignalCompressor::FIRST_ACTIVE_POSTPONE_NEXT)
                    );

        temp = QElapsedTimer();
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

        KIS_ASSERT(pullConsumer->is_stopped());

        if (mode == KisMediaConsumer::PUSH && producer) {
            const int SCRUB_AUDIO_WINDOW = profile->frame_rate_num() * SCRUB_AUDIO_SECONDS;
            for (int i = 0; i < SCRUB_AUDIO_WINDOW; i++ ) {
                Mlt::Frame* f = producer->get_frame(frame + i );
                pushConsumer->push(f);
            }
            // It turns out that get_frame actually seeks to the frame too,
            // Not having this last seek will cause unexpected "jumps" at
            // the beginning of playback...
            producer->seek(frame);
        }
    }

    KisMediaConsumer::Mode mode;

    QScopedPointer<Mlt::Profile> profile;

    QScopedPointer<Mlt::Consumer> pullConsumer;
    QScopedPointer<Mlt::PushConsumer> pushConsumer;

    //Filters...
    //QSharedPointer<Mlt::Filter> loopFilter;

    QSharedPointer<Mlt::Producer> producer;

    QScopedPointer<KisSignalCompressorWithParam<int>> sigPushAudioCompressor;

    QElapsedTimer temp;
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


KisMediaConsumer::KisMediaConsumer(QObject* parent)
    : QObject(parent)
    , m_d(new Private(this))
{
    QSharedPointer<Mlt::Producer> p( new Mlt::Producer(*m_d->profile, "timewarp", "1.0:count"));
    setProducer(p);
}

KisMediaConsumer::~KisMediaConsumer()
{
}

void KisMediaConsumer::seek(int p_frame)
{
    if (m_d->producer) {
        m_d->producer->seek(p_frame);
    }
}

void KisMediaConsumer::pushAudio()
{
    if (m_d->producer && m_d->mode == PUSH) {
        const int pos = m_d->producer->position();
        m_d->sigPushAudioCompressor->start(pos);
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
    StopAndResumeConsumer srPullConsumer(m_d->pullConsumer.data());
    StopAndResumeConsumer srPushConsumer(m_d->pushConsumer.data());

    m_d->profile->set_frame_rate(fps, 1);
}

int KisMediaConsumer::getFrameRate()
{
    return m_d->profile->frame_rate_num();
}

void KisMediaConsumer::setPlaybackSpeed(float p_nSpeed)
{
    StopAndResumeConsumer srPullConsumer(m_d->pullConsumer.data());
    StopAndResumeConsumer srPushConsumer(m_d->pushConsumer.data());

    /*QString file(m_d->producer->get("warp_resource"));
    QString resource = (QString::number(p_nSpeed) + ":" + file);
    m_d->producer->set("resource", resource.toUtf8().data());*/
}

float KisMediaConsumer::playbackSpeed()
{
    return m_d->producer->get_double("warp_speed");
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

void KisMediaConsumer::resync(const KisFrameDisplayProxy& displayProxy)
{
    m_d->producer->seek(displayProxy.visibleFrame());
}

QString KisMediaConsumer::debugInfo() {
    return QString("producer_position = ") + QString::number(m_d->producer->position()) + QString(", consumer_position = ") + QString::number(m_d->pullConsumer->position());
}

Mlt::Profile* KisMediaConsumer::getProfile()
{
    return m_d->profile.data();
}

void KisMediaConsumer::setProducer(QSharedPointer<Mlt::Producer> p_producer)
{
    StopAndResumeConsumer c1(m_d->pushConsumer.data());
    StopAndResumeConsumer c2(m_d->pullConsumer.data());
    m_d->pullConsumer->disconnect_all_producers();
    m_d->producer = p_producer;
    m_d->producer->set_profile(*m_d->profile);
    m_d->pullConsumer->connect_producer(*m_d->producer);
}

float KisMediaConsumer::temp_stopWatch()
{
    qint64 elapsed = m_d->temp.elapsed();
    m_d->temp.restart();
    return elapsed;
}
