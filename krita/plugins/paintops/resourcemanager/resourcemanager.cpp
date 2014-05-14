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
#include <QFileInfo>

#include <klocale.h>
#include <kglobal.h>
#include <kstandarddirs.h>
#include <kcomponentdata.h>
#include <kpluginfactory.h>

#include <KoResourceTagStore.h>
#include <KoFileDialog.h>
#include <KoResource.h>
#include <KoResourceServer.h>
#include <KoResourceServerProvider.h>

#include <kis_debug.h>
#include <kis_action.h>
#include <kis_view2.h>
#include <kis_resource_server_provider.h>
#include "kis_workspace_resource.h"
#include "kis_paintop_preset.h"
#include "kis_brush_server.h"

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
    Q_UNUSED(ResourceBundleServerProvider::instance()); // load the bundles

    KisAction *action = new KisAction(i18n("Resource Manager..."), this);
    addAction("resourcemanager", action);
    connect(action, SIGNAL(triggered()), this, SLOT(slotResourceManager()));

    action = new KisAction(i18n("Import Resources or Bundles..."), this);
    addAction("import", action);
    connect(action, SIGNAL(triggered()), this, SLOT(slotImport()));

    action = new KisAction(i18n("Create Resource Bundle..."), this);
    addAction("createbundle", action);
    connect(action, SIGNAL(triggered()), this, SLOT(slotCreateBundle()));
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

void ResourceManager::slotImport()
{
    KoFileDialog dlg(m_view, KoFileDialog::OpenFiles, "krita_resources");
    dlg.setCaption(i18n("Add Resources"));

    QStringList nameFilters;
    nameFilters << "Brushes (*.gbr *gih *.abr *png *svg)"
                << "Brush Presets (*.kpp)"
                << "Gradients (*.ggr *.svg *.kgr)"
                << "Resource Bundles (*.bundle)"
                << "Patterns (*.pat *.jpg *.gif *.png *.tif *.xpm *.bmp)"
                << "Palettes (*.gpl *.pal *.act *.aco *.colors)"
                << "Workspaces (*.kts)";

    dlg.setNameFilters(nameFilters, nameFilters.first());

    QStringList resources = dlg.urls();
    QString resourceType = dlg.selectedNameFilter();

    switch(nameFilters.indexOf(resourceType)) {
    case 0:
    {
        KoResourceServer<KisBrush>* server = KisBrushServer::instance()->brushServer();
        foreach(const QString &res, resources) {
            server->importResourceFile(res);
        }
        break;
    }
    case 1:
    {
        KoResourceServer<KisPaintOpPreset>* server = KisResourceServerProvider::instance()->paintOpPresetServer();
        foreach(const QString &res, resources) {
            server->importResourceFile(res);
        }
        break;
    }
    case 2:
    {
        KoResourceServer<KoAbstractGradient>* server = KoResourceServerProvider::instance()->gradientServer();
        foreach(const QString &res, resources) {
            server->importResourceFile(res);
        }
        break;
    }
    case 3:
    {
        KoResourceServer<KoResourceBundle> *server = ResourceBundleServerProvider::instance()->resourceBundleServer();
        foreach(const QString &res, resources) {
            server->importResourceFile(res);
            KoResourceBundle *bundle = server->resourceByFilename(QFileInfo(res).fileName());
            bundle->install();
        }
        break;
    }
    case 4:
    {
        KoResourceServer<KoPattern>* server = KoResourceServerProvider::instance()->patternServer();
        foreach(const QString &res, resources) {
            server->importResourceFile(res);
        }
        break;
    }
    case 5:
    {
        KoResourceServer<KoColorSet>* server = KoResourceServerProvider::instance()->paletteServer();
        foreach(const QString &res, resources) {
            server->importResourceFile(res);
        }
        break;
    }
    case 6:
    {
        KoResourceServer< KisWorkspaceResource >* server = KisResourceServerProvider::instance()->workspaceServer();
        foreach(const QString &res, resources) {
            server->importResourceFile(res);
        }
        break;
    }
    default:
        qWarning() << "Trying to add a resource of an undefined type";
    }
}

void ResourceManager::slotCreateBundle()
{

}



#include "resourcemanager.moc"
