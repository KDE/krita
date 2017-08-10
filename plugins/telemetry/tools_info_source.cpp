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

#include "tools_info_source.h"

#include <QMutexLocker>
#include <QSysInfo>
#include <QThread>
#include <QVariant>
#include <cmath>

using namespace UserFeedback;
using namespace KUserFeedback;

TelemetryToolsInfoSource::TelemetryToolsInfoSource()
    : AbstractDataSource(QStringLiteral("Tools"), Provider::DetailedSystemInformation)
{
}

QString TelemetryToolsInfoSource::description() const
{
    return QObject::tr("Inforamation about tools");
}

QVariant TelemetryToolsInfoSource::data()
{
    static bool firstCall = false;//kuserfeedback feature
    firstCall = !firstCall;
    if (firstCall) {
        m_tools.clear();
        return QVariant() ;
    }
    foreach (toolInfo tool, m_toolsMap) {
        KisTelemetryTicket* ticket = tool.ticket.data();
        KisTimeTicket* timeTicket = nullptr;

        timeTicket = dynamic_cast<KisTimeTicket*>(ticket);
        if (timeTicket) {
            int timeUse = timeTicket->useTimeMSeconds();
            if (!timeUse) {
                continue;
            }
            QVariantMap m;
            m.insert(QStringLiteral("toolName"), ticket->ticketId());
            m.insert(QStringLiteral("timeUseMSeconds"), timeUse);
            m.insert(QStringLiteral("countUse"), tool.countUse);
            m_tools.push_back(m);
        }
    }
    m_toolsMap.clear();

    return m_tools;
}

void TelemetryToolsInfoSource::activateTool(QSharedPointer<KisTelemetryTicket> ticket)
{
    QMutexLocker locker(&m_mutex);

    m_currentTools.insert(ticket->ticketId(), ticket);
    if (!m_toolsMap.count(ticket->ticketId())) {
        m_toolsMap.insert(ticket->ticketId(), { ticket, 0 });
    }
}

void TelemetryToolsInfoSource::deactivateTool(QString id)
{
    QMutexLocker locker(&m_mutex);
    KisTelemetryTicket* ticket = m_currentTools.value(id).data();
    KisTimeTicket* timeTicket = dynamic_cast<KisTimeTicket*>(ticket);
    if (timeTicket) {
        QDateTime deactivateTime = QDateTime::currentDateTime();
        timeTicket->setEndTime(deactivateTime);

        KisTelemetryTicket* mainTicket = m_toolsMap[id].ticket.data();
        KisTimeTicket* mainTimeTicket = dynamic_cast<KisTimeTicket*>(mainTicket);

        if (mainTimeTicket) {
            m_toolsMap[id].countUse++;
            mainTimeTicket->addMSecs(timeTicket->useTimeMSeconds());
        }
    }
    m_currentTools.remove(id);
}
