/*
 * SPDX-FileCopyrightText: 2017 Boudewijn Rempt <boud@valdyas.org>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef DLG_SYSINFO
#define DLG_SYSINFO

#include <KoDialog.h>
#include "dlg_buginfo.h"


class QSettings;

class DlgSysInfo: public DlgBugInfo
{
    Q_OBJECT
public:
    DlgSysInfo(QWidget * parent = 0);
    ~DlgSysInfo() override;

    QString defaultNewFileName() override {
        return "KritaSystemInformation.txt";
    }

    QString originalFileName() override;

public:
    QString replacementWarningText() override;
    QString captionText() override;
};

#endif // DLG_SYSINFO
