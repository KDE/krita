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

#include <kmessagebox.h>
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
#include <kis_workspace_resource.h>
#include <kis_paintop_preset.h>
#include <kis_brush_server.h>

#include "dlg_bundle_manager.h"
#include "KoDlgCreateBundle.h"

ResourceBundleServerProvider::ResourceBundleServerProvider()
{
    // user-local
    KGlobal::mainComponent().dirs()->addResourceType("kis_resourcebundles", "data", "krita/bundles/");
    KGlobal::mainComponent().dirs()->addResourceDir("kis_resourcebundles", QDir::homePath() + QString("/.create/bundles"));
    m_resourceBundleServer = new KoResourceServer<KoResourceBundle>("kis_resourcebundles", "*.bundle");
    if (!QFileInfo(m_resourceBundleServer->saveLocation()).exists()) {
        QDir().mkpath(m_resourceBundleServer->saveLocation());
    }
    KoResourceLoaderThread loader(m_resourceBundleServer);
    loader.start();
    loader.barrier();
    foreach(KoResourceBundle *bundle, m_resourceBundleServer->resources()) {
        bundle->install();
    }
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


class ResourceManager::Private {

public:

    Private() {
        brushServer = KisBrushServer::instance()->brushServer();
        paintopServer = KisResourceServerProvider::instance()->paintOpPresetServer();
        gradientServer = KoResourceServerProvider::instance()->gradientServer();
        bundleServer = ResourceBundleServerProvider::instance()->resourceBundleServer();
        patternServer = KoResourceServerProvider::instance()->patternServer();
        paletteServer = KoResourceServerProvider::instance()->paletteServer();
        workspaceServer = KisResourceServerProvider::instance()->workspaceServer();
    }

    KisBrushResourceServer* brushServer;
    KoResourceServer<KisPaintOpPreset>* paintopServer;
    KoResourceServer<KoAbstractGradient>* gradientServer;
    KoResourceServer<KoResourceBundle> *bundleServer;
    KoResourceServer<KoPattern>* patternServer;
    KoResourceServer<KoColorSet>* paletteServer;
    KoResourceServer< KisWorkspaceResource >* workspaceServer;

};

K_PLUGIN_FACTORY(ResourceManagerFactory, registerPlugin<ResourceManager>();)
K_EXPORT_PLUGIN(ResourceManagerFactory("krita"))

ResourceManager::ResourceManager(QObject *parent, const QVariantList &)
    : KisViewPlugin(parent, "kritaplugins/resourcemanager.rc")
    , d(new Private())
{
    Q_UNUSED(ResourceBundleServerProvider::instance()); // load the bundles

    KisAction *action = new KisAction(i18n("Import Resources or Bundles..."), this);
    addAction("import", action);
    connect(action, SIGNAL(triggered()), this, SLOT(slotImport()));

    action = new KisAction(i18n("Create Resource Bundle..."), this);
    addAction("createbundle", action);
    connect(action, SIGNAL(triggered()), this, SLOT(slotCreateBundle()));

    action = new KisAction(i18n("Manage Resource Bundles..."), this);
    addAction("managebundles", action);
    connect(action, SIGNAL(triggered()), this, SLOT(slotManageBundles()));
}

ResourceManager::~ResourceManager()
{
}

void ResourceManager::slotImport()
{
    KoFileDialog dlg(m_view, KoFileDialog::OpenFiles, "krita_resources");
    dlg.setCaption(i18n("Add Resources"));

    QStringList nameFilters;
    nameFilters << i18n("GIMP Brushes (*.gbr)")
                << i18n("Imagepipe Brushes (*.gih)")
                << i18n("Photoshop Brushes (*.abr)")
                << i18n("PNG Brushes (*.png)")
                << i18n("SVG Brushes (*.svg)")
                << i18n("Brush Presets (*.kpp)")
                << i18n("GIMP Gradients (*.ggr)")
                << i18n("SVG Gradients (*.svg)")
                << i18n("Karbon Gradients (*.kgr)")
                << i18n("Resource Bundles (*.bundle)")
                << i18n("GIMP Patterns (*.pat)")
                << i18n("Jpeg Patterns (*.jpg)")
                << i18n("Gif Patterns (*.gif)")
                << i18n("PNG Patterns (*.png)")
                << i18n("Tiff Patterns (*.tif)")
                << i18n("XPM Patterns (*.xpm)")
                << i18n("BMP Patterns (*.bmp)")
                << i18n("Palettes (*.gpl *.pal *.act *.aco *.colors)")
                << i18n("Workspaces (*.kts)");

    dlg.setNameFilters(nameFilters, nameFilters.first());

    QStringList resources = dlg.urls();
    QString resourceType = dlg.selectedNameFilter();
    resourceType = resourceType.left(resourceType.indexOf(" ("));

    int i = -1;
    foreach(const QString &nf, nameFilters) {
        i++;
        if (nf.startsWith(resourceType)) {
            break;
        }
    }

    switch(i) {
    case 0:
    {
        foreach(const QString &res, resources) {
            d->brushServer->importResourceFile(res);
        }
        break;
    }
    case 1:
    {
        foreach(const QString &res, resources) {
            d->paintopServer->importResourceFile(res);
        }
        break;
    }
    case 2:
    {
        foreach(const QString &res, resources) {
            d->gradientServer->importResourceFile(res);
        }
        break;
    }
    case 3:
    {
        foreach(const QString &res, resources) {
            d->bundleServer->importResourceFile(res);
            KoResourceBundle *bundle = d->bundleServer->resourceByFilename(QFileInfo(res).fileName());
            if (bundle) {
                bundle->install();
            }
        }
        break;
    }
    case 4:
    {
         foreach(const QString &res, resources) {
            d->patternServer->importResourceFile(res);
        }
        break;
    }
    case 5:
    {
        foreach(const QString &res, resources) {
            d->paletteServer->importResourceFile(res);
        }
        break;
    }
    case 6:
    {
        foreach(const QString &res, resources) {
            d->workspaceServer->importResourceFile(res);
        }
        break;
    }
    default:
        qWarning() << "Trying to add a resource of an undefined type";
    }
}

void ResourceManager::slotCreateBundle()
{
    KoDlgCreateBundle dlgCreateBundle;
    if (dlgCreateBundle.exec() != QDialog::Accepted) {
        return;
    }

    QString bundlePath =  dlgCreateBundle.saveLocation() + "/" + dlgCreateBundle.bundleName() + ".bundle";
    KoResourceBundle* newBundle = new KoResourceBundle(bundlePath);

    newBundle->addMeta("name", dlgCreateBundle.bundleName());
    newBundle->addMeta("author", dlgCreateBundle.authorName());
    newBundle->addMeta("email", dlgCreateBundle.email());
    newBundle->addMeta("license", dlgCreateBundle.license());
    newBundle->addMeta("website", dlgCreateBundle.website());
    newBundle->addMeta("description", dlgCreateBundle.description());

    QStringList res = dlgCreateBundle.selectedBrushes();
    foreach(const QString &r, res) {
        KoResource *res = d->brushServer->resourceByFilename(r).data();
        newBundle->addResource("kis_brushes", res->filename(), d->brushServer->tagObject()->assignedTagsList(res), res->md5());
    }

    res = dlgCreateBundle.selectedGradients();
    foreach(const QString &r, res) {
        KoResource *res = d->gradientServer->resourceByFilename(r);
        newBundle->addResource("ko_gradients", res->filename(), d->gradientServer->tagObject()->assignedTagsList(res), res->md5());
    }

    res = dlgCreateBundle.selectedPalettes();
    foreach(const QString &r, res) {
        KoResource *res = d->paletteServer->resourceByFilename(r);
        newBundle->addResource("ko_palettes", res->filename(), d->paletteServer->tagObject()->assignedTagsList(res), res->md5());
    }

    res = dlgCreateBundle.selectedPatterns();
    foreach(const QString &r, res) {
        KoResource *res = d->patternServer->resourceByFilename(r);
        newBundle->addResource("kis_patterns", res->filename(), d->patternServer->tagObject()->assignedTagsList(res), res->md5());
    }

    res = dlgCreateBundle.selectedPresets();
    foreach(const QString &r, res) {
        KoResource *res = d->paintopServer->resourceByFilename(r);
        newBundle->addResource("kis_paintoppresets", res->filename(), d->paintopServer->tagObject()->assignedTagsList(res), res->md5());
    }

    res = dlgCreateBundle.selectedWorkspaces();
    foreach(const QString &r, res) {
        KoResource *res = d->workspaceServer->resourceByFilename(r);
        newBundle->addResource("kis_workspaces", res->filename(), d->workspaceServer->tagObject()->assignedTagsList(res), res->md5());
    }

    newBundle->addMeta("fileName", bundlePath);
    newBundle->addMeta("created", QDate::currentDate().toString("dd/MM/yyyy"));

    newBundle->setThumbnail(dlgCreateBundle.previewImage());

    if (!newBundle->save()) {
        KMessageBox::error(m_view, i18n("Could not create the new bundle."), i18n("Error"));
    }


}

void ResourceManager::slotManageBundles()
{
    DlgBundleManager dlg;
    if (dlg.exec() != QDialog::Accepted) {
        return;
    }

}



#include "resourcemanager.moc"
