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
#include <QGlobalStatic>
#include <QStandardPaths>

#include <klocalizedstring.h>
#include <KoResourcePaths.h>
#include <kpluginfactory.h>

#include <KoFileDialog.h>
#include <resources/KoResource.h>
#include <KoResourceServer.h>
#include <KoResourceServerProvider.h>

#include <kis_debug.h>
#include <kis_action.h>
#include <KisViewManager.h>
#include <KisResourceServerProvider.h>
#include <kis_workspace_resource.h>
#include <brushengine/kis_paintop_preset.h>
#include <kis_brush_server.h>
#include <kis_paintop_settings.h>
#include "dlg_bundle_manager.h"
#include "dlg_create_bundle.h"
#include <KisPaintopSettingsIds.h>
#include "krita_container_utils.h"

class ResourceManager::Private {

public:

    Private()
    {
        brushServer = KisBrushServer::instance()->brushServer();
        paintopServer = KisResourceServerProvider::instance()->paintOpPresetServer();
        gradientServer = KoResourceServerProvider::instance()->gradientServer();
        patternServer = KoResourceServerProvider::instance()->patternServer();
        paletteServer = KoResourceServerProvider::instance()->paletteServer();
        workspaceServer = KisResourceServerProvider::instance()->workspaceServer();
        gamutMaskServer = KoResourceServerProvider::instance()->gamutMaskServer();
    }

    KisBrushResourceServer* brushServer;
    KisPaintOpPresetResourceServer * paintopServer;
    KoResourceServer<KoAbstractGradient>* gradientServer;
    KoResourceServer<KoPattern> *patternServer;
    KoResourceServer<KoColorSet>* paletteServer;
    KoResourceServer<KisWorkspaceResource>* workspaceServer;
    KoResourceServer<KoGamutMask>* gamutMaskServer;
};

K_PLUGIN_FACTORY_WITH_JSON(ResourceManagerFactory, "kritaresourcemanager.json", registerPlugin<ResourceManager>();)

ResourceManager::ResourceManager(QObject *parent, const QVariantList &)
    : KisActionPlugin(parent)
    , d(new Private())
{
    KisAction *action = new KisAction(i18n("Import Bundles..."), this);
    addAction("import_bundles", action);
    connect(action, SIGNAL(triggered()), this, SLOT(slotImportBundles()));

    action = new KisAction(i18n("Import Brushes..."), this);
    addAction("import_brushes", action);
    connect(action, SIGNAL(triggered()), this, SLOT(slotImportBrushes()));

    action = new KisAction(i18n("Import Gradients..."), this);
    addAction("import_gradients", action);
    connect(action, SIGNAL(triggered()), this, SLOT(slotImportGradients()));

    action = new KisAction(i18n("Import Palettes..."), this);
    addAction("import_palettes", action);
    connect(action, SIGNAL(triggered()), this, SLOT(slotImportPalettes()));

    action = new KisAction(i18n("Import Patterns..."), this);
    addAction("import_patterns", action);
    connect(action, SIGNAL(triggered()), this, SLOT(slotImportPatterns()));

    action = new KisAction(i18n("Import Presets..."), this);
    addAction("import_presets", action);
    connect(action, SIGNAL(triggered()), this, SLOT(slotImportPresets()));

    action = new KisAction(i18n("Import Workspaces..."), this);
    addAction("import_workspaces", action);
    connect(action, SIGNAL(triggered()), this, SLOT(slotImportWorkspaces()));

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

void ResourceManager::slotCreateBundle()
{
    DlgCreateBundle dlgCreateBundle;
    if (dlgCreateBundle.exec() != QDialog::Accepted) {
        return;
    }
    saveBundle(dlgCreateBundle);
}

KisResourceBundle *ResourceManager::saveBundle(const DlgCreateBundle &dlgCreateBundle)
{
    QString bundlePath =  dlgCreateBundle.saveLocation() + "/" + dlgCreateBundle.bundleName() + ".bundle";

    KisResourceBundle *newBundle = new KisResourceBundle(bundlePath);

    newBundle->addMeta("name", dlgCreateBundle.bundleName());
    newBundle->addMeta("author", dlgCreateBundle.authorName());
    newBundle->addMeta("email", dlgCreateBundle.email());
    newBundle->addMeta("license", dlgCreateBundle.license());
    newBundle->addMeta("website", dlgCreateBundle.website());
    newBundle->addMeta("description", dlgCreateBundle.description());
    newBundle->setThumbnail(dlgCreateBundle.previewImage());

    QStringList res = dlgCreateBundle.selectedBrushes();
    Q_FOREACH (const QString &r, res) {
        KoResource *res = d->brushServer->resourceByFilename(r).data();
        newBundle->addResource("kis_brushes", res->filename(), d->brushServer->assignedTagsList(res), res->md5());
    }

    res = dlgCreateBundle.selectedGradients();
    Q_FOREACH (const QString &r, res) {
        KoResource *res = d->gradientServer->resourceByFilename(r);
        newBundle->addResource("ko_gradients", res->filename(), d->gradientServer->assignedTagsList(res), res->md5());
    }

    res = dlgCreateBundle.selectedPalettes();
    Q_FOREACH (const QString &r, res) {
        KoResource *res = d->paletteServer->resourceByFilename(r);
        newBundle->addResource("ko_palettes", res->filename(), d->paletteServer->assignedTagsList(res), res->md5());
    }

    res = dlgCreateBundle.selectedPatterns();
    Q_FOREACH (const QString &r, res) {
        KoResource *res = d->patternServer->resourceByFilename(r);
        newBundle->addResource("ko_patterns", res->filename(), d->patternServer->assignedTagsList(res), res->md5());
    }

    res = dlgCreateBundle.selectedPresets();
    Q_FOREACH (const QString &r, res) {
        KisPaintOpPresetSP preset = d->paintopServer->resourceByFilename(r);
        KoResource *res = preset.data();
        newBundle->addResource("kis_paintoppresets", res->filename(), d->paintopServer->assignedTagsList(res), res->md5());
        KisPaintOpSettingsSP settings = preset->settings();

        QStringList requiredFiles = settings->getStringList(KisPaintOpUtils::RequiredBrushFilesListTag);
        requiredFiles << settings->getString(KisPaintOpUtils::RequiredBrushFileTag);
        KritaUtils::makeContainerUnique(requiredFiles);

        Q_FOREACH (const QString &brushFile, requiredFiles) {
            KisBrush *brush = d->brushServer->resourceByFilename(brushFile).data();
            if (brush) {
                newBundle->addResource("kis_brushes", brushFile, d->brushServer->assignedTagsList(brush), brush->md5());
            } else {
                qWarning() << "There is no brush with name" << brushFile;
            }
        }
    }

    res = dlgCreateBundle.selectedWorkspaces();
    Q_FOREACH (const QString &r, res) {
        KoResource *res = d->workspaceServer->resourceByFilename(r);
        newBundle->addResource("kis_workspaces", res->filename(), d->workspaceServer->assignedTagsList(res), res->md5());
    }

    res = dlgCreateBundle.selectedGamutMasks();
    Q_FOREACH (const QString &r, res) {
        KoResource *res = d->gamutMaskServer->resourceByFilename(r);
        newBundle->addResource("ko_gamutmasks", res->filename(), d->gamutMaskServer->assignedTagsList(res), res->md5());
    }

    newBundle->addMeta("fileName", bundlePath);
    newBundle->addMeta("created", QDateTime::currentDateTime().toString(Qt::ISODate));

    if (!newBundle->save()) {
        QMessageBox::critical(viewManager()->mainWindow(), i18nc("@title:window", "Krita"), i18n("Could not create the new bundle."));
    }
    else {
        newBundle->setValid(true);
        if (QDir(KisResourceBundleServerProvider::instance()->resourceBundleServer()->saveLocation()) != QDir(QFileInfo(bundlePath).path())) {
            newBundle->setFilename(KisResourceBundleServerProvider::instance()->resourceBundleServer()->saveLocation() + "/" + dlgCreateBundle.bundleName() + ".bundle");
        }
        if (KisResourceBundleServerProvider::instance()->resourceBundleServer()->resourceByName(newBundle->name())) {
            KisResourceBundleServerProvider::instance()->resourceBundleServer()->removeResourceFromServer(
                        KisResourceBundleServerProvider::instance()->resourceBundleServer()->resourceByName(newBundle->name()));
        }
        KisResourceBundleServerProvider::instance()->resourceBundleServer()->addResource(newBundle, true);
        newBundle->load();
    }

    return newBundle;
}

void ResourceManager::slotManageBundles()
{
    DlgBundleManager* dlg = new DlgBundleManager(this, viewManager()->actionManager());
    if (dlg->exec() != QDialog::Accepted) {
        return;
    }
}

QStringList ResourceManager::importResources(const QString &title, const QStringList &mimes) const
{
    KoFileDialog dialog(viewManager()->mainWindow(), KoFileDialog::OpenFiles, "krita_resources");
    dialog.setDefaultDir(QStandardPaths::writableLocation(QStandardPaths::HomeLocation));
    dialog.setCaption(title);
    dialog.setMimeTypeFilters(mimes);
    return dialog.filenames();
}

void ResourceManager::slotImportBrushes()
{
    QStringList resources = importResources(i18n("Import Brushes"), QStringList() << "image/x-gimp-brush"
                                   << "image/x-gimp-x-gimp-brush-animated"
                                   << "image/x-adobe-brushlibrary"
                                   << "image/png"
                                   << "image/svg+xml");
    Q_FOREACH (const QString &res, resources) {
        d->brushServer->importResourceFile(res);
    }
}

void ResourceManager::slotImportPresets()
{
    QStringList resources = importResources(i18n("Import Presets"), QStringList() << "application/x-krita-paintoppreset");
    Q_FOREACH (const QString &res, resources) {
        d->paintopServer->importResourceFile(res);
    }
}

void ResourceManager::slotImportGradients()
{
    QStringList resources = importResources(i18n("Import Gradients"), QStringList() << "image/svg+xml"
                                   << "application/x-gimp-gradient");
    Q_FOREACH (const QString &res, resources) {
        d->gradientServer->importResourceFile(res);
    }
}

void ResourceManager::slotImportBundles()
{
    QStringList resources = importResources(i18n("Import Bundles"), QStringList() << "application/x-krita-bundle");
    Q_FOREACH (const QString &res, resources) {
        KisResourceBundle *bundle = KisResourceBundleServerProvider::instance()->resourceBundleServer()->createResource(res);
        bundle->load();
        if (bundle->valid()) {
            if (!bundle->install()) {
                QMessageBox::warning(0, i18nc("@title:window", "Krita"), i18n("Could not install the resources for bundle %1.", res));
            }
        }
        else {
            QMessageBox::warning(0, i18nc("@title:window", "Krita"), i18n("Could not load bundle %1.", res));
        }

        QFileInfo fi(res);
        QString newFilename = KisResourceBundleServerProvider::instance()->resourceBundleServer()->saveLocation() + fi.completeBaseName() + bundle->defaultFileExtension();
        QFileInfo fileInfo(newFilename);

        int i = 1;
        while (fileInfo.exists()) {
            fileInfo.setFile(KisResourceBundleServerProvider::instance()->resourceBundleServer()->saveLocation() + fi.completeBaseName() + QString("%1").arg(i) + bundle->defaultFileExtension());
            i++;
        }
        bundle->setFilename(fileInfo.filePath());
        QFile::copy(res, newFilename);
        KisResourceBundleServerProvider::instance()->resourceBundleServer()->addResource(bundle, false);
    }
}

void ResourceManager::slotImportPatterns()
{
    QStringList resources = importResources(i18n("Import Patterns"), QStringList() << "image/png"
                                   << "image/svg+xml"
                                   << "application/x-gimp-pattern"
                                   << "image/jpeg"
                                   << "image/tiff"
                                   << "image/bmp"
                                   << "image/xpg");
    Q_FOREACH (const QString &res, resources) {
        d->patternServer->importResourceFile(res);
    }
}

void ResourceManager::slotImportPalettes()
{
    QStringList resources = importResources(i18n("Import Palettes"), QStringList() << "image/x-gimp-color-palette");
    Q_FOREACH (const QString &res, resources) {
        d->paletteServer->importResourceFile(res);
    }
}

void ResourceManager::slotImportWorkspaces()
{
    QStringList resources = importResources(i18n("Import Workspaces"), QStringList() << "application/x-krita-workspace");
    Q_FOREACH (const QString &res, resources) {
        d->workspaceServer->importResourceFile(res);
    }
}

#include "resourcemanager.moc"
