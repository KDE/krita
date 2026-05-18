/*
 * resourcemanager.cc -- Part of Krita
 *
 * SPDX-FileCopyrightText: 2004 Boudewijn Rempt (boud@valdyas.org)
 * SPDX-FileCopyrightText: 2021 L. E. Segovia <amy@amyspark.me>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "resourcemanager.h"

#include <klocalizedstring.h>
#include <kpluginfactory.h>

#include <kis_action.h>
#include <KisViewManager.h>
#include <KisPart.h>

#include "dlg_bundle_manager.h"
#include "dlg_create_bundle.h"
#include "DlgResourceManager.h"

#ifdef Q_OS_ANDROID
#include "KisSupporterBundlesDialog.h"
#endif

class ResourceManager::Private {
public:

    Private()
    {
    }
};

K_PLUGIN_FACTORY_WITH_JSON(ResourceManagerFactory, "kritaresourcemanager.json", registerPlugin<ResourceManager>();)

ResourceManager::ResourceManager(QObject *parent, const QVariantList &)
    : KisActionPlugin(parent)
    , d(new Private())
{
    KisAction *action = new KisAction(i18n("Manage Resource Libraries..."), this);
    addAction("manage_bundles", action);
    connect(action, SIGNAL(triggered()), this, SLOT(slotManageBundles()));

    action = new KisAction(i18n("Manage Resources..."), this);
    addAction("manage_resources", action);
    connect(action, SIGNAL(triggered()), this, SLOT(slotManageResources()));

#ifdef Q_OS_ANDROID
    action = new KisAction(i18n("Get More Resources..."), this);
    addAction("manage_supporter_bundles", action);
    connect(action, &QAction::triggered, this, &ResourceManager::slotManageSupporterBundles);
#endif
}

ResourceManager::~ResourceManager()
{
}

void ResourceManager::slotManageBundles()
{
    QPointer<DlgBundleManager> dlg = new DlgBundleManager(qApp->activeWindow());
    dlg->exec();
}

void ResourceManager::slotManageResources()
{
    DlgResourceManager dlg(viewManager()->actionManager());
    dlg.exec();
}


#ifdef Q_OS_ANDROID
void ResourceManager::slotManageSupporterBundles()
{
    // Gotta summon it like this, doing it with exec() causes kinetic scrolling
    // to not work properly, requiring two fingers instead of one. No clue why.
    KisSupporterBundlesDialog *dlg = new KisSupporterBundlesDialog(qApp->activeWindow());
    dlg->setAttribute(Qt::WA_DeleteOnClose);
    dlg->show();
}
#endif

#include "resourcemanager.moc"
