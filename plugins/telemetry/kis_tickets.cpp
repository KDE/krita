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
#include "kis_telemetry_actions.h"
#include <KoColorProfile.h>
#include <KoColorSpace.h>
#include <QFileInfo>
#include <QTime>
#include <kis_image.h>
#include <limits>

KisTimeTicket::KisTimeTicket(QString id)
    : KisTicket(id)
{
    m_start = QDateTime::currentDateTime();
    m_end = m_start;
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
    int timeUse = static_cast<int>(m_end.toMSecsSinceEpoch() - m_start.toMSecsSinceEpoch());
    return timeUse;
}

void KisTimeTicket::addMSecs(int seconds)
{
    if (seconds < 0) {
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

KisImagePropertiesTicket::KisImagePropertiesTicket(KisSaveImageProperties::ImageInfo imageInfo, QString id)
    : m_imageInfo(imageInfo)
    , m_fileInfo(id)
{
}

QSize KisImagePropertiesTicket::size() const
{
    return m_imageInfo.size;
}

int KisImagePropertiesTicket::getNumLayers() const
{
    return m_imageInfo.numLayers;
}

QString KisImagePropertiesTicket::getFileFormat() const
{
    return m_fileInfo.completeSuffix();
}

QString KisImagePropertiesTicket::getColorSpace() const
{
    return m_imageInfo.colorSpace;
}

qint64 KisImagePropertiesTicket::getImageSize() const
{
    return m_fileInfo.size();
}

QString KisImagePropertiesTicket::getColorProfile() const
{
    return m_imageInfo.colorProfile;
}

KisActionInfoTicket::KisActionInfoTicket(KisSaveActionInfo::ActionInfo actionInfo, QString id)
    : KisTicket(id)
    , m_actionInfo(actionInfo)
{
}

KisSaveActionInfo::ActionInfo KisActionInfoTicket::actionInfo() const
{
    return m_actionInfo;
}
