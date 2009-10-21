/*
 *  Copyright (c) 2007 Boudewijn Rempt <boud@valdyas.org>
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

#include "kis_filter_manager.h"

#include "kis_filter_manager.moc"

#include <QHash>

#include <kactionmenu.h>
#include <kactioncollection.h>

#include <KoID.h>

// krita/image
#include <filter/kis_filter.h>
#include <filter/kis_filter_registry.h>
#include <kis_layer.h>
#include <kis_paint_layer.h>
#include <kis_paint_device.h>

// krita/ui
#include "kis_filter_handler.h"
#include "kis_view2.h"
#include <KoColorSpaceRegistry.h>
#include <KoColorSpaceRegistry.h>

struct KisFilterManager::Private {
    Private() : reapplyAction(0), actionCollection(0) {

    }
    KAction* reapplyAction;
    QHash<QString, KActionMenu*> filterActionMenus;
    QHash<QString, KisFilterHandler*> filterHandlers;
    QHash<KisFilter*, KAction*> filters2Action;
    KActionCollection * actionCollection;
    KisView2* view;
};

KisFilterManager::KisFilterManager(KisView2 * parent, KisDoc2 * doc) : d(new Private)
{
    Q_UNUSED(doc);
    d->view = parent;
}

KisFilterManager::~KisFilterManager()
{
    delete d;
}

void KisFilterManager::setup(KActionCollection * ac)
{
    d->actionCollection = ac;

    // Setup reapply action
    d->reapplyAction = new KAction(i18n("Apply Filter Again"), this);
    d->actionCollection->addAction("filter_apply_again", d->reapplyAction);
    d->reapplyAction->setShortcut(QKeySequence(Qt::CTRL + Qt::SHIFT + Qt::Key_F));
    d->reapplyAction->setEnabled(false);

    // Setup list of filters
    QList<QString> filterList = KisFilterRegistry::instance()->keys();
    for (QList<QString>::Iterator it = filterList.begin(); it != filterList.end(); ++it) {
        insertFilter(*it);
    }
    connect(KisFilterRegistry::instance(), SIGNAL(filterAdded(QString)), SLOT(insertFilter(const QString &)));
}

void KisFilterManager::insertFilter(const QString & name)
{
    Q_ASSERT(d->actionCollection);
    KisFilterSP f = KisFilterRegistry::instance()->value(name);
    Q_ASSERT(f);
    if (d->filters2Action.keys().contains(f.data())) {
        warnKrita << "Filter" << name << " has already been inserted";
        return;
    }
    KoID category = f->menuCategory();
    KActionMenu* actionMenu = d->filterActionMenus[ category.id()];
    if (!actionMenu) {
        actionMenu = new KActionMenu(category.name(), this);
        d->actionCollection->addAction(category.id(), actionMenu);
        d->filterActionMenus[category.id()] = actionMenu;
    }

    KisFilterHandler* handler = new KisFilterHandler(this, f, d->view);

    KAction * a = new KAction(f->menuEntry(), this);
    d->actionCollection->addAction(QString("krita_filter_%1").arg(name), a);
    d->filters2Action[f.data()] = a;
    connect(a, SIGNAL(triggered()), handler, SLOT(showDialog()));
    actionMenu->addAction(a);
}

void KisFilterManager::updateGUI()
{
    if (!d->view) return;

    bool enable = false;
    KisPaintLayerSP player = 0;
    if (d->view->activeLayer()) {
        KisNodeSP layer = d->view->activeNode();
        player = KisPaintLayerSP(dynamic_cast<KisPaintLayer*>(layer.data()));
        if (player && !(*player->colorSpace() == *KoColorSpaceRegistry::instance()->alpha8())) {
            enable = (!layer->userLocked()) && layer->visible() && (!layer->systemLocked());
        }
    }

    d->reapplyAction->setEnabled(enable);
    for (QHash<KisFilter*, KAction*>::iterator it = d->filters2Action.begin();
            it != d->filters2Action.end(); ++it) {
        if (enable && player && it.key()->workWith(player->paintDevice()->colorSpace())) {
            it.value()->setEnabled(enable);
        } else {
            it.value()->setEnabled(false);
        }
    }
}

void KisFilterManager::setLastFilterHandler(KisFilterHandler* handler)
{
    disconnect(d->reapplyAction, SIGNAL(triggered()), 0 , 0);
    connect(d->reapplyAction, SIGNAL(triggered()), handler, SLOT(reapply()));
    d->reapplyAction->setEnabled(true);
    d->reapplyAction->setText(i18n("Apply Filter Again: %1", handler->filter()->name()));
}

