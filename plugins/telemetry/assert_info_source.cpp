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
#include "assert_info_source.h"
#include <QVariantMap>
#include <kconfig.h>
#include <kconfiggroup.h>
#include <ksharedconfig.h>
#include <QDebug>
using namespace UserFeedback;
using namespace KUserFeedback;

TelemetryAssertInfoSource::TelemetryAssertInfoSource()
    : AbstractDataSource(QStringLiteral("asserts"), Provider::DetailedSystemInformation)

{
}

QString UserFeedback::TelemetryAssertInfoSource::description() const
{
    return QObject::tr("The information about usual asserts");
}

QVariant UserFeedback::TelemetryAssertInfoSource::data()
{
    qDebug()<<"TelemetryAssertInfoSource";
    m_assertsDumps.clear();
    foreach (assertInfo assert, m_assertsDumpsMap) {
        KisTelemetryTicket* ticket = assert.ticket.data();

        KisAssertInfoTicket* assertTicket = nullptr;

        assertTicket = dynamic_cast<KisAssertInfoTicket*>(ticket);
        if (assertTicket) {
            QVariantMap m;
            m.insert(QStringLiteral("assertText"), assertTicket->assertInfo().assertText);
            qDebug()<<assertTicket->assertInfo().assertText;
            m.insert(QStringLiteral("assertFile"), assertTicket->assertInfo().file);

            m.insert(QStringLiteral("assertLine"), assertTicket->assertInfo().line);
            m.insert(QStringLiteral("count"), assert.count);
            m.insert(QStringLiteral("isFatal"), false);

            m_assertsDumps.push_back(m);
        }
    }
    m_assertsDumpsMap.clear();

    return m_assertsDumps;
}

void TelemetryAssertInfoSource::removeAssert(QString id)
{
    m_assertsDumpsMap.remove(id);
}

void TelemetryAssertInfoSource::addCounter(QString id)
{
}

void TelemetryAssertInfoSource::insert(QSharedPointer<KisTelemetryTicket> ticket)
{
    if (m_assertsDumpsMap.count(ticket->ticketId())) {
        int count = m_assertsDumpsMap.value(ticket->ticketId()).count;
        m_assertsDumpsMap.insert(ticket->ticketId(), { ticket, ++count });
    } else {
        m_assertsDumpsMap.insert(ticket->ticketId(), { ticket, 1 });
    }
}

int TelemetryAssertInfoSource::count(QString id)
{
    return 0;
}
