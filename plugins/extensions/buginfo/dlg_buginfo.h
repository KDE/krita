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
