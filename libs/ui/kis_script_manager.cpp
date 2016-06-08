/*
 *  Copyright (c) 2014 Boudewijn Rempt <boud@valdyas.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "kis_script_manager.h"

#include <klocale.h>
#include <kactionmenu.h>
#include <kactioncollection.h>

#include "KisViewManager.h"

struct KisScriptManager::Private {
    Private()
        : actionCollection(0)
        , view(0)
        , scriptMenu(0)
    {
    }

    QHash<KisFilter*, QAction*> script2Action;

    KActionCollection *actionCollection;
    KisViewManager *view;
    KActionMenu *scriptMenu;
};


KisScriptManager::KisScriptManager(KisViewManager *view)
    : QObject(view)
    , d(new Private())
{
    d->view = view;
}

KisScriptManager::~KisScriptManager()
{
    delete d;
}


void KisScriptManager::setup(KActionCollection * ac)
{
    d->actionCollection = ac;
    d->scriptMenu = new KActionMenu(i18n("Scripts"),this);
    d->actionCollection->addAction("scripting", d->scriptMenu);
}

void KisScriptManager::updateGUI()
{
    if (!d->view) return;

//    bool enable = false;

//    KisNodeSP activeNode = d->view->activeNode();
//    enable = activeNode && activeNode->hasEditablePaintDevice();

//    d->reapplyAction->setEnabled(enable);

//    foreach(KAction *action, d->script2Action.values()) {
//        action->setEnabled(enable);
    //    }
}

void KisScriptManager::addAction(QAction *action)
{
    d->scriptMenu->addAction(action);
}
