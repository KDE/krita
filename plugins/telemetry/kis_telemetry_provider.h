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

#ifndef KIS_TELEMETRY_REGULAR_PROVIDER_H
#define KIS_TELEMETRY_REGULAR_PROVIDER_H
#include "QScopedPointer"
#include <KUserFeedback/AbstractDataSource>
#include <KUserFeedback/CompilerInfoSource>
#include <KUserFeedback/LocaleInfoSource>
#include <KUserFeedback/OpenGLInfoSource>
#include <KUserFeedback/PlatformInfoSource>
#include <KUserFeedback/QtVersionSource>
#include <KUserFeedback/QtVersionSource>
#include <KUserFeedback/ScreenInfoSource>
#include <KUserFeedback/provider.h>
#include <QVariant>

#include "kis_cpuinfosource.h"

#include "kis_telemetry_actions.h"
#include "kis_tickets.h"
#include <QMultiMap>
#include <QVector>
#include <QWeakPointer>
#include <kis_telemetry_abstruct.h>
#include <memory>

class KRITAFLAKE_EXPORT KisTelemetryProvider : public KisTelemetryAbstruct {
public:
    KisTelemetryProvider();
    void sendData(QString path, QString adress = QString()) override;

    virtual ~KisTelemetryProvider();

protected:
    void getTimeTicket(QString id) override;
    void putTimeTicket(QString id) override;

private:
    enum TelemetryCategory {
        tools,
        install
    };

private:
    QScopedPointer<KUserFeedback::Provider> m_toolsProvider;
    std::vector<std::unique_ptr<KUserFeedback::AbstractDataSource> > m_toolSources;
    QScopedPointer<KUserFeedback::Provider> m_installProvider;
    std::vector<std::unique_ptr<KUserFeedback::AbstractDataSource> > m_installSources;
    QMultiMap<QString, QWeakPointer<KisTicket> > m_tickets;

private:
    TelemetryCategory pathToKind(QString path);
};

#endif
