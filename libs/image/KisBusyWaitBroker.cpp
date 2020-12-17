/*
 *  SPDX-FileCopyrightText: 2020 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "KisBusyWaitBroker.h"

#include <QGlobalStatic>
#include <QMutex>
#include <QMutexLocker>

#include <QThread>
#include <QApplication>

#include "kis_image.h"


Q_GLOBAL_STATIC(KisBusyWaitBroker, s_instance)


struct KisBusyWaitBroker::Private
{
    QMutex lock;
    QSet<KisImage*> waitingOnImages;
    int guiThreadLockCount = 0;

    std::function<void(KisImageSP)> feedbackCallback;
};


KisBusyWaitBroker::KisBusyWaitBroker()
    : m_d(new Private)
{
}

KisBusyWaitBroker::~KisBusyWaitBroker()
{
}

KisBusyWaitBroker *KisBusyWaitBroker::instance()
{
    return s_instance;
}

void KisBusyWaitBroker::notifyWaitOnImageStarted(KisImage* image)
{
    if (QThread::currentThread() != qApp->thread()) return;

    {
        QMutexLocker l(&m_d->lock);
        m_d->guiThreadLockCount++;
        KIS_SAFE_ASSERT_RECOVER_NOOP(!m_d->waitingOnImages.contains(image));
        m_d->waitingOnImages.insert(image);
    }

    if (m_d->feedbackCallback && image->refCount()) {
        m_d->feedbackCallback(image);
    }
}

void KisBusyWaitBroker::notifyWaitOnImageEnded(KisImage* image)
{
    if (QThread::currentThread() != qApp->thread()) return;

    {
        QMutexLocker l(&m_d->lock);
        m_d->guiThreadLockCount--;
        KIS_SAFE_ASSERT_RECOVER_NOOP(m_d->waitingOnImages.contains(image));
        m_d->waitingOnImages.remove(image);
    }
}

void KisBusyWaitBroker::notifyGeneralWaitStarted()
{
    if (QThread::currentThread() != qApp->thread()) return;

    QMutexLocker l(&m_d->lock);
    m_d->guiThreadLockCount++;
}

void KisBusyWaitBroker::notifyGeneralWaitEnded()
{
    if (QThread::currentThread() != qApp->thread()) return;

    QMutexLocker l(&m_d->lock);
    m_d->guiThreadLockCount--;
}

void KisBusyWaitBroker::setFeedbackCallback(std::function<void (KisImageSP)> callback)
{
    m_d->feedbackCallback = callback;
}

bool KisBusyWaitBroker::guiThreadIsWaitingForBetterWeather() const
{
    return m_d->guiThreadLockCount;
}


