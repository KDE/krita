/* This file is part of the KDE project
 * Copyright (C) 2006-2007 Thomas Zander <zander@kde.org>
 * Copyright (C) 2009 Boudewijn Rempt <boud@valdyas.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */
#include "KoProgressUpdater.h"

#include <QApplication>
#include <QString>
#include <QTextStream>
#include <QTimer>

#include "KoUpdaterPrivate_p.h"
#include "KoUpdater.h"
#include "KoProgressProxy.h"

#include <kis_debug.h>

class Q_DECL_HIDDEN KoProgressUpdater::Private
{
public:

    Private(KoProgressUpdater *_q, KoProgressProxy *proxy, QPointer<KoUpdater> parentUpdater, Mode _mode)
        : q(_q)
        , parentProgressProxy(proxy)
        , parentUpdater(parentUpdater)
        , mode(_mode)
        , currentProgress(0)
        , updated(false)
        , updateGuiTimer(_q)
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
    bool updated;          // is true whenever the progress needs to be recomputed
    QTimer updateGuiTimer; // fires regularly to update the progress bar widget
    QList<QPointer<KoUpdaterPrivate> > subtasks;
    bool canceled;
    int updateInterval = 250; // ms, 4 updates per second should be enough
    bool autoNestNames = false;
    QString taskName;
    int taskMax = -1;
    bool isStarted = false;

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
    connect(&d->updateGuiTimer, SIGNAL(timeout()), SLOT(updateUi()));
}

KoProgressUpdater::KoProgressUpdater(QPointer<KoUpdater> updater)
    : d (new Private(this, 0, updater, Unthreaded))
{
    KIS_ASSERT_RECOVER_RETURN(updater);
    connect(&d->updateGuiTimer, SIGNAL(timeout()), SLOT(updateUi()));
}

KoProgressUpdater::~KoProgressUpdater()
{
    if (d->progressProxy()) {
        d->progressProxy()->setRange(0, d->taskMax);
        d->progressProxy()->setValue(d->progressProxy()->maximum());
    }

    // make sure to stop the timer to avoid accessing
    // the data we are going to delete right now
    d->updateGuiTimer.stop();

    qDeleteAll(d->subtasks);
    d->subtasks.clear();

    delete d;
}

void KoProgressUpdater::start(int range, const QString &text)
{
    d->clearState();
    d->taskName = text;
    d->taskMax = range - 1;
    d->isStarted = true;

    if (d->progressProxy()) {
        d->progressProxy()->setRange(0, d->taskMax);
        d->progressProxy()->setValue(0);
        d->updateParentText();
    }

    d->updateGuiTimer.start(d->updateInterval);
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
    d->subtasks.append(p);
    connect(p, SIGNAL(sigUpdated()), SLOT(update()));

    QPointer<KoUpdater> updater = p->connectedUpdater();

    if (!d->updateGuiTimer.isActive()) {
        // we maybe need to restart the timer if it was stopped in updateUi() cause
        // other sub-tasks created before this one finished already.
        d->updateGuiTimer.start(d->updateInterval);
    }

    d->updated = true;
    return updater;
}

void KoProgressUpdater::removePersistentSubtask(QPointer<KoUpdater> updater)
{
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

    updateUi();
}

void KoProgressUpdater::cancel()
{
    Q_FOREACH (KoUpdaterPrivate *updater, d->subtasks) {
        updater->setProgress(100);
        updater->setInterrupted(true);
    }
    d->canceled = true;
    updateUi();
}

void KoProgressUpdater::update()
{
    d->updated = true;
    if (d->mode == Unthreaded) {
        qApp->processEvents();
    }

    if (!d->updateGuiTimer.isActive()) {
        d->updateGuiTimer.start(d->updateInterval);
    }
}

void KoProgressUpdater::updateUi()
{
    // This function runs in the app main thread. All the progress
    // updates arrive at the KoUpdaterPrivate instances through
    // queued connections, so until we relinquish control to the
    // event loop, the progress values cannot change, and that
    // won't happen until we return from this function (which is
    // triggered by a timer)

    /**
     * We shouldn't let progress updater to interfere the progress
     * reporting when it is not initialized.
     */
    if (d->subtasks.isEmpty()) return;

    if (d->updated) {
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

        d->updated = false;
    }

    if (d->progressProxy()) {
        if (!d->isUndefinedState) {
            d->progressProxy()->setRange(0, d->taskMax);

            if (d->currentProgress == -1) {
                d->currentProgress = d->progressProxy()->maximum();
            }

            if (d->currentProgress >= d->progressProxy()->maximum()) {
                // we're done
                d->updateGuiTimer.stop();
                d->clearState();
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

    progressProxy()->setRange(0, taskMax);
    progressProxy()->setValue(progressProxy()->maximum());

    canceled = false;
}

bool KoProgressUpdater::interrupted() const
{
    return d->canceled;
}

void KoProgressUpdater::setUpdateInterval(int ms)
{
    d->updateInterval = ms;

    if (d->updateGuiTimer.isActive()) {
        d->updateGuiTimer.start(d->updateInterval);
    }
}

int KoProgressUpdater::updateInterval() const
{
    return d->updateInterval;
}

void KoProgressUpdater::setAutoNestNames(bool value)
{
    d->autoNestNames = value;
    update();
}

bool KoProgressUpdater::autoNestNames() const
{
    return d->autoNestNames;
}
