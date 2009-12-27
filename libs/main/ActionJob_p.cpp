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

#include "ActionJob_p.h"

#include "KoAction.h"

#include <QCoreApplication>
#include <QEvent>
#include <QThread>
#include <KDebug>

class ActionJobEvent : public QEvent
{
public:
    ActionJobEvent() : QEvent(QEvent::User) {}
};

ActionJob::ActionJob(KoAction *parent, Enable enable, const QVariant &params)
        : Job(0), // don't pass a parent since QObject refuses to work when you pass a parent thats in a different thread
        m_action(parent),
        m_enable(enable),
        m_started(false),
        m_params(params)
{
    connect(this, SIGNAL(done(ThreadWeaver::Job*)), this, SLOT(deleteLater()), Qt::DirectConnection);
    moveToThread(QCoreApplication::instance()->thread());
}

void ActionJob::run()
{
    m_started = true;
    if (m_action.isNull())
        return;
    m_action->doAction(m_params);
    if (m_action.isNull())
        return;
    switch (m_enable) {
    case EnableOn:
        m_action->setEnabled(true);
        break;
    case EnableOff:
        m_action->setEnabled(false);
        break;
    case EnableNoChange:
        break;
    }
    if (QThread::currentThread() == QCoreApplication::instance()->thread())
        m_action->doActionUi(m_params);
    else {
        // do it in the main thread.
        m_mutex.lock();
        QCoreApplication::postEvent(this, new ActionJobEvent());
        m_waiter.wait(&m_mutex);
        m_mutex.unlock();
    }
}

bool ActionJob::event(QEvent *e)
{
    ActionJobEvent *event = dynamic_cast<ActionJobEvent*>(e);
    if (event) {
        if (! m_action.isNull())
            m_action->doActionUi(m_params);
        m_mutex.lock(); // wait for the above to start waiting on us.
        m_waiter.wakeAll();
        m_mutex.unlock();
        return true;
    }
    return QObject::event(e);
}

#include <ActionJob_p.moc>
