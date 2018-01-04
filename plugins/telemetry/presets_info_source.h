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
* */

#ifndef KISUSERFEEDBACK_PRESETSSOURCE_H
#define KISUSERFEEDBACK_PRESETSSOURCE_H

#include "abstractdatasource.h"
#include "kuserfeedbackcore_export.h"
#include "kis_telemetry_tickets.h"
#include <QMap>
#include <QVariant>
#include <QSharedPointer>


namespace UserFeedback {

/*! Data source reporting about presets
 */
class TelemetryPresetsSource : public KUserFeedback::AbstractDataSource {
public:
    TelemetryPresetsSource();
    QString description() const override;
    QVariant data() override;


private:
    QVariantList m_imageDumps;
    QMap<QString, QSharedPointer<KisTelemetryTicket> > m_imagesDumpsMap;
};
}

#endif // KISUSERFEEDBACK_PRESETSSOURCE_H
