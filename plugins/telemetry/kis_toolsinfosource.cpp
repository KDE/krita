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

#include "kis_toolsinfosource.h"

#include <QMutexLocker>
#include <QSysInfo>
#include <QThread>
#include <QVariant>
#include <cmath>
#include <iostream>

using namespace KisUserFeedback;
using namespace KUserFeedback;

ToolsInfoSource::ToolsInfoSource()
    : AbstractDataSource(QStringLiteral("Tools"), Provider::DetailedSystemInformation)
{
}

QString ToolsInfoSource::description() const
{
    return QObject::tr("Inforamation about tools");
}

QVariant ToolsInfoSource::data()
{
    static int countCalls = 0;
    countCalls++;
    if (!countCalls % 2) { //kuserfeedback feature
        m_tools.clear();
    }
    foreach (QString id, m_currentTools.keys()) {
        deactivateTool(id);
    }
    foreach (QSharedPointer<KisTicket> tool, m_toolsMap) {
        KisTicket* ticket = tool.data();
        KisTimeTicket* timeTicket = nullptr;

        timeTicket = dynamic_cast<KisTimeTicket*>(ticket);
        if (timeTicket) {
            int timeUse = timeTicket->useTimeMSeconds();
            QVariantMap m;
            m.insert(QStringLiteral("toolname"), ticket->ticketId());
            m.insert(QStringLiteral("timeUseSeconds"), timeUse);
            std::cout << "Time use" << timeUse << std::endl;
            m_tools.push_back(m);
        }
    }
    m_toolsMap.clear();
    m_currentTools.clear();

    return m_tools;
}

void ToolsInfoSource::activateTool(QSharedPointer<KisTicket> ticket)
{
    QMutexLocker locker(&m_mutex);

    m_currentTools.insert(ticket->ticketId(), ticket);
    if (!m_toolsMap.count(ticket->ticketId()))
        m_toolsMap.insert(ticket->ticketId(), ticket);
    std::cout << "ACTIVATE TOOL " << ticket->ticketId().toStdString() << std::endl;
}

void ToolsInfoSource::deactivateTool(QString id)
{
    QMutexLocker locker(&m_mutex);
    KisTicket* ticket = m_currentTools.value(id).data();
    KisTimeTicket* timeTicket = dynamic_cast<KisTimeTicket*>(ticket);
    std::cout << "SIZES current:" << m_currentTools.size() << " all" << m_toolsMap.size() << std::endl;
    if (timeTicket) {
        QDateTime deactivateTime = QDateTime::currentDateTime();
        timeTicket->setEndTime(deactivateTime);

        KisTicket* mainTicket = m_toolsMap[id].data();
        KisTimeTicket* mainTimeTicket = dynamic_cast<KisTimeTicket*>(mainTicket);

        if (mainTimeTicket) {
            std::cout << "AdditionalTIme " << timeTicket->useTimeMSeconds() << std::endl;
            mainTimeTicket->addMSecs(timeTicket->useTimeMSeconds());
        }
        std::cout << "DE_ACTIVATE TOOL " << ticket->ticketId().toStdString() << std::endl;
    }
    m_currentTools.remove(id);
}
