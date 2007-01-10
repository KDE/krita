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
#include "ActionJob_p.h"
#include "KoAction.h"

#include <QCoreApplication>
#include <QEvent>
#include <QThread>

class ActionJobEvent : public QEvent {
public:
    ActionJobEvent() : QEvent(QEvent::User) {}
};

ActionJob::ActionJob(KoAction *parent, Enable enable, QVariant *params)
    : Job(parent),
    m_action(parent),
    m_enable(enable),
    m_started(false),
    m_params(params)
{
}

void ActionJob::run() {
    m_started = true;
    m_action->doAction(m_params);
    switch(m_enable) {
        case EnableOn:
            m_action->setEnabled(true);
            break;
        case EnableOff:
            m_action->setEnabled(false);
            break;
        case EnableNoChange:
            break;
    }
    if(QThread::currentThread() == QCoreApplication::instance()->thread())
        m_action->doActionUi(m_params);
    else {
        // do it in the main thread.
        QCoreApplication::postEvent(this, new ActionJobEvent());
        m_semaphore.acquire();
    }
}

bool ActionJob::event(QEvent *e) {
    ActionJobEvent *event = dynamic_cast<ActionJobEvent*> (e);
    if(event) {
        m_action->doActionUi(m_params);
        m_semaphore.release();
        return true;
    }
    return QObject::event(e);
}

#include "ActionJob_p.moc"
