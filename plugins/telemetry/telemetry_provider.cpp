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
#include <presets_info_source.h>

TelemetryProvider::TelemetryProvider()
{
    for (int i = 0; i < lastElement - 1; i++) {
        m_providers.emplace_back(new KUserFeedback::Provider);
        m_sources.push_back(std::move( TSources{}));
        m_providers[i]->setTelemetryMode(KUserFeedback::Provider::DetailedUsageStatistics);
    }
    KConfigGroup configGroup = KSharedConfig::openConfig()->group("KisTelemetryInstall");

    QString isFirstStart = configGroup.readEntry("FirstStart");
    QString version = configGroup.readEntry("Version", "noversion");
    m_sendInstallInfo = false;
    qDebug() << "APP VERSION______" << qApp->applicationVersion();
    if (qApp->applicationVersion() != version) {
        configGroup.writeEntry("Version", qApp->applicationVersion());
        qDebug() << "SEND INSTALL__" << version;
#ifndef HAVE_BACKTRACE
        if (isFirstStart.isEmpty()) {
#endif
            //don't append install source
            TSource cpu(new UserFeedback::TelemetryCpuInfoSource());
            m_sources[install].push_back(std::move(cpu));
            TSource qt(new KUserFeedback::QtVersionSource());
            m_sources[install].push_back(std::move(qt));
            TSource compiler(new KUserFeedback::CompilerInfoSource());
            m_sources[install].push_back(std::move(compiler));
            TSource opengl(new KUserFeedback::OpenGLInfoSource());
            m_sources[install].push_back(std::move(opengl));
            TSource platform(new KUserFeedback::PlatformInfoSource());
            m_sources[install].push_back(std::move(platform));
            TSource screen(new KUserFeedback::ScreenInfoSource());
            m_sources[install].push_back(std::move(screen));
#ifdef HAVE_BACKTRACE
            std::unique_ptr<KUserFeedback::AbstractDataSource> general(new UserFeedback::TelemetryGeneralInfoSource);
#endif
            m_sources[install].push_back(std::move(general));
            for (auto& source : m_sources[install]) {
                m_providers[install]->addDataSource(source.get());
            }
            m_sendInstallInfo = true;
#ifndef HAVE_BACKTRACE
        }
#endif
    }

    TSource tools1(new UserFeedback::TelemetryToolsInfoSource);
    m_sources[tools].push_back(std::move(tools1));

    TSource asserts1(new UserFeedback::TelemetryFatalAssertInfoSource);
    m_sources[fatalAsserts].push_back(std::move(asserts1));

    TSource imageProperties1(new UserFeedback::TelemetryImagePropertiesSource);
    m_sources[imageProperties].push_back(std::move(imageProperties1));

    TSource actionsInfo(new UserFeedback::TelemetryActionsInfoSource);
    m_sources[actions].push_back(std::move(actionsInfo));

    TSource assertsInfo(new UserFeedback::TelemetryAssertInfoSource);
    m_sources[asserts].push_back(std::move(assertsInfo));

    for (int i = 0; i < lastElement - 1; i++) {
        if (i != install) {
            for (auto& source : m_sources[i]) {
                m_providers[i]->addDataSource(source.get());
            }
        }
    }
    //TODO
    TSource presets(new UserFeedback::TelemetryPresetsSource());
    //m_installSources.push_back(std::move(screen));
}

void TelemetryProvider::sendData(QString path, QString adress)
{
    if (!path.endsWith(QLatin1Char('/')))
        path += QLatin1Char('/');
    TelemetryCategory enumPath = pathToKind(path);
    path += m_apiVersion;
    QString finalAdress = adress.isNull() ? m_adress : adress;
    switch (enumPath) {
    case tools: {
         m_providers[tools]->setFeedbackServer(QUrl(finalAdress + path));
         m_providers[tools]->submit();
        break;
    }
    case install: {
        //  if (m_sendInstallInfo) {
        m_providers[install]->setFeedbackServer(QUrl(finalAdress + path));
         m_providers[install]->submit();
        m_sendInstallInfo = false;
        break;
        //   }
    }
    case asserts: {
         m_providers[asserts]->setFeedbackServer(QUrl(finalAdress + path));
         m_providers[asserts]->submit();
        break;
    }
    case fatalAsserts: {
         m_providers[fatalAsserts]->setFeedbackServer(QUrl(finalAdress + path));
         m_providers[fatalAsserts]->submit();
        break;
    }
    case imageProperties: {
        m_providers[imageProperties]->setFeedbackServer(QUrl(finalAdress + path));
         m_providers[imageProperties]->submit();

        break;
    }
    case actions: {
        qDebug() << "Send actions!";
         m_providers[actions]->setFeedbackServer(QUrl(finalAdress + path));
         m_providers[actions]->submit();
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
    KUserFeedback::AbstractDataSource* m_tools = m_sources[tools][0].get();
    UserFeedback::TelemetryToolsInfoSource* tools1 = nullptr;

    timeTicket = dynamic_cast<KisTimeTicket*>(ticket);
    tools1 = dynamic_cast<UserFeedback::TelemetryToolsInfoSource*>(m_tools);

    KIS_SAFE_ASSERT_RECOVER_RETURN(timeTicket);
    KIS_SAFE_ASSERT_RECOVER_RETURN(tools1);
    KIS_SAFE_ASSERT_RECOVER_RETURN(ticket);

    tools1->deactivateTool(id);
    m_tickets.remove(id);
}

void TelemetryProvider::putTimeTicket(QString id)
{
    if (m_tickets.count(id)) {
        return;
    }
    KUserFeedback::AbstractDataSource* m_tools =  m_sources[tools][0].get();
    UserFeedback::TelemetryToolsInfoSource* tools1 = nullptr;

    QSharedPointer<KisTelemetryTicket> timeTicket = QSharedPointer<KisTimeTicket>::create(id);

    tools1 = dynamic_cast<UserFeedback::TelemetryToolsInfoSource*>(m_tools);

    KIS_SAFE_ASSERT_RECOVER_RETURN(tools1);

    QWeakPointer<KisTelemetryTicket> weakTimeTicket(timeTicket);

    m_tickets.insert(id, weakTimeTicket);
    tools1->activateTool(timeTicket);
}

void TelemetryProvider::saveImageProperites(QString fileName, KisImagePropertiesTicket::ImageInfo imageInfo)
{
    KUserFeedback::AbstractDataSource* m_imageProperties =  m_sources[imageProperties][0].get();
    UserFeedback::TelemetryImagePropertiesSource* imageProperties1 = nullptr;

    QSharedPointer<KisTelemetryTicket> imagePropertiesTicket = QSharedPointer<KisImagePropertiesTicket>::create(imageInfo, fileName);

    imageProperties1 = dynamic_cast<UserFeedback::TelemetryImagePropertiesSource*>(m_imageProperties);

    KIS_SAFE_ASSERT_RECOVER_RETURN(imageProperties1);

    if (m_tickets.count(fileName)) {
        imageProperties1->removeDumpProperties(fileName);
        return;
    }

    QWeakPointer<KisTelemetryTicket> weakimagePropertiesTicket(imagePropertiesTicket);

    m_tickets.insert(fileName, weakimagePropertiesTicket);
    imageProperties1->createNewImageProperties(imagePropertiesTicket);
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
        KUserFeedback::AbstractDataSource* m_actionsInfo = m_sources[actions][0].get();
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
    KUserFeedback::AbstractDataSource* m_assertSource = m_sources[asserts][0].get();
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
    else if (path == "presets/")
        return presets;
    return tools;
}
