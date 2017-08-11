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
*
#ifndef KISUSERFEEDBACK_ASSERTINFOSOURCE_H
#define KISUSERFEEDBACK_ASSERTINFOSOURCE_H

#include "abstractdatasource.h"
#include "kuserfeedbackcore_export.h"
#include <exception>

namespace KisUserFeedback {

/*! Data source reporting the assert info
 */

#include <QSize>
#include <image_properties_source.h>
#include <QDebug>
using namespace UserFeedback;
using namespace KUserFeedback;

TelemetryImagePropertiesSource::TelemetryImagePropertiesSource()
    : AbstractDataSource(QStringLiteral("Images"), Provider::DetailedSystemInformation)
{
}

QString TelemetryImagePropertiesSource::description() const
{
    return QObject::tr("The informtion about images");
}

QVariant TelemetryImagePropertiesSource::data()
{
    m_imageDumps.clear();
    foreach (QSharedPointer<KisTelemetryTicket> imageDump, m_imagesDumpsMap) {
        KisImagePropertiesTicket* imagePropertiesTicket = nullptr;

        imagePropertiesTicket = dynamic_cast<KisImagePropertiesTicket*>(imageDump.data());
        if (imagePropertiesTicket) {
            QVariantMap m;
            m.insert(QStringLiteral("width"), imagePropertiesTicket->size().width());
            m.insert(QStringLiteral("height"), imagePropertiesTicket->size().height());
            m.insert(QStringLiteral("size"), imagePropertiesTicket->getImageSize());
            m.insert(QStringLiteral("colorProfile"), imagePropertiesTicket->getColorProfile());
            m.insert(QStringLiteral("colorSpace"), imagePropertiesTicket->getColorSpace());
            m.insert(QStringLiteral("numLayers"), imagePropertiesTicket->getNumLayers());

            m_imageDumps.push_back(m);
        }
    }
    m_imagesDumpsMap.clear();


    return m_imageDumps;
}

void TelemetryImagePropertiesSource::removeDumpProperties(QString id)
{
    m_imagesDumpsMap.remove(id);
}

void TelemetryImagePropertiesSource::createNewImageProperties(QSharedPointer<KisTelemetryTicket> ticket)
{
    m_imagesDumpsMap.insert(ticket->ticketId(), ticket);
}
