/*
 * SPDX-FileCopyrightText: 2017 Boudewijn Rempt <boud@valdyas.org>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
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
};

#endif // DLG_BUGINFO
