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

#include "kis_telemetry_sensor.h"
#include "QDebug"

KisTelemetrySensor::KisTelemetrySensor(QString id, KisTelemetryAbstruct::Action action, KisTelemetryAbstruct::UseMode mode)
    : m_id(id)
    , m_action(action)
    , m_useMode(mode)
{
}

KisTelemetrySensor::~KisTelemetrySensor()
{
    if (!KisPart::instance()->provider(KisPart::RegularProvider)) {
       return;
    }
    if(m_action==KisTelemetryAbstruct::getTimeTicket_)
        qDebug()<<"GET TIME TICKET";
    KisTelemetryAbstruct* provider = KisPart::instance()->provider(KisPart::RegularProvider);
    switch (m_action) {
    case KisTelemetryAbstruct::getTimeTicket_:
        provider->getTimeTicket(m_id, m_useMode);
        break;
    case KisTelemetryAbstruct::putTimeTicket_:
        provider->putTimeTicket(m_id, m_useMode);
        break;
    default:
        break;
    }
}
