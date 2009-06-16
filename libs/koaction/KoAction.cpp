/*
 * Copyright (C) 2006-2007 Thomas Zander <zander@kde.org>
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

#include "KoAction.h"

#include "KoJobsListPolicy.h"
#include "KoExecutePolicy.h"

#include <threadweaver/WeaverInterface.h>

class KoAction::Private
{
public:
    Private()
            : policy(KoExecutePolicy::simpleQueuedPolicy),
            weaver(ThreadWeaver::Weaver::instance()),
            enabled(true) {
        jobsQueue = new KoJobsListPolicy();
    }

    ~Private() {
        delete jobsQueue;
    }

    KoExecutePolicy *policy;
    ThreadWeaver::WeaverInterface *weaver;
    bool enabled;
    KoJobsListPolicy *jobsQueue;
};


KoAction::KoAction(QObject *parent)
        : QObject(parent),
        d(new Private())
{
}

KoAction::~KoAction()
{
    delete d;
}

void KoAction::execute(const QVariant &params)
{
    if (!d->enabled)
        return;
    Q_ASSERT(d->weaver);
    d->policy->schedule(this, d->jobsQueue, params);
}

void KoAction::doAction(const QVariant &params)
{
    emit triggered(params);
}

void KoAction::doActionUi(const QVariant &params)
{
    emit updateUi(params);
}

int KoAction::jobCount()
{
    return d->jobsQueue->count();
}

void KoAction::setWeaver(ThreadWeaver::WeaverInterface *weaver)
{
    d->weaver = weaver;
}

ThreadWeaver::WeaverInterface *KoAction::weaver() const
{
    return d->weaver;
}

void KoAction::setExecutePolicy(KoExecutePolicy *policy)
{
    d->policy = policy;
}

void KoAction::setEnabled(bool enabled)
{
    d->enabled = enabled;
}

bool KoAction::isEnabled() const
{
    return d->enabled;
}

#include "KoAction.moc"
