/*
 *  kis_resourceserver.cc - part of KImageShop
 *
 *  Copyright (c) 1999 Matthias Elter <elter@kde.org>
 *  Copyright (c) 2003 Patrick Julien <freak@codepimps.org>
 *  Copyright (c) 2005 Sven Langkamp <sven.langkamp@gmail.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "kis_resource_server_provider.h"

#include <QDir>
#include <QApplication>
#include <QDebug>

#include <kglobal.h>
#include <kstandarddirs.h>
#include <kcomponentdata.h>

#include <KoResource.h>
#include <KoResourceServer.h>
#include <KoResourceServerProvider.h>
#include <KoResourceServerAdapter.h>

#include <kis_debug.h>
#include <KoPattern.h>
#include <kis_paintop_preset.h>
#include <kis_workspace_resource.h>


#include <kis_brush_server.h>

typedef KoResourceServer<KisPaintOpPreset, SharedPointerStroragePolicy<KisPaintOpPresetSP> > KisPaintOpPresetResourceServer;
typedef KoResourceServerAdapter<KisPaintOpPreset, SharedPointerStroragePolicy<KisPaintOpPresetSP> > KisPaintOpPresetResourceServerAdapter;


KisResourceServerProvider::KisResourceServerProvider()
{

    KisBrushServer *brushServer = KisBrushServer::instance();

    KGlobal::mainComponent().dirs()->addResourceType("kis_paintoppresets", "data", "krita/paintoppresets/");
    KGlobal::mainComponent().dirs()->addResourceDir("kis_paintoppresets", QDir::homePath() + QString("/.create/paintoppresets/krita"));

    KGlobal::mainComponent().dirs()->addResourceType("kis_workspaces", "data", "krita/workspaces/");
    
    m_paintOpPresetServer = new KisPaintOpPresetResourceServer("kis_paintoppresets", "*.kpp");
    if (!QFileInfo(m_paintOpPresetServer->saveLocation()).exists()) {
        QDir().mkpath(m_paintOpPresetServer->saveLocation());
    }
    paintOpPresetThread = new KoResourceLoaderThread(m_paintOpPresetServer);
    paintOpPresetThread->start();
    if (!qApp->applicationName().toLower().contains("krita")) {
        paintOpPresetThread->barrier();
    }

    m_workspaceServer = new KoResourceServer<KisWorkspaceResource>("kis_workspaces", "*.kws");
    if (!QFileInfo(m_workspaceServer->saveLocation()).exists()) {
        QDir().mkpath(m_workspaceServer->saveLocation());
    }
    workspaceThread = new KoResourceLoaderThread(m_workspaceServer);
    workspaceThread->start();
    if (!qApp->applicationName().toLower().contains("krita")) {
        workspaceThread->barrier();
    }


    connect(this, SIGNAL(notifyBrushBlacklistCleanup()),
            brushServer, SLOT(slotRemoveBlacklistedResources()));

}

KisResourceServerProvider::~KisResourceServerProvider()
{
    delete paintOpPresetThread;
    delete workspaceThread;

    delete m_paintOpPresetServer;
    delete m_workspaceServer;
}

KisResourceServerProvider* KisResourceServerProvider::instance()
{
    K_GLOBAL_STATIC(KisResourceServerProvider, s_instance);
    return s_instance;
}


KisPaintOpPresetResourceServer* KisResourceServerProvider::paintOpPresetServer()
{
    paintOpPresetThread->barrier();
    return m_paintOpPresetServer;
}

KoResourceServer< KisWorkspaceResource >* KisResourceServerProvider::workspaceServer()
{
    workspaceThread->barrier();
    return m_workspaceServer;
}

void KisResourceServerProvider::brushBlacklistCleanup()
{
    emit notifyBrushBlacklistCleanup();
}


#include "kis_resource_server_provider.moc"
