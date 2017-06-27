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

#include "kis_toolsinfosource.h"
#include <KoToolRegistry.h>
#include <iostream>
#include <kis_global.h>
#include <kis_types.h>
#include <tuple>

KisTelemetryRegularProvider::KisTelemetryRegularProvider()
{
    m_provider.reset(new KUserFeedback::Provider);
    m_provider.data()->setTelemetryMode(KUserFeedback::Provider::DetailedUsageStatistics);

    std::unique_ptr<KUserFeedback::AbstractDataSource> tools(new KisUserFeedback::ToolsInfoSource);
    m_sources.push_back(std::move(tools));

    for (auto& source : m_sources) {
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

void KisTelemetryRegularProvider::storeData(QVector<QString>& args)
{
    QString whatIsIt = args[0];
    QString toolName = args[1];

    KUserFeedback::AbstractDataSource* m_tools = m_sources[0].get();
    KisUserFeedback::ToolsInfoSource* tools = nullptr;

    try {
        tools = dynamic_cast<KisUserFeedback::ToolsInfoSource*>(m_tools);
    } catch (...) {
        return;
    }

    if (whatIsIt == QString("Activate")) {
        tools->activateTool(toolName);
    } else {
        tools->deactivateTool(toolName);
    }
}
