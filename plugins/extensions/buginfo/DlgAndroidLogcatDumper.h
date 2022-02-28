/*
 * SPDX-FileCopyrightText: 2022 Sharaf Zaman <shzam@sdf.org>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#ifndef __DLGANDROIDLOGCATDUMPER_H_
#define __DLGANDROIDLOGCATDUMPER_H_

#include <QProcess>
#include <QString>
#include <dlg_buginfo.h>

#include <kis_signal_compressor.h>

class DlgAndroidLogcatDumper : public DlgBugInfo
{
    Q_OBJECT
public:
    DlgAndroidLogcatDumper(QWidget *parent = nullptr);
    ~DlgAndroidLogcatDumper();

    QString defaultNewFileName() override;
    QString originalFileName() override;
    QString captionText() override;
    QString replacementWarningText() override;

private Q_SLOTS:
    void writeTextToWidget();

private:
    void enableLogging();
    void disableLogging();

private:
    QProcess m_logcatProcess;
    KisSignalCompressor m_updateWidgetCompressor;
};

#endif // __DLGANDROIDLOGCATDUMPER_H_
