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

#include "kis_telemetry_regular_provider.h"
#include "KPluginFactory"
#include "KisPart.h"
#include <klocalizedstring.h>
#include <ksharedconfig.h>

#include <kis_debug.h>
#include <kpluginfactory.h>

#include <KoToolRegistry.h>
#include <iostream>
#include <kis_global.h>
#include <kis_types.h>
#include "Vc/cpuid.h"

KisTelemetryRegularProvider::KisTelemetryRegularProvider()
{
    m_provider.reset(new KUserFeedback::Provider);
    m_provider.data()->setTelemetryMode(KUserFeedback::Provider::DetailedUsageStatistics);

    for (auto &source : m_sources) {
        m_provider.data()->addDataSource(source.get());
    }
}

KUserFeedback::Provider* KisTelemetryRegularProvider::provider()
{
    return m_provider.data();
}

void KisTelemetryRegularProvider::sendData()
{
    // m_provider.data()->setFeedbackServer(QUrl("http://akapustin.me:8080/"));
    m_provider.data()->setFeedbackServer(QUrl(m_adress));
    m_provider.data()->submit();
}

KisTelemetryRegularProvider::~KisTelemetryRegularProvider()
{
}

\
