/*
 *  SPDX-FileCopyrightText: 2017 Boudewijn Rempt <boud@valdyas.org>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "buginfo.h"

#include <cmath>

#include <klocalizedstring.h>
#include <kis_debug.h>
#include <kpluginfactory.h>
#include <kis_icon.h>
#include <KisViewManager.h>
#include <kis_action.h>
#include "DlgKritaLog.h"
#include "DlgSysInfo.h"


K_PLUGIN_FACTORY_WITH_JSON(BugInfoFactory, "kritabuginfo.json", registerPlugin<BugInfo>();)



BugInfo::BugInfo(QObject *parent, const QVariantList &)
        : KisActionPlugin(parent)
{
    KisAction *actionBug  = createAction("buginfo");
    KisAction *actionSys  = createAction("sysinfo");
    connect(actionBug, SIGNAL(triggered()), this, SLOT(slotKritaLog()));
    connect(actionSys, SIGNAL(triggered()), this, SLOT(slotSysInfo()));

}


BugInfo::~BugInfo()
{
}

void BugInfo::slotKritaLog()
{
    DlgKritaLog dlgKritaLog(viewManager()->mainWindow());
    dlgKritaLog.exec();
}

void BugInfo::slotSysInfo()
{
    DlgSysInfo dlgSysInfo(viewManager()->mainWindow());
    dlgSysInfo.exec();
}


#include "buginfo.moc"
