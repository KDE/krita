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

#include "kis_tickets.h"
#include "kis_toolsinfosource.h"
#include <KoToolRegistry.h>
#include <iostream>
#include <kis_global.h>
#include <kis_types.h>

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

void KisTelemetryRegularProvider::getTimeTicket(QString id,UseMode mode)
{
    qDebug()<<"get TIme ticket call";

    id = getToolId(id, mode);
    qDebug()<<ppVar(m_tickets.count(id));

    KisTicket* ticket = m_tickets.value(id).lock().data();
    KisTimeTicket* timeTicket;
    KUserFeedback::AbstractDataSource* m_tools = m_sources[0].get();
    KisUserFeedback::ToolsInfoSource* tools = nullptr;

    timeTicket = dynamic_cast<KisTimeTicket*>(ticket);
    if (!ticket) {
        Q_ASSERT_X(1 == 0, "timeTicket is lost", id.);
        return;
    }
    qDebug()<<"timeTicket is not lost";
    tools = dynamic_cast<KisUserFeedback::ToolsInfoSource*>(m_tools);
    if (!timeTicket || !tools) {
        Q_ASSERT_X(1 == 0, "get tool's timeTicket ", id.toStdString().c_str());
        return;
    }
    qDebug()<<"Before deactivate";
    tools->deactivateTool(id);
    m_tickets.remove(id);
}

void KisTelemetryRegularProvider::putTimeTicket(QString id, UseMode mode)
{
    id = getToolId(id, mode);
    if(m_tickets.count(id)){
        return;
    }
    KUserFeedback::AbstractDataSource* m_tools = m_sources[0].get();
    KisUserFeedback::ToolsInfoSource* tools = nullptr;

    QSharedPointer<KisTicket> timeTicket;
    timeTicket.reset(new KisTimeTicket(id));

    tools = dynamic_cast<KisUserFeedback::ToolsInfoSource*>(m_tools);

    if (!tools) {
        Q_ASSERT_X(1 == 0, "create tool's timeTicket ", id);
        return;
    }
    QWeakPointer<KisTicket> weakTimeTicket(timeTicket);

    m_tickets.insert(id, weakTimeTicket);
    tools->activateTool(timeTicket);
}

KisTelemetryRegularProvider::~KisTelemetryRegularProvider()
{
}
