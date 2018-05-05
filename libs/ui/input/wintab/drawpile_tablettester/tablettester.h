/*
   Drawpile - a collaborative drawing program.

   Copyright (C) 2017 Calle Laakkonen

   Drawpile is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   Drawpile is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with Drawpile.  If not, see <http://www.gnu.org/licenses/>.
*/
#ifndef TABLETTESTDIALOG_H
#define TABLETTESTDIALOG_H

#include <QDialog>

class Ui_TabletTest;

class TabletTestDialog : public QDialog
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
