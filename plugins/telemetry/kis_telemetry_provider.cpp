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

#include "kis_telemetry_provider.h"
#include "KPluginFactory"
#include "KisPart.h"
#include <klocalizedstring.h>
#include <ksharedconfig.h>

#include <kis_debug.h>
#include <kpluginfactory.h>

#include <kis_global.h>
#include <kis_types.h>
#include <KoToolRegistry.h>
#include <iostream>


KisTelemetryProvider::KisTelemetryProvider()
{
    m_provider.reset(new UserFeedback::Provider);
    std::unique_ptr<UserFeedback::AbstractDataSource> cpu(new UserFeedback::CpuInfoSource());
    m_sources.push_back(std::move(cpu));
    std::cout << "\n"
              << "PROVIDER LOADED" << std::endl;
    UserFeedback::AbstractDataSource *src2= new UserFeedback::QtVersionSource();
    m_provider.data()->setStatisticsCollectionMode(UserFeedback::Provider::DetailedUsageStatistics);

    m_provider.data()->addDataSource(m_sources[0].get(), UserFeedback::Provider::DetailedUsageStatistics);
    m_provider.data()->addDataSource(src2, UserFeedback::Provider::DetailedUsageStatistics);


}

UserFeedback::Provider* KisTelemetryProvider::provider()
{
    return m_provider.data();
}

void KisTelemetryProvider::sendData()
{
    m_provider.data()->setFeedbackServer(QUrl("http://akapustin.me/"));
    m_provider.data()->submit();
}

KisTelemetryProvider::~KisTelemetryProvider()
{

}


