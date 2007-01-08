/*
 *  Copyright (c) 2006 Thomas Zander <zander@kde.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */
#include "kis_progress_updater.h"
#include "kis_progress_updater_p.h"

#include <threadAction/KoAction.h>
#include <threadAction/KoExecutePolicy.h>
#include <threadweaver/ThreadWeaver.h>

#include <QProgressBar>
#include <QString>

KisProgressUpdater::KisProgressUpdater(QProgressBar *progressBar)
    : m_progressBar(progressBar),
    m_totalWeight(0),
    m_currentProgress(0)
{
    Q_ASSERT(m_progressBar);

    m_action = new KoAction(this);
    m_action->setExecutePolicy(KoExecutePolicy::onlyLastPolicy);
    m_action->setWeaver(ThreadWeaver::Weaver::instance());
    connect(m_action, SIGNAL(triggered(const QVariant &)), SLOT(update()));
    connect(m_action, SIGNAL(updateUi(const QVariant &)), SLOT(updateUi()));
}

KisProgressUpdater::~KisProgressUpdater() {
    qDeleteAll(m_subtasks);
    m_subtasks.clear();
}

void KisProgressUpdater::start(int range, const QString &text) {
    m_lock.lock();
    qDeleteAll(m_subtasks);
    m_subtasks.clear();
    m_progressBar->setRange(0, range-1);
    m_progressBar->setValue(0);
    if(! text.isEmpty())
        m_progressBar->setFormat(text);
    m_totalWeight = 0;
    m_lock.unlock();
}

KisUpdater KisProgressUpdater::startSubtask(int weight) {
    m_lock.lock();
    KisUpdaterPrivate *p = new KisUpdaterPrivate(this, weight);
    m_totalWeight += weight;
    m_subtasks.append(p);
    m_lock.unlock();
    return KisUpdater(p);
}

void KisProgressUpdater::scheduleUpdate() {
    m_action->execute();
}

void KisProgressUpdater::update() {
    // this method is called by the action. The action will ensure it is called
    // serially from one thread only. With an updateUi followed directly after
    // this one (forced to the Gui Thread).
    m_lock.lock();
    int totalProgress =0;
    foreach(KisUpdaterPrivate *Kisupdater, m_subtasks) {
        if(Kisupdater->interrupted()) {
            m_currentProgress = -1;
            break;
        }
        int progress = Kisupdater->progress();
        if(progress > 100 || progress < 0)
            progress = Kisupdater->progress(); // see comment in KisUpdaterPrivate cpp file
        totalProgress += progress * Kisupdater->weight();
    }
    m_currentProgress = totalProgress / m_totalWeight;
    m_lock.unlock();
}

void KisProgressUpdater::updateUi() {
    if(m_currentProgress == -1) {
        m_progressBar->setValue(m_progressBar->maximum());
        // should we hide the progressbar after a little while?
        return;
    }
    m_progressBar->setValue(m_currentProgress);
}

void KisProgressUpdater::cancel() {
    m_lock.lock();
    foreach(KisUpdaterPrivate *Kisupdater, m_subtasks) {
        Kisupdater->setProgress(100);
        Kisupdater->interrupt();
    }
    m_lock.unlock();
    scheduleUpdate();
}


// -------- KisUpdater ----------
KisUpdater::KisUpdater(const KisUpdater &other) {
    d = other.d;
}

KisUpdater::KisUpdater(KisUpdaterPrivate *p)
{
    d = p;
    Q_ASSERT(p);
    Q_ASSERT(!d.isNull());
}

void KisUpdater::cancel() {
    if(!d.isNull())
        d->cancel();
}

void KisUpdater::setProgress(int percent) {
    if(!d.isNull())
        d->setProgress(percent);
}

int KisUpdater::progress() const {
    if(d.isNull())
        return 100;
    return d->progress();
}

bool KisUpdater::interrupted() const {
    if(d.isNull())
        return true;
    return d->interrupted();
}

#include "kis_progress_updater.moc"
