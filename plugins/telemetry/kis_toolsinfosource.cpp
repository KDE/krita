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
    return m_tools;
}

void ToolsInfoSource::activateTool(QSharedPointer<KisTicket> ticket)
{
    QMutexLocker locker(&m_mutex);

    m_currentTools.insert(ticket->ticketId(), ticket);
    std::cout << "ACTIVATE TOOL " << ticket->ticketId().toStdString()<< std::endl;
}

void ToolsInfoSource::deactivateTool(QString id)
{
    QMutexLocker locker(&m_mutex);
    KisTicket* ticket = m_currentTools.value(id).data();
    KisTimeTicket *timeTicket = nullptr;

    try {
        timeTicket = dynamic_cast<KisTimeTicket*>(ticket);

    } catch (...) {
         Q_ASSERT_X(1!=0,"deactivate tool","");
         return;
    }
    QTime deactivateTime = QTime::currentTime();

    timeTicket->setEndTime(deactivateTime);
    int timeUse = timeTicket->endTime().second() - timeTicket->startTime().second();
    QVariantMap m;
    m.insert(QStringLiteral("toolname"), ticket->ticketId());
    m.insert(QStringLiteral("timeUseSeconds"), timeUse);
    std::cout<<"Time use"<<timeUse<<std::endl;
    std::cout<<"Time start"<<timeTicket->startTime().second()<<std::endl;
    std::cout<<"Time end"<<timeTicket->endTime().second()<<std::endl;



    m_tools.push_back(m);
    m_currentTools.remove(id);
    std::cout<< "DE_ACTIVATE TOOL " << ticket->ticketId().toStdString() << std::endl;
}
