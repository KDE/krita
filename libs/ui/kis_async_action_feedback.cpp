/*
 *  SPDX-FileCopyrightText: 2016 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_async_action_feedback.h"

#include <QtConcurrent>
#include <QProgressDialog>


struct KisAsyncActionFeedback::Private
{
    QScopedPointer<QProgressDialog> progress;
};

KisAsyncActionFeedback::KisAsyncActionFeedback(const QString &message, QWidget *parent)
    : m_d(new Private)
{
    m_d->progress.reset(new QProgressDialog(message, "", 0, 0, parent));
    m_d->progress->setWindowModality(Qt::ApplicationModal);
    m_d->progress->setCancelButton(0);
    m_d->progress->setMinimumDuration(1000);
    m_d->progress->setValue(0);

    // disable close button
    m_d->progress->setWindowFlags(Qt::CustomizeWindowHint | Qt::WindowTitleHint);
}

KisAsyncActionFeedback::~KisAsyncActionFeedback()
{
}

template <typename T>
T runActionImpl(std::function<T()> func)
{
    QFuture<T> result = QtConcurrent::run(func);
    QFutureWatcher<T> watcher;
    watcher.setFuture(result);

    while (watcher.isRunning()) {
        qApp->processEvents(QEventLoop::ExcludeUserInputEvents);
    }

    watcher.waitForFinished();
    return watcher.result();
}

KisImportExportErrorCode KisAsyncActionFeedback::runAction(std::function<KisImportExportErrorCode()> func)
{
    return runActionImpl(func);
}

void KisAsyncActionFeedback::runVoidAction(std::function<void()> func)
{
    QFuture<void> result = QtConcurrent::run(func);
    QFutureWatcher<void> watcher;
    watcher.setFuture(result);

    while (watcher.isRunning()) {
        qApp->processEvents(QEventLoop::ExcludeUserInputEvents);
    }

    watcher.waitForFinished();
}

void KisAsyncActionFeedback::waitForMutex(QMutex *mutex)
{
    while (!mutex->tryLock()) {
        qApp->processEvents(QEventLoop::ExcludeUserInputEvents);
    }

    mutex->unlock();
}
