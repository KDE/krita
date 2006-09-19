/*
 * Copyright (C) 2006 Thomas Zander <zander@kde.org>
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

#include "KoExecutePolicy.h"
#include "KoAction.h"
#include "ActionJob_p.h"
#include "KoJobsListPolicy.h"
#include <WeaverInterface.h>

using namespace ThreadWeaver;

void KoOnlyLastPolicy::schedule(KoAction *action, KoJobsListPolicy *jobsList, QVariant *params) {
    if(action->weaver() == 0) {
        qWarning("Action has no weaver set, ignoring scheduling request");
        return;
    }
    jobsList->lock();
    foreach(Job *job, jobsList->jobs()) {
        ActionJob *aj = dynamic_cast<ActionJob*> (job);
        if(aj && aj->action() == action) {
            if(! aj->started()) {
                action->weaver()->dequeue(aj);
                jobsList->free(aj);
            }
        }
    }
    jobsList->unlock();
    ActionJob *job = new ActionJob(action, action->isEnabled() ? ActionJob::EnableOn :
            ActionJob::EnableOff, params);
    job->assignQueuePolicy(jobsList);
    jobsList->addJob(job);
    action->weaver()->enqueue(job);
}

void KoDirectPolicy::schedule(KoAction *action, KoJobsListPolicy *jobsList, QVariant *params) {
    Q_UNUSED(jobsList);
    ActionJob *job = new ActionJob(action, ActionJob::EnableNoChange, params);
    job->run();
}

void KoQueuedPolicy::schedule(KoAction *action, KoJobsListPolicy *jobsList, QVariant *params) {
    if(action->weaver() == 0) {
        qWarning("Action has no weaver set, ignoring scheduling request");
        return;
    }
    ActionJob *job = new ActionJob(action, action->isEnabled() ? ActionJob::EnableOn :
            ActionJob::EnableOff, params);
    job->assignQueuePolicy(jobsList);
    jobsList->addJob(job);
    action->setEnabled(false);
    action->weaver()->enqueue(job);
}

void KoSimpleQueuedPolicy::schedule(KoAction *action, KoJobsListPolicy *jobsList, QVariant *params) {
    if(action->weaver() == 0) {
        qWarning("Action has no weaver set, ignoring scheduling request");
        return;
    }
    ActionJob *job = new ActionJob(action, ActionJob::EnableNoChange, params);
    job->assignQueuePolicy(jobsList);
    jobsList->addJob(job);
    action->weaver()->enqueue(job);
}

// statics
KoExecutePolicy *const KoExecutePolicy::onlyLastPolicy = new KoOnlyLastPolicy();
KoExecutePolicy *const KoExecutePolicy::directPolicy = new KoDirectPolicy();
KoExecutePolicy *const KoExecutePolicy::queuedPolicy = new KoQueuedPolicy();
KoExecutePolicy *const KoExecutePolicy::simpleQueuedPolicy = new KoSimpleQueuedPolicy();
