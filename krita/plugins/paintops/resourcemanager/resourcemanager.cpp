/*
 * resourcemanager.cc -- Part of Krita
 *
 * Copyright (c) 2004 Boudewijn Rempt (boud@valdyas.org)
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

#include "resourcemanager.h"

#include <QDir>

#include <klocale.h>
#include <kglobal.h>
#include <kstandarddirs.h>
#include <kcomponentdata.h>
#include <kpluginfactory.h>

#include <kis_debug.h>
#include "kis_action.h"

#include <KoResource.h>
#include <KoResourceServer.h>
#include <KoResourceServerProvider.h>

#include "KoResourceManagerWidget.h"

ResourceBundleServerProvider::ResourceBundleServerProvider()
{
    // user-local
    KGlobal::mainComponent().dirs()->addResourceType("kis_resourcebundles", "data", "krita/bundles/");
    KGlobal::mainComponent().dirs()->addResourceDir("kis_resourcebundles", QDir::homePath() + QString("/.create/bundles"));
    m_resourceBundleServer = new KoResourceServer<KoResourceBundle>("kis_resourcebundles", "*.bundle");
    KoResourceLoaderThread loader(m_resourceBundleServer);
    loader.start();
    loader.barrier();
}


ResourceBundleServerProvider *ResourceBundleServerProvider::instance()
{
    K_GLOBAL_STATIC(ResourceBundleServerProvider, s_instance);
    return s_instance;
}

ResourceBundleServerProvider::~ResourceBundleServerProvider()
{
    delete m_resourceBundleServer;
}

KoResourceServer<KoResourceBundle> *ResourceBundleServerProvider::resourceBundleServer()
{
    return m_resourceBundleServer;
}



K_PLUGIN_FACTORY(ResourceManagerFactory, registerPlugin<ResourceManager>();)
K_EXPORT_PLUGIN(ResourceManagerFactory("krita"))

ResourceManager::ResourceManager(QObject *parent, const QVariantList &)
    : KisViewPlugin(parent, "kritaplugins/resourcemanager.rc")
{
    KisAction *action = new KisAction(i18n("Resource Manager..."), this);
    addAction("resourcemanager", action);
    connect(action, SIGNAL(triggered()), this, SLOT(slotResourceManager()));
}

ResourceManager::~ResourceManager()
{
}

void ResourceManager::slotResourceManager()
{
    KoResourceManagerWidget * resourceManager = new KoResourceManagerWidget();
    Q_CHECK_PTR(resourceManager);
    resourceManager->setObjectName("ResourceManager");
    resourceManager->show();

}



#include "resourcemanager.moc"
