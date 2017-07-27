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
#ifndef KISUSERFEEDBACK_ACTIONSINFOSOURCE_H
#define KISUSERFEEDBACK_ACTIONSINFOSOURCE_H

#include "QSharedPointer"
#include "abstractdatasource.h"
#include "kis_tickets.h"
#include "kuserfeedbackcore_export.h"
#include <QMap>
#include <QVariant>

namespace KisUserFeedback {

/*! Data source reporting about actions.
 *
 *  The default telemetry mode for this source is Provider::DetailedSystemInformation.
 */
class ActionsInfoSource : public KUserFeedback::AbstractDataSource {
public:
    ActionsInfoSource();
    QString description() const override;
    QVariant data() override;
    void insert(QSharedPointer<KisTicket> ticket);
    void clear();

private:
    struct actionInfo {
        QSharedPointer<KisTicket> ticket;
        int mutable countUse;
        void plusCount() const { countUse++; }
    };

private:
    QVariantList m_actionsInfo;
    QMap<QString, actionInfo> m_actionsInfoMap;
};
}

#endif // KISUSERFEEDBACK_ACTIONSINFOSOURCE_H
