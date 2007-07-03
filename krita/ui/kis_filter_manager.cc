/*
 *  Copyright (c) 2007 Cyrille Berger <cberger@cberger.net>
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


#include "kis_filter_manager.moc"

#include <QHash>

#include <kactionmenu.h>
#include <kactioncollection.h>

#include <KoID.h>

#include "kis_filter.h"
#include "kis_filter_registry.h"

struct KisFilterManager::Private {
    Private() : actionCollection(0) {
        
    }
    QHash<QString, KActionMenu*> filterActionMenus;
    KActionCollection * actionCollection;
};

KisFilterManager::KisFilterManager(KisView2 * parent, KisDoc2 * doc) : d(new Private)
{
    Q_UNUSED(parent);
    Q_UNUSED(doc);
}

KisFilterManager::~KisFilterManager()
{
    delete d;
}

void KisFilterManager::setup(KActionCollection * ac)
{
    d->actionCollection = ac;
    QList<QString> filterList = KisFilterRegistry::instance()->keys();
    for ( QList<QString>::Iterator it = filterList.begin(); it != filterList.end(); ++it ) {
        insertFilter(*it);
    }

}

void KisFilterManager::insertFilter(QString name)
{
    Q_ASSERT(d->actionCollection);
    const KisFilter* f = KisFilterRegistry::instance()->get( name );
    Q_ASSERT(f);
    KoID category = f->menuCategory();
    KActionMenu* actionMenu = d->filterActionMenus[category.id()];
    if(not actionMenu)
    {
        actionMenu = new KActionMenu(category.name(), this);
        d->actionCollection->addAction(category.id(), actionMenu );
        d->filterActionMenus[category.id()] = actionMenu;
        kDebug() << "Creating entry menu for " << category.id() << " with name " << category.name() << endl;
    }
}

void KisFilterManager::updateGUI()
{
    
}
