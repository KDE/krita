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

#include <KoToolRegistry.h>
#include <iostream>
#include <kis_global.h>
#include <kis_types.h>
#include "Vc/cpuid.h"

KisTelemetryProvider::KisTelemetryProvider()
{
    m_provider.reset(new KUserFeedback::Provider);
    m_provider.data()->setTelemetryMode(KUserFeedback::Provider::DetailedUsageStatistics);

    std::unique_ptr<KUserFeedback::AbstractDataSource> cpu(new KisUserFeedback::CpuInfoSource());
    m_sources.push_back(std::move(cpu));
    std::unique_ptr<KUserFeedback::AbstractDataSource> qt(new KUserFeedback::QtVersionSource());
    m_sources.push_back(std::move(qt));
    std::unique_ptr<KUserFeedback::AbstractDataSource> compiler(new KUserFeedback::CompilerInfoSource());
    m_sources.push_back(std::move(compiler));
    std::unique_ptr<KUserFeedback::AbstractDataSource> locale(new KUserFeedback::LocaleInfoSource());
    m_sources.push_back(std::move(locale));
    std::unique_ptr<KUserFeedback::AbstractDataSource> opengl(new KUserFeedback::OpenGLInfoSource());
    m_sources.push_back(std::move(opengl));
    std::unique_ptr<KUserFeedback::AbstractDataSource> platform(new KUserFeedback::PlatformInfoSource());
    m_sources.push_back(std::move(platform));
    std::unique_ptr<KUserFeedback::AbstractDataSource> screen(new KUserFeedback::ScreenInfoSource());
    m_sources.push_back(std::move(screen));

    for (auto &source : m_sources) {
        m_provider.data()->addDataSource(source.get());
    }
}

KUserFeedback::Provider* KisTelemetryProvider::provider()
{
    return m_provider.data();
}

void KisTelemetryProvider::sendData()
{
    // m_provider.data()->setFeedbackServer(QUrl("http://akapustin.me:8080/"));
    m_provider.data()->setFeedbackServer(QUrl("http://localhost:8080/"));
    m_provider.data()->submit();
}

KisTelemetryProvider::~KisTelemetryProvider()
{
}
