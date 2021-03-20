/*
 *  SPDX-FileCopyrightText: 2018 Boudewijn Rempt <boud@valdyas.org>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "DbExplorer.h"

#include <cmath>

#include <klocalizedstring.h>
#include <kis_debug.h>
#include <kpluginfactory.h>
#include <kis_icon.h>
#include <KisViewManager.h>
#include <kis_action.h>
#include "DlgDbExplorer.h"

K_PLUGIN_FACTORY_WITH_JSON(DbExplorerFactory, "kritadbexplorer.json", registerPlugin<DbExplorer>();)

DbExplorer::DbExplorer(QObject *parent, const QVariantList &)
    : KisActionPlugin(parent)
{
    KisAction *action  = createAction("dbexplorer");
    connect(action, SIGNAL(triggered()), this, SLOT(slotDbExplorer()));
}


DbExplorer::~DbExplorer()
{
}

void DbExplorer::slotDbExplorer()
{
    DlgDbExplorer dlgDbExplorer(viewManager()->mainWindow());
    dlgDbExplorer.exec();
}

#include "DbExplorer.moc"
