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

#ifdef Q_OS_ANDROID
#include "DlgAndroidLogcatDumper.h"
#endif

#if defined(Q_OS_ANDROID) || defined(Q_OS_WIN)
#include "DlgCrashLog.h"
#endif

K_PLUGIN_FACTORY_WITH_JSON(BugInfoFactory, "kritabuginfo.json", registerPlugin<BugInfo>();)



BugInfo::BugInfo(QObject *parent, const QVariantList &)
        : KisActionPlugin(parent)
{
    KisAction *actionBug  = createAction("buginfo");
    KisAction *actionSys  = createAction("sysinfo");
    connect(actionBug, SIGNAL(triggered()), this, SLOT(slotKritaLog()));
    connect(actionSys, SIGNAL(triggered()), this, SLOT(slotSysInfo()));

#ifdef Q_OS_ANDROID
    KisAction *actionLogcatdump = createAction("logcatdump");
    connect(actionLogcatdump, SIGNAL(triggered()), this, SLOT(slotDumpLogcat()));
#endif

#if defined(Q_OS_ANDROID) || defined(Q_OS_WIN)
    KisAction *actionCrashLog = createAction("crashlog");
    connect(actionCrashLog, SIGNAL(triggered()), this, SLOT(slotCrashLog()));
#endif
}


BugInfo::~BugInfo()
{
}

void BugInfo::slotKritaLog()
{
    DlgKritaLog dlgKritaLog(viewManager()->mainWindowAsQWidget());
    dlgKritaLog.exec();
}

void BugInfo::slotSysInfo()
{
    DlgSysInfo dlgSysInfo(viewManager()->mainWindowAsQWidget());
    dlgSysInfo.exec();
}

#ifdef Q_OS_ANDROID
void BugInfo::slotDumpLogcat()
{
    DlgAndroidLogcatDumper dlgLogcatDumper(viewManager()->mainWindowAsQWidget());
    dlgLogcatDumper.exec();
}
#endif

#if defined(Q_OS_ANDROID) || defined(Q_OS_WIN)
void BugInfo::slotCrashLog()
{
    DlgCrashLog dlgCrashLog(viewManager()->mainWindowAsQWidget());
    dlgCrashLog.exec();
}
#endif

#include "buginfo.moc"
