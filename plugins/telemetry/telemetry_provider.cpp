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

#include "telemetry_provider.h"
#include "KPluginFactory"
#include "KisPart.h"
#include <klocalizedstring.h>
#include <ksharedconfig.h>

#include <kis_debug.h>
#include <kpluginfactory.h>

#include "Vc/cpuid.h"
#include "actions_info_source.h"
#include "general_info_source.h"
#include "tools_info_source.h"
#include <KoToolRegistry.h>
#include <QTime>
#include <assert_info_source.h>
#include <config-debug.h>
#include <fatal_assert_info_source.h>
#include <image_properties_source.h>
#include <iostream>
#include <kconfig.h>
#include <kconfiggroup.h>
#include <kis_global.h>
#include <kis_types.h>
#include <ksharedconfig.h>

TelemetryProvider::TelemetryProvider()
{
    m_installProvider.reset(new KUserFeedback::Provider);
    m_installProvider->setTelemetryMode(KUserFeedback::Provider::DetailedUsageStatistics);
    KConfigGroup configGroup = KSharedConfig::openConfig()->group("KisTelemetryInstall");

    QString isFirstStart = configGroup.readEntry("FirstStart");
    QString version = configGroup.readEntry("Version", "noversion");
    m_sendInstallInfo = false;
    qDebug() << "APP VERSION______" << qApp->applicationVersion();
   // if (qApp->applicationVersion() != version) {
        configGroup.writeEntry("Version", qApp->applicationVersion());
        qDebug() << "SEND INSTALL__"<<version;
//#ifndef HAVE_BACKTRACE
//        if (isFirstStart.isEmpty()) {
//#endif
            //don't append install source
            std::unique_ptr<KUserFeedback::AbstractDataSource> cpu(new UserFeedback::TelemetryCpuInfoSource());
            m_installSources.push_back(std::move(cpu));
            std::unique_ptr<KUserFeedback::AbstractDataSource> qt(new KUserFeedback::QtVersionSource());
            m_installSources.push_back(std::move(qt));
            std::unique_ptr<KUserFeedback::AbstractDataSource> compiler(new KUserFeedback::CompilerInfoSource());
            m_installSources.push_back(std::move(compiler));
            std::unique_ptr<KUserFeedback::AbstractDataSource> opengl(new KUserFeedback::OpenGLInfoSource());
            m_installSources.push_back(std::move(opengl));
            std::unique_ptr<KUserFeedback::AbstractDataSource> platform(new KUserFeedback::PlatformInfoSource());
            m_installSources.push_back(std::move(platform));
            std::unique_ptr<KUserFeedback::AbstractDataSource> screen(new KUserFeedback::ScreenInfoSource());
            m_installSources.push_back(std::move(screen));
            //#ifdef HAVE_BACKTRACE
            std::unique_ptr<KUserFeedback::AbstractDataSource> general(new UserFeedback::TelemetryGeneralInfoSource);
            //#endif
            m_installSources.push_back(std::move(general));
            for (auto& source : m_installSources) {
                m_installProvider->addDataSource(source.get());
            }
            m_sendInstallInfo = true;
//#ifndef HAVE_BACKTRACE
//        }
//#endif
   // }

    m_toolsProvider.reset(new KUserFeedback::Provider);
    m_toolsProvider.data()->setTelemetryMode(KUserFeedback::Provider::DetailedUsageStatistics);
    std::unique_ptr<KUserFeedback::AbstractDataSource> tools(new UserFeedback::TelemetryToolsInfoSource);
    m_toolSources.push_back(std::move(tools));

    for (auto& source : m_toolSources) {
        m_toolsProvider->addDataSource(source.get());
    }

    m_fatalAssertsProvider.reset(new KUserFeedback::Provider);
    m_toolsProvider->setTelemetryMode(KUserFeedback::Provider::DetailedUsageStatistics);
    std::unique_ptr<KUserFeedback::AbstractDataSource> asserts(new UserFeedback::TelemetryFatalAssertInfoSource);
    m_fatalAssertsSources.push_back(std::move(asserts));
    for (auto& source : m_fatalAssertsSources) {
        m_fatalAssertsProvider->addDataSource(source.get());
    }

    m_imagePropertiesProvider.reset(new KUserFeedback::Provider);
    m_imagePropertiesProvider->setTelemetryMode(KUserFeedback::Provider::DetailedUsageStatistics);
    std::unique_ptr<KUserFeedback::AbstractDataSource> imageProperties(new UserFeedback::TelemetryImagePropertiesSource);
    m_imagePropertiesSources.push_back(std::move(imageProperties));
    for (auto& source : m_imagePropertiesSources) {
        m_imagePropertiesProvider.data()->addDataSource(source.get());
    }

    m_actionsInfoProvider.reset(new KUserFeedback::Provider);
    m_actionsInfoProvider->setTelemetryMode(KUserFeedback::Provider::DetailedUsageStatistics);
    std::unique_ptr<KUserFeedback::AbstractDataSource> actionsInfo(new UserFeedback::TelemetryActionsInfoSource);
    m_actionsSources.push_back(std::move(actionsInfo));
    for (auto& source : m_actionsSources) {
        m_actionsInfoProvider->addDataSource(source.get());
    }
    //asserts
    m_assertsProvider.reset(new KUserFeedback::Provider);
    m_assertsProvider->setTelemetryMode(KUserFeedback::Provider::DetailedUsageStatistics);
    std::unique_ptr<KUserFeedback::AbstractDataSource> assertsInfo(new UserFeedback::
            TelemetryAssertInfoSource);
    m_assertsSources.push_back(std::move(assertsInfo));
    for (auto& source : m_assertsSources) {
        m_assertsProvider->addDataSource(source.get());
    }
}

void TelemetryProvider::sendData(QString path, QString adress)
{
    if (!path.endsWith(QLatin1Char('/')))
        path += QLatin1Char('/');
    TelemetryCategory enumPath = pathToKind(path);
    QString finalAdress = adress.isNull() ? m_adress : adress;
    switch (enumPath) {
    case tools: {
        m_toolsProvider->setFeedbackServer(QUrl(finalAdress + path));
        m_toolsProvider->submit();
        break;
    }
    case install: {
      //  if (m_sendInstallInfo) {
            m_installProvider->setFeedbackServer(QUrl(finalAdress + path));
            m_installProvider->submit();
            m_sendInstallInfo = false;
            break;
     //   }
    }
    case asserts: {
        m_assertsProvider->setFeedbackServer(QUrl(finalAdress + path));
        m_assertsProvider->submit();
        break;
    }
    case fatalAsserts: {
        m_fatalAssertsProvider->setFeedbackServer(QUrl(finalAdress + path));
        m_fatalAssertsProvider->submit();
        break;
    }
    case imageProperties: {
        m_imagePropertiesProvider->setFeedbackServer(QUrl(finalAdress + path));
        m_imagePropertiesProvider->submit();

        break;
    }
    case actions: {
        qDebug() << "Send actions!";
        m_actionsInfoProvider->setFeedbackServer(QUrl(finalAdress + path));
        m_actionsInfoProvider->submit();
        break;
    }
    default:
        break;
    }
}

void TelemetryProvider::getTimeTicket(QString id)
{
    KisTelemetryTicket* ticket = m_tickets.value(id).lock().data();
    if (!ticket) {
        return;
    }
    KisTimeTicket* timeTicket = nullptr;
    KUserFeedback::AbstractDataSource* m_tools = m_toolSources[0].get();
    UserFeedback::TelemetryToolsInfoSource* tools = nullptr;

    timeTicket = dynamic_cast<KisTimeTicket*>(ticket);
    tools = dynamic_cast<UserFeedback::TelemetryToolsInfoSource*>(m_tools);

    KIS_SAFE_ASSERT_RECOVER_RETURN(timeTicket);
    KIS_SAFE_ASSERT_RECOVER_RETURN(tools);
    KIS_SAFE_ASSERT_RECOVER_RETURN(ticket);

    tools->deactivateTool(id);
    m_tickets.remove(id);
}

void TelemetryProvider::putTimeTicket(QString id)
{
    if (m_tickets.count(id)) {
        return;
    }
    KUserFeedback::AbstractDataSource* m_tools = m_toolSources[0].get();
    UserFeedback::TelemetryToolsInfoSource* tools = nullptr;

    QSharedPointer<KisTelemetryTicket> timeTicket = QSharedPointer<KisTimeTicket>::create(id);

    tools = dynamic_cast<UserFeedback::TelemetryToolsInfoSource*>(m_tools);

    KIS_SAFE_ASSERT_RECOVER_RETURN(tools);

    QWeakPointer<KisTelemetryTicket> weakTimeTicket(timeTicket);

    m_tickets.insert(id, weakTimeTicket);
    tools->activateTool(timeTicket);
}

void TelemetryProvider::saveImageProperites(QString fileName, KisImagePropertiesTicket::ImageInfo imageInfo)
{
    KUserFeedback::AbstractDataSource* m_imageProperties = m_imagePropertiesSources[0].get();
    UserFeedback::TelemetryImagePropertiesSource* imageProperties = nullptr;

    QSharedPointer<KisTelemetryTicket> imagePropertiesTicket = QSharedPointer<KisImagePropertiesTicket>::create(imageInfo, fileName);

    imageProperties = dynamic_cast<UserFeedback::TelemetryImagePropertiesSource*>(m_imageProperties);

    KIS_SAFE_ASSERT_RECOVER_RETURN(imageProperties);

    if (m_tickets.count(fileName)) {
        imageProperties->removeDumpProperties(fileName);
        return;
    }

    QWeakPointer<KisTelemetryTicket> weakimagePropertiesTicket(imagePropertiesTicket);

    m_tickets.insert(fileName, weakimagePropertiesTicket);
    imageProperties->createNewImageProperties(imagePropertiesTicket);
}

void TelemetryProvider::saveActionInfo(QString id, KisActionInfoTicket::ActionInfo actionInfo)
{
    static QString lastAction = "start name";
    static QTime lastTime = QTime(0, 0, 0, 0);
    bool isSameAction = false;
    if (lastAction != actionInfo.name || QTime::currentTime().secsTo(lastTime)) {
        isSameAction = false;
    } else {
        isSameAction = true;
    }
    if (!isSameAction) {
        KUserFeedback::AbstractDataSource* m_actionsInfo = m_actionsSources[0].get();
        UserFeedback::TelemetryActionsInfoSource* actionsInfoSource = nullptr;

        QSharedPointer<KisTelemetryTicket> actionsInfoTicket = QSharedPointer<KisActionInfoTicket>::create(actionInfo, id);

        actionsInfoSource = dynamic_cast<UserFeedback::TelemetryActionsInfoSource*>(m_actionsInfo);

        KIS_SAFE_ASSERT_RECOVER_RETURN(actionsInfoSource);

        QWeakPointer<KisTelemetryTicket> weakactionsInfoTicket(actionsInfoTicket);

        m_tickets.insert(id, weakactionsInfoTicket);
        actionsInfoSource->insert(actionsInfoTicket);
        qDebug() << "ACTION STORED";
    }

    lastTime = QTime::currentTime();
    lastAction = actionInfo.name;
}

void TelemetryProvider::saveAssertInfo(QString id, KisAssertInfoTicket::AssertInfo assertInfo)
{
    KUserFeedback::AbstractDataSource* m_assertSource = m_assertsSources[0].get();
    UserFeedback::TelemetryAssertInfoSource* assertSource = nullptr;

    QSharedPointer<KisTelemetryTicket> assertInfoTicket = QSharedPointer<KisAssertInfoTicket>::create(assertInfo, id);

    assertSource = dynamic_cast<UserFeedback::TelemetryAssertInfoSource*>(m_assertSource);

    KIS_SAFE_ASSERT_RECOVER_RETURN(imageProperties);
    assertSource->insert(assertInfoTicket);
}

TelemetryProvider::~TelemetryProvider()
{
}

TelemetryProvider::TelemetryCategory TelemetryProvider::pathToKind(QString path)
{
    if (path == "tools/")
        return tools;
    else if (path == "install/")
        return install;
    else if (path == "fatalAsserts/")
        return fatalAsserts;
    else if (path == "imageProperties/")
        return imageProperties;
    else if (path == "actions/")
        return actions;
    else if (path == "asserts/")
        return asserts;
    return tools;
}
