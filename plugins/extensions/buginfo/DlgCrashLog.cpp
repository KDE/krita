/*
 * SPDX-FileCopyrightText: 2022 Sharaf Zaman <shzam@sdf.org>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include "DlgCrashLog.h"

#include <QStandardPaths>

DlgCrashLog::DlgCrashLog(QWidget *parent)
    : DlgBugInfo(parent)
{
    initialize();
}

QString DlgCrashLog::defaultNewFileName() { return "KritaCrashLog.txt"; }

QString DlgCrashLog::originalFileName()
{
#ifdef Q_OS_WIN
    return QStandardPaths::writableLocation(QStandardPaths::GenericConfigLocation).replace(L'/', L'\\')
         + QStringLiteral("\\kritacrash.log");
#elif defined(Q_OS_ANDROID)
    return QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/kritacrashlog.txt";
#else
    // since we only have crash log for windows and android
    return QString();
#endif
}

QString DlgCrashLog::captionText()
{
    return i18nc("Caption of the dialog with crash log for bug reports",
                 "Krita Crash Log: please paste this information to the bug report");
}

QString DlgCrashLog::replacementWarningText() { return "No Crashes!\n"; }
