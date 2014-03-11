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

#include <klocale.h>
#include <kis_debug.h>
#include <kpluginfactory.h>

//#include "dlg_resourcemanager.h"
#include "KoResourceManagerWidget.h"
#include "kis_action.h"

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
