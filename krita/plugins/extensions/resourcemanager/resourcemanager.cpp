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
#include <QTimer>
#include <QThread>

#include <QMessageBox>
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
#include <KisViewManager.h>
#include <kis_resource_server_provider.h>
#include <kis_workspace_resource.h>
#include <kis_paintop_preset.h>
#include <kis_brush_server.h>

#include "dlg_bundle_manager.h"
#include "dlg_create_bundle.h"

ResourceBundleServerProvider::ResourceBundleServerProvider()
{
    // user-local
    KGlobal::mainComponent().dirs()->addResourceType("kis_resourcebundles", "data", "krita/bundles/");
    KGlobal::mainComponent().dirs()->addResourceDir("kis_resourcebundles", QDir::homePath() + QString("/.create/bundles"));
    m_resourceBundleServer = new KoResourceServerSimpleConstruction<ResourceBundle>("kis_resourcebundles", "*.bundle");
    if (!QFileInfo(m_resourceBundleServer->saveLocation()).exists()) {
        QDir().mkpath(m_resourceBundleServer->saveLocation());
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

KoResourceServer<ResourceBundle> *ResourceBundleServerProvider::resourceBundleServer()
{
    return m_resourceBundleServer;
}


class ResourceManager::Private {

public:

    Private()
        : loader(0)
    {
        brushServer = KisBrushServer::instance()->brushServer(false);
        paintopServer = KisResourceServerProvider::instance()->paintOpPresetServer(false);
        gradientServer = KoResourceServerProvider::instance()->gradientServer(false);
        patternServer = KoResourceServerProvider::instance()->patternServer(false);
        paletteServer = KoResourceServerProvider::instance()->paletteServer(false);
        workspaceServer = KisResourceServerProvider::instance()->workspaceServer(false);
    }

    KisBrushResourceServer* brushServer;
    KisPaintOpPresetResourceServer * paintopServer;
    KoResourceServer<KoAbstractGradient>* gradientServer;
    KoResourceServer<KoPattern> *patternServer;
    KoResourceServer<KoColorSet>* paletteServer;
    KoResourceServer<KisWorkspaceResource>* workspaceServer;

    QThread *loader;

};

K_PLUGIN_FACTORY(ResourceManagerFactory, registerPlugin<ResourceManager>();)
K_EXPORT_PLUGIN(ResourceManagerFactory("krita"))

ResourceManager::ResourceManager(QObject *parent, const QVariantList &)
    : KisViewPlugin(parent)
    , d(new Private())
{
    QTimer::singleShot(0, this, SLOT(loadBundles()));

    KisAction *action = new KisAction(i18n("Import Resources or Bundles..."), this);
    addAction("import_resources", action);
    connect(action, SIGNAL(triggered()), this, SLOT(slotImport()));

    action = new KisAction(i18n("Create Resource Bundle..."), this);
    addAction("create_bundle", action);
    connect(action, SIGNAL(triggered()), this, SLOT(slotCreateBundle()));

    action = new KisAction(i18n("Manage Resources..."), this);
    addAction("manage_bundles", action);
    connect(action, SIGNAL(triggered()), this, SLOT(slotManageBundles()));

}

ResourceManager::~ResourceManager()
{
}

void ResourceManager::slotImport()
{
    KoFileDialog dlg(m_view->mainWindow(), KoFileDialog::OpenFiles, "krita_resources");
    dlg.setCaption(i18n("Add Resources"));

    QMap<QString, QString> filterToTypeMap;
    filterToTypeMap[i18n("Krita Brush Presets (*.kpp)")] = "presets";
    filterToTypeMap[i18n("GIMP Brushes (*.gbr)")] = "brushes";
    filterToTypeMap[i18n("Imagepipe Brushes (*.gih)")] = "brushes";
    filterToTypeMap[i18n("Photoshop Brushes (*.abr)")] = "brushes";
    filterToTypeMap[i18n("PNG Brushes (*.png)")] = "brushes";
    filterToTypeMap[i18n("SVG Brushes (*.svg)")] = "brushes";
    filterToTypeMap[i18n("GIMP Gradients (*.ggr)")] = "gradients";
    filterToTypeMap[i18n("SVG Gradients (*.svg)")] = "gradients";
    filterToTypeMap[i18n("Karbon Gradients (*.kgr)")] = "gradients";
    filterToTypeMap[i18n("Resource Bundles (*.bundle)")] = "bundles";
    filterToTypeMap[i18n("GIMP Patterns (*.pat)")] = "patterns";
    filterToTypeMap[i18n("JPEG Patterns (*.jpg)")] = "patterns";
    filterToTypeMap[i18n("GIF Patterns (*.gif)")] = "patterns";
    filterToTypeMap[i18n("PNG Patterns (*.png)")] = "patterns";
    filterToTypeMap[i18n("TIFF Patterns (*.tif)")] = "patterns";
    filterToTypeMap[i18n("XPM Patterns (*.xpm)")] = "patterns";
    filterToTypeMap[i18n("BMP Patterns (*.bmp)")] = "patterns";
    filterToTypeMap[i18n("Palettes (*.gpl *.pal *.act *.aco *.colors)")] = "palettes";
    filterToTypeMap[i18n("Workspaces (*.kts)")] = "workspaces";

    QStringList nameFilters = filterToTypeMap.keys();

    dlg.setNameFilters(nameFilters, nameFilters[13]);  // start with resource bundle as default type (filterToTypeMap is alphabetized)

    QStringList resources = dlg.urls();
    QString resourceType = dlg.selectedNameFilter();
    if (!filterToTypeMap.contains(resourceType)) {
        QMessageBox::warning(0, i18nc("@title:window", "Krita"), i18n("The selected resource type is unknown."));
        return;
    }

    resourceType = filterToTypeMap[resourceType];

    if (resourceType == "brushes") {
        foreach(const QString &res, resources) {
            d->brushServer->importResourceFile(res);
        }
    }
    else if (resourceType == "presets") {
        foreach(const QString &res, resources) {
            d->paintopServer->importResourceFile(res);
        }
    }
    else if (resourceType == "gradients") {
        foreach(const QString &res, resources) {
            d->gradientServer->importResourceFile(res);
        }
    }
    else if (resourceType == "bundles") {
        foreach(const QString &res, resources) {
            ResourceBundle *bundle = ResourceBundleServerProvider::instance()->resourceBundleServer()->createResource(res);
            bundle->load();
            if (bundle->valid()) {
                if (!bundle->install()) {
                    QMessageBox::warning(0, i18nc("@title:window", "Krita"), i18n("Could not install the resources for bundle %1.").arg(res));
                }
            }
            else {
                QMessageBox::warning(0, i18nc("@title:window", "Krita"), i18n("Could not load bundle %1.").arg(res));
            }

            QFileInfo fi(res);
            QString newFilename = ResourceBundleServerProvider::instance()->resourceBundleServer()->saveLocation() + fi.baseName() + bundle->defaultFileExtension();
            QFileInfo fileInfo(newFilename);

            int i = 1;
            while (fileInfo.exists()) {
                fileInfo.setFile(ResourceBundleServerProvider::instance()->resourceBundleServer()->saveLocation() + fi.baseName() + QString("%1").arg(i) + bundle->defaultFileExtension());
                i++;
            }
            bundle->setFilename(fileInfo.filePath());
            QFile::copy(res, newFilename);
            ResourceBundleServerProvider::instance()->resourceBundleServer()->addResource(bundle, false);
        }
    }
    else if (resourceType == "patterns") {
         foreach(const QString &res, resources) {
            d->patternServer->importResourceFile(res);
        }
    }
    else if (resourceType == "palettes") {
        foreach(const QString &res, resources) {
            d->paletteServer->importResourceFile(res);
        }
    }
    else if (resourceType == "workspaces") {
        foreach(const QString &res, resources) {
            d->workspaceServer->importResourceFile(res);
        }
    }
    else {
        qWarning() << "Trying to add a resource of an undefined type";
    }

}

void ResourceManager::slotCreateBundle()
{
    DlgCreateBundle dlgCreateBundle;
    if (dlgCreateBundle.exec() != QDialog::Accepted) {
        return;
    }

    QString bundlePath =  dlgCreateBundle.saveLocation() + "/" + dlgCreateBundle.bundleName() + ".bundle";
    ResourceBundle* newBundle = new ResourceBundle(bundlePath);

    newBundle->addMeta("name", dlgCreateBundle.bundleName());
    newBundle->addMeta("author", dlgCreateBundle.authorName());
    newBundle->addMeta("email", dlgCreateBundle.email());
    newBundle->addMeta("license", dlgCreateBundle.license());
    newBundle->addMeta("website", dlgCreateBundle.website());
    newBundle->addMeta("description", dlgCreateBundle.description());

    QStringList res = dlgCreateBundle.selectedBrushes();
    foreach(const QString &r, res) {
        KoResource *res = d->brushServer->resourceByFilename(r).data();
        newBundle->addResource("kis_brushes", res->filename(), d->brushServer->assignedTagsList(res), res->md5());
    }

    res = dlgCreateBundle.selectedGradients();
    foreach(const QString &r, res) {
        KoResource *res = d->gradientServer->resourceByFilename(r);
        newBundle->addResource("ko_gradients", res->filename(), d->gradientServer->assignedTagsList(res), res->md5());
    }

    res = dlgCreateBundle.selectedPalettes();
    foreach(const QString &r, res) {
        KoResource *res = d->paletteServer->resourceByFilename(r);
        newBundle->addResource("ko_palettes", res->filename(), d->paletteServer->assignedTagsList(res), res->md5());
    }

    res = dlgCreateBundle.selectedPatterns();
    foreach(const QString &r, res) {
        KoResource *res = d->patternServer->resourceByFilename(r);
        newBundle->addResource("ko_patterns", res->filename(), d->patternServer->assignedTagsList(res), res->md5());
    }

    res = dlgCreateBundle.selectedPresets();
    foreach(const QString &r, res) {
        KisPaintOpPresetSP preset = d->paintopServer->resourceByFilename(r);
        KoResource *res = preset.data();
        newBundle->addResource("kis_paintoppresets", res->filename(), d->paintopServer->assignedTagsList(res), res->md5());
    }

    res = dlgCreateBundle.selectedWorkspaces();
    foreach(const QString &r, res) {
        KoResource *res = d->workspaceServer->resourceByFilename(r);
        newBundle->addResource("kis_workspaces", res->filename(), d->workspaceServer->assignedTagsList(res), res->md5());
    }

    newBundle->addMeta("fileName", bundlePath);
    newBundle->addMeta("created", QDate::currentDate().toString("dd/MM/yyyy"));

    newBundle->setThumbnail(dlgCreateBundle.previewImage());

    if (!newBundle->save()) {
        QMessageBox::critical(m_view->mainWindow(), i18nc("@title:window", "Krita"), i18n("Could not create the new bundle."));
    }


}

void ResourceManager::slotManageBundles()
{


    DlgBundleManager* dlg = new DlgBundleManager(m_view->actionManager());
    if (dlg->exec() != QDialog::Accepted) {
        return;
    }

}

void ResourceManager::loadBundles()
{
    d->loader = new KoResourceLoaderThread(ResourceBundleServerProvider::instance()->resourceBundleServer());
    connect(d->loader, SIGNAL(finished()), this, SLOT(bundlesLoaded()));
    d->loader->start();
}

void ResourceManager::bundlesLoaded()
{
    delete d->loader;
    d->loader = 0;

    foreach(ResourceBundle *bundle, ResourceBundleServerProvider::instance()->resourceBundleServer()->resources()) {
        if (!bundle->install()) {
            qWarning() << "Could not install resources for bundle" << bundle->name();
        }
    }

}



#include "resourcemanager.moc"
