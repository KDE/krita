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

#ifndef KISUSERFEEDBACK_TOOLSINFOSOURCE_H
#define KISUSERFEEDBACK_TOOLSINFOSOURCE_H

#include "abstractdatasource.h"
#include "kuserfeedbackcore_export.h"
#include <QMap>
#include <QMutex>
#include <QPair>
#include <QSharedPointer>
#include <QDateTime>
#include <QVariantMap>
#include <QVector>
#include "kis_telemetry_abstruct.h"
#include "kis_tickets.h"


namespace KisUserFeedback {

/*! Data source reporting the type and amount of CPUs.
 *
 *  The default telemetry mode for this source is Provider::DetailedSystemInformation.
 */
class ToolsInfoSource : public KUserFeedback::AbstractDataSource {
public:
    ToolsInfoSource();
    QString description() const override;
    QVariant data() override;
    void activateTool(QSharedPointer<KisTicket> ticket);
    void deactivateTool(QString id);

private:
    QVariantList m_tools;
    QMap<QString,QSharedPointer<KisTicket>> m_toolsMap;
    QMap<QString, QSharedPointer<KisTicket> > m_currentTools;
    QMutex m_mutex;
};
}

#endif // KISUSERFEEDBACK_TOOLSINFOSOURCE_H
