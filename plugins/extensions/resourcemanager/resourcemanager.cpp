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
    KisAction *action = new KisAction(i18n("Manage Resources Libraries..."), this);
    addAction("manage_bundles", action);
    connect(action, SIGNAL(triggered()), this, SLOT(slotManageBundles()));

    action = new KisAction(i18n("Manage Resources..."), this);
    addAction("manage_resources", action);
    connect(action, SIGNAL(triggered()), this, SLOT(slotManageResources()));
}

ResourceManager::~ResourceManager()
{
}

void ResourceManager::slotManageBundles()
{
    QPointer<DlgBundleManager> dlg = new DlgBundleManager(KisPart::instance()->currentMainwindow());
    dlg->exec();
}

void ResourceManager::slotManageResources()
{
    DlgResourceManager dlg(viewManager()->actionManager());
    dlg.exec();
}

#include "resourcemanager.moc"
