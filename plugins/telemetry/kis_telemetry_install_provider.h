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

#ifndef KIS_TELEMETRY_INSTALL_PROVIDER_H
#define KIS_TELEMETRY_INSTALL_PROVIDER_H
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

#include <kis_telemetry_abstruct.h>
#include <memory>
#include <vector>

class KisTelemetryInstallProvider : public KisTelemetryAbstruct {
public:
    KisTelemetryInstallProvider();
    KUserFeedback::Provider* provider() override;
    void sendData() override;
    void getTimeTicket(QString id, UseMode mode = Activate) override {}
    void putTimeTicket(QString id, UseMode mode = Activate) override {}
    virtual ~KisTelemetryInstallProvider();

private:
    QScopedPointer<KUserFeedback::Provider> m_provider;
    std::vector<std::unique_ptr<KUserFeedback::AbstractDataSource> > m_sources;
};

#endif
