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

#include "kis_tickets.h"
#include <QTime>
#include <limits>

KisTimeTicket::KisTimeTicket(QString id)
    : KisTicket(id)
{
    m_start = QDateTime::currentDateTime();
}

void KisTimeTicket::setStartTime(QDateTime& time)
{
    m_start = time;
}

void KisTimeTicket::setEndTime(QDateTime& time)
{
    m_end = time;
}

QDateTime KisTimeTicket::startTime() const
{
    return m_start;
}

QDateTime KisTimeTicket::endTime() const
{
    return m_end;
}

int KisTimeTicket::useTimeMSeconds() const
{
    int timeUse =  static_cast<int>(m_end.toMSecsSinceEpoch() - m_start.toMSecsSinceEpoch());
    return timeUse;
}

void KisTimeTicket::addMSecs(int seconds)
{
    if(seconds<0){
        seconds = std::numeric_limits<int>::max();
    }
    m_end.addMSecs(seconds);
}

KisTicket::KisTicket(QString id)
    : m_id(id)
{
}

QString KisTicket::ticketId() const
{
    return m_id;
}

void KisTicket::setTickedId(QString id)
{
    m_id = id;
}
