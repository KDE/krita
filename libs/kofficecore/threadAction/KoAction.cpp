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
#include "KoAction.h"
#include "KoExecutePolicy.h"
#include "ActionJob_p.h"

#include <QVariant>

KoAction::KoAction(QObject *parent)
    : QObject(parent),
    m_policy(KoExecutePolicy::simpleQueuedPolicy),
    m_weaver(0),
    m_enabled(true)
{
}

void KoAction::execute() {
    execute(0);
}

void KoAction::execute(QVariant *params) {
    if(!m_enabled)
        return;
    Q_ASSERT(m_weaver);
    m_policy->schedule(this, &m_jobsQueue, params);
}

void KoAction::doAction(QVariant *params) {
    if(params)
        emit triggered(*params);
    else {
        QVariant variant(0);
        emit triggered(&variant);
    }
}

void KoAction::doActionUi(QVariant *params) {
    if(params)
        emit updateUi(*params);
    else {
        QVariant variant(0);
        emit updateUi(&variant);
    }
}

#include "KoAction.moc"
