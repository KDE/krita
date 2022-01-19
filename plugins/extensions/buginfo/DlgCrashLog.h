/*
 * SPDX-FileCopyrightText: 2022 Sharaf Zaman <shzam@sdf.org>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#ifndef __DLGCRASHLOG_H_
#define __DLGCRASHLOG_H_

#include <dlg_buginfo.h>

class DlgCrashLog : public DlgBugInfo
{
    Q_OBJECT
public:
    DlgCrashLog(QWidget *parent);
    QString defaultNewFileName() override;
    QString originalFileName() override;
    QString captionText() override;
    QString replacementWarningText() override;
};

#endif // __DLGCRASHLOG_H_
