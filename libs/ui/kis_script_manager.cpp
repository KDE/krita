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

#include <klocalizedstring.h>
#include <kactionmenu.h>
#include <kactioncollection.h>

#include <kis_action_manager.h>
#include "KisViewManager.h"

struct KisScriptManager::Private {
    Private()
        : actionCollection(0)
        , actionManager(0)
        , view(0)
        , scriptMenu(0)
    {
    }

    KActionCollection *actionCollection;
    KisActionManager *actionManager;
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


void KisScriptManager::setup(KActionCollection * ac, KisActionManager *actionManager)
{
    d->actionCollection = ac;
    d->actionManager = actionManager;

    d->scriptMenu = new KActionMenu(i18n("Scripts"),this);
    d->actionCollection->addAction("scripts", d->scriptMenu);
}

void KisScriptManager::updateGUI()
{
    if (!d->view) return;
}

void KisScriptManager::addAction(QAction *action)
{
    Q_ASSERT(d->actionCollection);
    d->scriptMenu->addAction(action);
}
