/*
 *  SPDX-FileCopyrightText: 2015 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_idle_watcher.h"

#include <QTimer>
#include "kis_image.h"
#include "kis_signal_auto_connection.h"
#include "kis_signal_compressor.h"


struct KisIdleWatcher::Private
{
    static const int IDLE_CHECK_COUNT = 4; /* ticks */

    Private(int delay, KisIdleWatcher *q)
        : imageModifiedCompressor(delay,
                                  KisSignalCompressor::POSTPONE, q),
          idleCheckCounter(0)
    {
        idleCheckTimer.setSingleShot(true);
        idleCheckTimer.setInterval(delay);
    }

    KisSignalAutoConnectionsStore connectionsStore;
    QVector<KisImageWSP> trackedImages;

    KisSignalCompressor imageModifiedCompressor;

    QTimer idleCheckTimer;

    /**
     * We wait until the counter reaches IDLE_CHECK_COUNT, then consider the
     * image to be really "idle". If the counter is negative, it means that
     * "no delay" update is triggered, which disables counting and the event
     * is triggered on the next non-busy tick.
     */
    int idleCheckCounter;
};

KisIdleWatcher::KisIdleWatcher(int delay, QObject *parent)
    : QObject(parent), m_d(new Private(delay, this))
{
    connect(&m_d->imageModifiedCompressor, SIGNAL(timeout()), SLOT(startIdleCheck()));
    connect(&m_d->idleCheckTimer, SIGNAL(timeout()), SLOT(slotIdleCheckTick()));
}

KisIdleWatcher::~KisIdleWatcher()
{
}

bool KisIdleWatcher::isIdle() const
{
    bool idle = true;

    Q_FOREACH (KisImageSP image, m_d->trackedImages) {
        if (!image) continue;

        if (!image->isIdle()) {
            idle = false;
            break;
        }
    }

    return idle;
}

bool KisIdleWatcher::isCounting() const
{
    return m_d->idleCheckTimer.isActive();
}

void KisIdleWatcher::setTrackedImages(const QVector<KisImageSP> &images)
{
    m_d->connectionsStore.clear();
    m_d->trackedImages.clear();

    Q_FOREACH (KisImageSP image, images) {
        if (image) {
            m_d->trackedImages << image;
            m_d->connectionsStore.addConnection(image, SIGNAL(sigImageModified()),
                                                this, SLOT(slotImageModified()));

            m_d->connectionsStore.addConnection(image, SIGNAL(sigIsolatedModeChanged()),
                                                this, SLOT(slotImageModified()));
        }
    }
}

void KisIdleWatcher::setTrackedImage(KisImageSP image)
{
    QVector<KisImageSP> images;
    images << image;
    setTrackedImages(images);
}

void KisIdleWatcher::restartCountdown()
{
    stopIdleCheck();
    m_d->imageModifiedCompressor.start();
}

void KisIdleWatcher::triggerCountdownNoDelay()
{
    stopIdleCheck();
    m_d->idleCheckCounter = -1;
    m_d->idleCheckTimer.start();
}

void KisIdleWatcher::slotImageModified()
{
    if (m_d->idleCheckCounter >= 0) {
        restartCountdown();
    }
    Q_EMIT imageModified();
}

void KisIdleWatcher::startIdleCheck()
{
    m_d->idleCheckCounter = 0;
    m_d->idleCheckTimer.start();
}

void KisIdleWatcher::stopIdleCheck()
{
    m_d->idleCheckTimer.stop();
    m_d->idleCheckCounter = 0;
}

void KisIdleWatcher::slotIdleCheckTick()
{
    if (isIdle()) {
        if (m_d->idleCheckCounter < 0 ||
            m_d->idleCheckCounter >= Private::IDLE_CHECK_COUNT) {

            stopIdleCheck();
            if (!m_d->trackedImages.isEmpty()) {
                Q_EMIT startedIdleMode();
            }
        } else {
            m_d->idleCheckCounter++;
            m_d->idleCheckTimer.start();
        }
    } else {
        if (m_d->idleCheckCounter >= 0) {
            restartCountdown();
        } else {
            m_d->idleCheckTimer.start();
        }
    }
}
