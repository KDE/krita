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

#ifndef DLG_KRITA_LOG
#define DLG_KRITA_LOG

#include <KoDialog.h>
#include <dlg_buginfo.h>

class QSettings;

class DlgKritaLog: public DlgBugInfo
{
    Q_OBJECT
public:
    DlgKritaLog(QWidget * parent = 0);
    ~DlgKritaLog() override;


    QString defaultNewFileName() override {
        return "KritaUsageLog.txt";
    }

    QString originalFileName() override;

public:
    QString replacementWarningText() override;
    QString captionText() override;

private:
    WdgBugInfo *m_page;
};

#endif // DLG_BUGINFO
