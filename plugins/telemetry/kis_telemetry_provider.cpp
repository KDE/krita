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

#include "Vc/cpuid.h"
#include "kis_tickets.h"
#include "kis_toolsinfosource.h"
#include <KoToolRegistry.h>
#include <iostream>
#include <kis_assertinfosource.h>
#include <kis_global.h>
#include <kis_imagepropertiessource.h>
#include <kis_types.h>

KisTelemetryProvider::KisTelemetryProvider()
{
    m_installProvider.reset(new KUserFeedback::Provider);
    m_installProvider.data()->setTelemetryMode(KUserFeedback::Provider::DetailedUsageStatistics);

    std::unique_ptr<KUserFeedback::AbstractDataSource> cpu(new KisUserFeedback::CpuInfoSource());
    m_installSources.push_back(std::move(cpu));
    std::unique_ptr<KUserFeedback::AbstractDataSource> qt(new KUserFeedback::QtVersionSource());
    m_installSources.push_back(std::move(qt));
    std::unique_ptr<KUserFeedback::AbstractDataSource> compiler(new KUserFeedback::CompilerInfoSource());
    m_installSources.push_back(std::move(compiler));
    std::unique_ptr<KUserFeedback::AbstractDataSource> locale(new KUserFeedback::LocaleInfoSource());
    m_installSources.push_back(std::move(locale));
    std::unique_ptr<KUserFeedback::AbstractDataSource> opengl(new KUserFeedback::OpenGLInfoSource());
    m_installSources.push_back(std::move(opengl));
    std::unique_ptr<KUserFeedback::AbstractDataSource> platform(new KUserFeedback::PlatformInfoSource());
    m_installSources.push_back(std::move(platform));
    std::unique_ptr<KUserFeedback::AbstractDataSource> screen(new KUserFeedback::ScreenInfoSource());
    m_installSources.push_back(std::move(screen));

    for (auto& source : m_installSources) {
        m_installProvider.data()->addDataSource(source.get());
    }

    m_toolsProvider.reset(new KUserFeedback::Provider);
    m_toolsProvider.data()->setTelemetryMode(KUserFeedback::Provider::DetailedUsageStatistics);
    std::unique_ptr<KUserFeedback::AbstractDataSource> tools(new KisUserFeedback::ToolsInfoSource);
    m_toolSources.push_back(std::move(tools));

    for (auto& source : m_toolSources) {
        m_toolsProvider.data()->addDataSource(source.get());
    }

    m_assertsProvider.reset(new KUserFeedback::Provider);
    m_toolsProvider.data()->setTelemetryMode(KUserFeedback::Provider::DetailedUsageStatistics);
    std::unique_ptr<KUserFeedback::AbstractDataSource> asserts(new KisUserFeedback::AssertInfoSource);
    m_assertsSources.push_back(std::move(asserts));
    for (auto& source : m_assertsSources) {
        m_assertsProvider.data()->addDataSource(source.get());
    }

    m_imagePropertiesProvider.reset(new KUserFeedback::Provider);
    m_imagePropertiesProvider.data()->setTelemetryMode(KUserFeedback::Provider::DetailedUsageStatistics);
    std::unique_ptr<KUserFeedback::AbstractDataSource> imageProperties(new KisUserFeedback::ImagePropertiesSource);
    m_imagePropertiesSources.push_back(std::move(imageProperties));
    for (auto& source : m_imagePropertiesSources) {
        m_imagePropertiesProvider.data()->addDataSource(source.get());
    }
}

void KisTelemetryProvider::sendData(QString path, QString adress)
{
    if (!path.endsWith(QLatin1Char('/')))
        path += QLatin1Char('/');
    TelemetryCategory enumPath = pathToKind(path);
    QString finalAdress = adress.isNull() ? m_adress : adress;
    switch (enumPath) {
    case tools: {
        m_toolsProvider.data()->setFeedbackServer(QUrl(finalAdress + path));
        m_toolsProvider.data()->submit();
        break;
    }
    case install: {
        m_installProvider.data()->setFeedbackServer(QUrl(finalAdress + path));
        m_installProvider.data()->submit();
        break;
    }
    case asserts: {
        m_assertsProvider.data()->setFeedbackServer(QUrl(finalAdress + path));
        m_assertsProvider.data()->submit();
        break;
    }
    case imageProperties: {
        m_imagePropertiesProvider.data()->setFeedbackServer(QUrl(finalAdress + path));
        m_imagePropertiesProvider.data()->submit();
        break;
    }
    default:
        break;
    }
}

void KisTelemetryProvider::getTimeTicket(QString id)
{
    KisTicket* ticket = m_tickets.value(id).lock().data();
    if (!ticket) {
        return;
    }
    KisTimeTicket* timeTicket = nullptr;
    KUserFeedback::AbstractDataSource* m_tools = m_toolSources[0].get();
    KisUserFeedback::ToolsInfoSource* tools = nullptr;

    timeTicket = dynamic_cast<KisTimeTicket*>(ticket);
    tools = dynamic_cast<KisUserFeedback::ToolsInfoSource*>(m_tools);

    KIS_SAFE_ASSERT_RECOVER_RETURN(timeTicket);
    KIS_SAFE_ASSERT_RECOVER_RETURN(tools);
    KIS_SAFE_ASSERT_RECOVER_RETURN(ticket);

    tools->deactivateTool(id);
    m_tickets.remove(id);
}

void KisTelemetryProvider::putTimeTicket(QString id)
{
    if (m_tickets.count(id)) {
        return;
    }
    KUserFeedback::AbstractDataSource* m_tools = m_toolSources[0].get();
    KisUserFeedback::ToolsInfoSource* tools = nullptr;

    QSharedPointer<KisTicket> timeTicket;
    timeTicket.reset(new KisTimeTicket(id));

    tools = dynamic_cast<KisUserFeedback::ToolsInfoSource*>(m_tools);

    KIS_SAFE_ASSERT_RECOVER_RETURN(tools);

    QWeakPointer<KisTicket> weakTimeTicket(timeTicket);

    m_tickets.insert(id, weakTimeTicket);
    tools->activateTool(timeTicket);
}

void KisTelemetryProvider::saveImageProperites(QString fileName, KisImageSP& image)
{
    KUserFeedback::AbstractDataSource* m_imageProperties = m_imagePropertiesSources[0].get();
    KisUserFeedback::ImagePropertiesSource* imageProperties = nullptr;

    QSharedPointer<KisTicket> imagePropertiesTicket;
    imagePropertiesTicket.reset(new KisImagePropertiesTicket(image, fileName));

    imageProperties = dynamic_cast<KisUserFeedback::ImagePropertiesSource*>(m_imageProperties);

    KIS_SAFE_ASSERT_RECOVER_RETURN(imageProperties);

    if (m_tickets.count(fileName)) {
        imageProperties->removeDumpProperties(fileName);
        return;
    }

    QWeakPointer<KisTicket> weakimagePropertiesTicket(imagePropertiesTicket);

    m_tickets.insert(fileName, weakimagePropertiesTicket);
    imageProperties->createNewImageProperties(imagePropertiesTicket);
}

KisTelemetryProvider::~KisTelemetryProvider()
{
}

KisTelemetryProvider::TelemetryCategory KisTelemetryProvider::pathToKind(QString path)
{
    if (path == "tools/")
        return tools;
    else if (path == "install/")
        return install;
    else if (path == "asserts/")
        return asserts;
    else if (path == "imageProperties/")
        return imageProperties;
    return tools;
}

