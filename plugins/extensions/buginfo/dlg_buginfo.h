/*
 * SPDX-FileCopyrightText: 2017 Boudewijn Rempt <boud@valdyas.org>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef DLG_BUGINFO
#define DLG_BUGINFO

#include <KoDialog.h>

#include "ui_wdg_buginfo.h"

class QSettings;

class WdgBugInfo : public QWidget, public Ui::WdgBugInfo
{
    Q_OBJECT

public:
    WdgBugInfo(QWidget *parent) : QWidget(parent) {
        setupUi(this);
    }
};

class DlgBugInfo: public KoDialog
{
    Q_OBJECT
public:
    DlgBugInfo(QWidget * parent = 0);
    ~DlgBugInfo() override;

    void initialize();
    void initializeText();
    void saveToFile();

    virtual QString defaultNewFileName() = 0;
    virtual QString originalFileName() = 0;
    virtual QString captionText() = 0;
    virtual QString replacementWarningText() = 0;
    QString infoText(QSettings& kritarc);

    QString basicSystemInformationReplacementText();

private:
    WdgBugInfo *m_page;
};

#endif // DLG_BUGINFO
