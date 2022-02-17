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

struct KisPlaybackHandle::Private {
    KisPlaybackHandle::Mode mode = KisPlaybackHandle::PUSH;
    int framesPerSecond = 24;
    int desiredFrame = 0;
};

KisPlaybackHandle::KisPlaybackHandle(QObject* parent)
    : QObject(parent)
    , m_d(new KisPlaybackHandle::Private())
{
}

KisPlaybackHandle::~KisPlaybackHandle()
{
}

void KisPlaybackHandle::seek(int p_frame)
{
    if (m_d->desiredFrame != p_frame) {
        m_d->desiredFrame = p_frame;
        emit sigDesiredFrameChanged(m_d->desiredFrame);
    }
}

void KisPlaybackHandle::pushAudio()
{
    if (m_d->mode == PUSH) {
        emit sigRequestPushAudio();
    }
}

void KisPlaybackHandle::setFrameRate(int p_frameRate)
{
    if (p_frameRate != m_d->framesPerSecond) {
        m_d->framesPerSecond = p_frameRate;
        emit sigFrameRateChanged(m_d->framesPerSecond);
    }
}

void KisPlaybackHandle::setMode(Mode setting)
{
    if (m_d->mode != setting) {
        m_d->mode = setting;
        emit sigModeChange(m_d->mode);
    }
}

KisPlaybackHandle::Mode KisPlaybackHandle::getMode()
{
    return m_d->mode;
}

void KisPlaybackHandle::resync(const KisFrameDisplayProxy& displayProxy)
{
}

