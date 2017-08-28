/* This file is part of the KDE project
   Copyright (C) 2017 Alexey Kapustin <akapust1n@mail.ru>


   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
*/
#include "actions_info_source.h"
#include <QDebug>
using namespace UserFeedback;
using namespace KUserFeedback;

UserFeedback::TelemetryActionsInfoSource::TelemetryActionsInfoSource()
    : AbstractDataSource(QStringLiteral("actions"), Provider::DetailedSystemInformation)
{
}

QString UserFeedback::TelemetryActionsInfoSource::description() const
{
    return QObject::tr("The number of used actions and their sources");
}

QVariant UserFeedback::TelemetryActionsInfoSource::data()
{
    m_actionsInfo.clear();
    foreach (actionInfo action, m_actionsInfoMap) {
        KisTelemetryTicket* ticket = action.ticket.data();
        KisActionInfoTicket* actionTicket = nullptr;

        actionTicket = dynamic_cast<KisActionInfoTicket*>(ticket);
        if (actionTicket) {

            QVariantMap m;
            m.insert(QStringLiteral("actionName"), ticket->ticketId());
            m.insert(QStringLiteral("sources"), actionTicket->actionInfo().source);
            m.insert(QStringLiteral("countUse"), action.countUse);
            m_actionsInfo.push_back(m);
        }
    }
    m_actionsInfoMap.clear();

    return m_actionsInfo;
}

void TelemetryActionsInfoSource::insert(QSharedPointer<KisTelemetryTicket> ticket)
{
    if (m_actionsInfoMap.count(ticket->ticketId())) {
        int countUse = m_actionsInfoMap.value(ticket->ticketId()).countUse;
        m_actionsInfoMap.insert(ticket->ticketId(), { ticket, ++countUse });
    } else {
        m_actionsInfoMap.insert(ticket->ticketId(), { ticket, 1 });
    }
}

void TelemetryActionsInfoSource::clear()
{
    m_actionsInfoMap.clear();
    m_actionsInfo.clear();
}
