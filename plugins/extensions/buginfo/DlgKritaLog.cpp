/*
 * SPDX-FileCopyrightText: 2017 Boudewijn Rempt <boud@valdyas.org>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "DlgKritaLog.h"
#include <QStandardPaths>

DlgKritaLog::DlgKritaLog(QWidget *parent)
    : DlgBugInfo(parent)
{
    initialize();
}

QString DlgKritaLog::originalFileName()
{
    return QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) + "/krita.log";
}

QString DlgKritaLog::captionText()
{
    return i18nc("Caption of the dialog with Krita usage log for bug reports", "Krita Usage Log: please paste this information to the bug report");
}

QString DlgKritaLog::replacementWarningText()
{
    return "WARNING: The Krita usage log file doesn't exist.";
}

DlgKritaLog::~DlgKritaLog()
{

}
