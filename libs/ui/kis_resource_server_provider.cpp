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
#include <QGlobalStatic>

#include <kis_debug.h>

#include <KoResourcePaths.h>

#include <resources/KoResource.h>
#include <KoResourceServer.h>
#include <KoResourceServerProvider.h>
#include <KoResourceServerAdapter.h>

#include <kis_debug.h>
#include <resources/KoPattern.h>
#include <brushengine/kis_paintop_preset.h>
#include <kis_workspace_resource.h>
#include <kis_psd_layer_style_resource.h>

#include <kis_brush_server.h>

Q_GLOBAL_STATIC(KisResourceServerProvider, s_instance)


typedef KoResourceServerSimpleConstruction<KisPaintOpPreset, SharedPointerStoragePolicy<KisPaintOpPresetSP> > KisPaintOpPresetResourceServer;
typedef KoResourceServerAdapter<KisPaintOpPreset, SharedPointerStoragePolicy<KisPaintOpPresetSP> > KisPaintOpPresetResourceServerAdapter;


inline bool isRunningInKrita() {
    return qApp->applicationName().contains(QLatin1String("krita"), Qt::CaseInsensitive);
}


KisResourceServerProvider::KisResourceServerProvider()
    : m_resourceBundleServer(0)
{
    KisBrushServer *brushServer = KisBrushServer::instance();

    m_paintOpPresetServer = new KisPaintOpPresetResourceServer("kis_paintoppresets", "*.kpp");
    if (!QFileInfo(m_paintOpPresetServer->saveLocation()).exists()) {
        QDir().mkpath(m_paintOpPresetServer->saveLocation());
    }

    m_paintOpPresetThread = new KoResourceLoaderThread(m_paintOpPresetServer);
    m_paintOpPresetThread->loadSynchronously();
//    if (!isRunningInKrita()) {
//        m_paintOpPresetThread->barrier();
//    }

    m_workspaceServer = new KoResourceServerSimpleConstruction<KisWorkspaceResource>("kis_workspaces", "*.kws");
    if (!QFileInfo(m_workspaceServer->saveLocation()).exists()) {
        QDir().mkpath(m_workspaceServer->saveLocation());
    }
    m_workspaceThread = new KoResourceLoaderThread(m_workspaceServer);
    m_workspaceThread->loadSynchronously();
//    if (!isRunningInKrita()) {
//        m_workspaceThread->barrier();
//    }

    m_layerStyleCollectionServer = new KoResourceServerSimpleConstruction<KisPSDLayerStyleCollectionResource>("psd_layer_style_collections", "*.asl");
    if (!QFileInfo(m_layerStyleCollectionServer->saveLocation()).exists()) {
        QDir().mkpath(m_layerStyleCollectionServer->saveLocation());
    }

    m_layerStyleCollectionThread = new KoResourceLoaderThread(m_layerStyleCollectionServer);
    m_layerStyleCollectionThread->loadSynchronously();
//    if (!isRunningInKrita()) {
//        m_layerStyleCollectionThread->barrier();
//    }

    connect(this, SIGNAL(notifyBrushBlacklistCleanup()),
            brushServer, SLOT(slotRemoveBlacklistedResources()));

}

KisResourceServerProvider::~KisResourceServerProvider()
{
    delete m_paintOpPresetThread;
    delete m_workspaceThread;
    delete m_layerStyleCollectionThread;

    delete m_paintOpPresetServer;
    delete m_workspaceServer;
    delete m_layerStyleCollectionServer;
}

KisResourceServerProvider* KisResourceServerProvider::instance()
{
    return s_instance;
}

KoResourceServer<KisResourceBundle> *KisResourceServerProvider::resourceBundleServer()
{

    if (!m_resourceBundleServer)   {
        m_resourceBundleServer = new KoResourceServerSimpleConstruction<KisResourceBundle>("kis_resourcebundles", "*.bundle");

        KoResourceLoaderThread bundleLoader(m_resourceBundleServer);
        bundleLoader.loadSynchronously();
        Q_FOREACH (KisResourceBundle *bundle, m_resourceBundleServer->resources()) {
            if (!bundle->install()) {
                warnKrita << "Could not install resources for bundle" << bundle->name();
            }
        }
    }

    return m_resourceBundleServer;
}


KisPaintOpPresetResourceServer* KisResourceServerProvider::paintOpPresetServer(bool block)
{
    if (block) m_paintOpPresetThread->barrier();
    return m_paintOpPresetServer;
}

KoResourceServer< KisWorkspaceResource >* KisResourceServerProvider::workspaceServer(bool block)
{

    if (block) m_workspaceThread->barrier();
    return m_workspaceServer;
}

KoResourceServer<KisPSDLayerStyleCollectionResource> *KisResourceServerProvider::layerStyleCollectionServer(bool block)
{
    if (block) m_layerStyleCollectionThread->barrier();
    return m_layerStyleCollectionServer;
}

void KisResourceServerProvider::brushBlacklistCleanup()
{
    emit notifyBrushBlacklistCleanup();
}

