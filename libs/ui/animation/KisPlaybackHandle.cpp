/*
 *  SPDX-FileCopyrightText: 2022 Eoin O'Neill <eoinoneill1991@gmail.com>
 *  SPDX-FileCopyrightText: 2022 Emmet O'Neill <emmetoneill.pdx@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "animation/KisPlaybackHandle.h"

#include <mlt++/MltFactory.h>
#include <mlt++/MltConsumer.h>
#include <mlt++/MltPushConsumer.h>
#include <mlt++/MltProducer.h>
#include <mlt++/MltProfile.h>
#include <functional>

#include "kis_signal_compressor_with_param.h"
#include "kis_debug.h"

struct KisPlaybackHandle::Private {
    explicit Private(KisFrameDisplayProxy* p_display)
        : displayProxy(p_display) {

    }
    PlaybackMode mode = PUSH;
    int framesPerSecond = 24; // TODO: Check if necessary -- perhaps we can just use frameRate setting stored in image entirely??
    QScopedPointer<QFileInfo> media;
    KisFrameDisplayProxy *displayProxy;
};

KisPlaybackHandle::KisPlaybackHandle(KisFrameDisplayProxy* displayProxy, QObject* parent)
    : QObject(parent)
    , m_d(new KisPlaybackHandle::Private(displayProxy))
{
}

KisPlaybackHandle::~KisPlaybackHandle()
{
}

void KisPlaybackHandle::seek(int p_frame)
{
    emit sigDesiredFrameChanged(p_frame);
}

int KisPlaybackHandle::visibleFrame()
{
    KIS_ASSERT(m_d->displayProxy);
    return m_d->displayProxy->visibleFrame();
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

int KisPlaybackHandle::frameRate()
{
    return m_d->framesPerSecond;
}

void KisPlaybackHandle::setMode(PlaybackMode p_setting)
{
    if (m_d->mode != p_setting) {
        m_d->mode = p_setting;
        emit sigModeChange(m_d->mode);
    }
}

PlaybackMode KisPlaybackHandle::getMode()
{
    return m_d->mode;
}

void KisPlaybackHandle::setPlaybackMedia(QFileInfo toLoad)
{
    if (m_d->media && *m_d->media == toLoad) {
        return;
    }


    m_d->media.reset(new QFileInfo(toLoad));
    emit sigPlaybackMediaChanged(*m_d->media);
}

QFileInfo* KisPlaybackHandle::playbackMedia()
{
    return m_d->media.data();
}

