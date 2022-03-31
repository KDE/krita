/* This file is part of the KDE project
 * SPDX-FileCopyrightText: 2006-2007 Thomas Zander <zander@kde.org>
 * SPDX-FileCopyrightText: 2009 Boudewijn Rempt <boud@valdyas.org>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */
#include "KoProgressUpdater.h"

#include <QApplication>
#include <QString>
#include <QTimer>
#include <QThread>
#include <QMutex>
#include <QMutexLocker>

#include "KoUpdaterPrivate_p.h"
#include "KoUpdater.h"
#include "KoProgressProxy.h"

#include "kis_signal_compressor.h"

#include <kis_debug.h>

class Q_DECL_HIDDEN KoProgressUpdater::Private
{
public:

    Private(KoProgressUpdater *_q, KoProgressProxy *proxy, QPointer<KoUpdater> parentUpdater, Mode _mode)
        : q(_q)
        , parentProgressProxy(proxy)
        , parentUpdater(parentUpdater)
        , mode(_mode)
        , updateCompressor(new KisSignalCompressor(250, KisSignalCompressor::FIRST_ACTIVE, q))
        , canceled(false)
    {
    }

    KoProgressUpdater *q;

private:
    KoProgressProxy *parentProgressProxy;
    QPointer<KoUpdater> parentUpdater;

public:
    Mode mode;
    int currentProgress = 0;
    bool isUndefinedState = false;
    KisSignalCompressor *updateCompressor;
    QList<QPointer<KoUpdaterPrivate> > subtasks;
    bool canceled;
    int updateInterval = 250; // ms, 4 updates per second should be enough
    bool autoNestNames = false;
    QString taskName;
    int taskMax = 99;
    bool isStarted = false;

    QMutex mutex;

    void updateParentText();
    void clearState();

    KoProgressProxy* progressProxy() {
        return parentUpdater ? parentUpdater : parentProgressProxy;
    }
};

// NOTE: do not make the KoProgressUpdater object part of the QObject
// hierarchy. Do not make KoProgressProxy its parent (note that KoProgressProxy
// is not necessarily castable to QObject ). This prevents proper functioning
// of progress reporting in multi-threaded environments.
KoProgressUpdater::KoProgressUpdater(KoProgressProxy *progressProxy, Mode mode)
    : d (new Private(this, progressProxy, 0, mode))
{
    KIS_ASSERT_RECOVER_RETURN(progressProxy);
    connect(d->updateCompressor, SIGNAL(timeout()), SLOT(updateUi()));
    connect(this, SIGNAL(triggerUpdateAsynchronously()), d->updateCompressor, SLOT(start()));
    Q_EMIT triggerUpdateAsynchronously();
}

KoProgressUpdater::KoProgressUpdater(QPointer<KoUpdater> updater)
    : d (new Private(this, 0, updater, Unthreaded))
{
    KIS_ASSERT_RECOVER_RETURN(updater);
    connect(d->updateCompressor, SIGNAL(timeout()), SLOT(updateUi()));
    connect(this, SIGNAL(triggerUpdateAsynchronously()), d->updateCompressor, SLOT(start()));
    Q_EMIT triggerUpdateAsynchronously();
}

KoProgressUpdater::~KoProgressUpdater()
{
    if (d->progressProxy()) {
        d->progressProxy()->setRange(0, d->taskMax);
        d->progressProxy()->setValue(d->progressProxy()->maximum());
    }

    // make sure to stop the timer to avoid accessing
    // the data we are going to delete right now
    d->updateCompressor->stop();

    qDeleteAll(d->subtasks);
    d->subtasks.clear();

    delete d;
}

void KoProgressUpdater::start(int range, const QString &text)
{
    {
        QMutexLocker l(&d->mutex);
        d->clearState();
        d->taskName = text;
        d->taskMax = range - 1;
        d->isStarted = true;
        d->currentProgress = 0;
    }

    Q_EMIT triggerUpdateAsynchronously();
}

QPointer<KoUpdater> KoProgressUpdater::startSubtask(int weight,
                                                    const QString &name,
                                                    bool isPersistent)
{
    if (!d->isStarted) {
        // lazy initialization for intermediate proxies
        start();
    }

    KoUpdaterPrivate *p = new KoUpdaterPrivate(this, weight, name, isPersistent);

    {
        QMutexLocker l(&d->mutex);
        d->subtasks.append(p);
    }
    connect(p, SIGNAL(sigUpdated()), SLOT(update()));

    QPointer<KoUpdater> updater = p->connectedUpdater();

    Q_EMIT triggerUpdateAsynchronously();
    return updater;
}

void KoProgressUpdater::removePersistentSubtask(QPointer<KoUpdater> updater)
{
    {
        QMutexLocker l(&d->mutex);

        for (auto it = d->subtasks.begin(); it != d->subtasks.end();) {
            if ((*it)->connectedUpdater() != updater) {
                ++it;
            } else {
                KIS_SAFE_ASSERT_RECOVER_NOOP((*it)->isPersistent());
                (*it)->deleteLater();
                it = d->subtasks.erase(it);
                break;
            }
        }
    }

    Q_EMIT triggerUpdateAsynchronously();
}

void KoProgressUpdater::cancel()
{
    KIS_SAFE_ASSERT_RECOVER_RETURN(QThread::currentThread() == this->thread());

    QList<QPointer<KoUpdaterPrivate> > subtasks;

    {
        QMutexLocker l(&d->mutex);
        subtasks = d->subtasks;
    }

    Q_FOREACH (QPointer<KoUpdaterPrivate> updater, subtasks) {
        if (!updater) continue;

        updater->setProgress(100);
        updater->setInterrupted(true);
    }
    d->canceled = true;

    Q_EMIT triggerUpdateAsynchronously();
}

void KoProgressUpdater::update()
{
    KIS_SAFE_ASSERT_RECOVER_RETURN(QThread::currentThread() == this->thread());

    if (d->mode == Unthreaded) {
        qApp->processEvents();
    }

    d->updateCompressor->start();
}

void KoProgressUpdater::updateUi()
{
    KIS_SAFE_ASSERT_RECOVER_RETURN(QThread::currentThread() == this->thread());

    // This function runs in the app main thread. All the progress
    // updates arrive at the KoUpdaterPrivate instances through
    // queued connections, so until we relinquish control to the
    // event loop, the progress values cannot change, and that
    // won't happen until we return from this function (which is
    // triggered by a timer)

    {
        QMutexLocker l(&d->mutex);

        if (!d->subtasks.isEmpty()) {
            int totalProgress = 0;
            int totalWeight = 0;
            d->isUndefinedState = false;

            Q_FOREACH (QPointer<KoUpdaterPrivate> updater, d->subtasks) {
                if (updater->interrupted()) {
                    d->currentProgress = -1;
                    break;
                }

                if (!updater->hasValidRange()) {
                    totalWeight = 0;
                    totalProgress = 0;
                    d->isUndefinedState = true;
                    break;
                }

                if (updater->isPersistent() && updater->isCompleted()) {
                    continue;
                }

                const int progress = qBound(0, updater->progress(), 100);
                totalProgress += progress * updater->weight();
                totalWeight += updater->weight();
            }

            const int progressPercent = totalWeight > 0 ? totalProgress / totalWeight : -1;

            d->currentProgress =
                    d->taskMax == 99 ?
                        progressPercent :
                        qRound(qreal(progressPercent) * d->taskMax / 99.0);
        }

    }

    if (d->progressProxy()) {
        if (!d->isUndefinedState) {
            d->progressProxy()->setRange(0, d->taskMax);

            if (d->currentProgress == -1) {
                d->currentProgress = d->progressProxy()->maximum();
            }

            if (d->currentProgress >= d->progressProxy()->maximum()) {
                {
                    QMutexLocker l(&d->mutex);
                    d->clearState();
                }
                d->progressProxy()->setRange(0, d->taskMax);
                d->progressProxy()->setValue(d->progressProxy()->maximum());
            } else {
                d->progressProxy()->setValue(d->currentProgress);
            }
        } else {
            d->progressProxy()->setRange(0,0);
            d->progressProxy()->setValue(0);
        }

        d->updateParentText();
    }
}

void KoProgressUpdater::Private::updateParentText()
{
    if (!progressProxy()) return;

    QString actionName = taskName;

    if (autoNestNames) {
        Q_FOREACH (QPointer<KoUpdaterPrivate> updater, subtasks) {

            if (updater->isPersistent() && updater->isCompleted()) {
                continue;
            }

            if (updater->progress() < 100) {
                const QString subTaskName = updater->mergedSubTaskName();

                if (!subTaskName.isEmpty()) {
                    if (actionName.isEmpty()) {
                        actionName = subTaskName;
                    } else {
                        actionName = QString("%1: %2").arg(actionName).arg(subTaskName);
                    }
                }
                break;
            }
        }
        progressProxy()->setAutoNestedName(actionName);
    } else {
        progressProxy()->setFormat(actionName);
    }

}

void KoProgressUpdater::Private::clearState()
{
    for (auto it = subtasks.begin(); it != subtasks.end();) {
        if (!(*it)->isPersistent()) {
            (*it)->deleteLater();
            it = subtasks.erase(it);
        } else {
            if ((*it)->interrupted()) {
                (*it)->setInterrupted(false);
            }
            ++it;
        }
    }

    canceled = false;
}

bool KoProgressUpdater::interrupted() const
{
    return d->canceled;
}

void KoProgressUpdater::setUpdateInterval(int ms)
{
    d->updateCompressor->setDelay(ms);
}

int KoProgressUpdater::updateInterval() const
{
    return d->updateCompressor->delay();
}

void KoProgressUpdater::setAutoNestNames(bool value)
{
    d->autoNestNames = value;
}

bool KoProgressUpdater::autoNestNames() const
{
    return d->autoNestNames;
}
