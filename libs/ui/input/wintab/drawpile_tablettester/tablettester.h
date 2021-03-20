/*
   Drawpile - a collaborative drawing program.

   SPDX-FileCopyrightText: 2017 Calle Laakkonen

   SPDX-License-Identifier: GPL-3.0-or-later
*/
#ifndef TABLETTESTDIALOG_H
#define TABLETTESTDIALOG_H

#include <KoDialog.h>

class Ui_TabletTest;

class TabletTestDialog : public KoDialog
{
    Q_OBJECT
public:
    TabletTestDialog(QWidget *parent=nullptr);
    ~TabletTestDialog();
    bool eventFilter(QObject *watched, QEvent *event) override;

private:
    Ui_TabletTest *m_ui;

};

#endif
