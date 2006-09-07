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

#include "ExecutePolicy.h"
#include "Action.h"
#include "ActionJob_p.h"
#include "JobsListPolicy.h"
#include <WeaverInterface.h>

namespace ThreadWeaver {

void OnlyLastPolicy::schedule(Action *action, JobsListPolicy *jobsList, QVariant *params) {
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

void DirectPolicy::schedule(Action *action, JobsListPolicy *jobsList, QVariant *params) {
    Q_UNUSED(jobsList);
    ActionJob *job = new ActionJob(action, ActionJob::EnableNoChange, params);
    job->run();
}

void QueuedPolicy::schedule(Action *action, JobsListPolicy *jobsList, QVariant *params) {
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

void SimpleQueuedPolicy::schedule(Action *action, JobsListPolicy *jobsList, QVariant *params) {
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
ExecutePolicy *const ExecutePolicy::onlyLastPolicy = new OnlyLastPolicy();
ExecutePolicy *const ExecutePolicy::directPolicy = new DirectPolicy();
ExecutePolicy *const ExecutePolicy::queuedPolicy = new QueuedPolicy();
ExecutePolicy *const ExecutePolicy::simpleQueuedPolicy = new SimpleQueuedPolicy();
}
