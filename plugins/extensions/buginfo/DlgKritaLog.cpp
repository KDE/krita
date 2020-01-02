/*
 * Copyright (c) 2017 Boudewijn Rempt <boud@valdyas.org>
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
