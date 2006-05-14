/*
 *  dlg_grow_selection.h -- part of Krita
 *
 *  Copyright (c) 2006 Michael Thaler <michael.thaler@physik.tu-muenchen.de>
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
#ifndef DLG_GROW_SELECTION_H
#define DLG_GROW_SELECTION_H

#include <kdialog.h>

#include "ui_wdg_grow_selection.h"

class WdgGrowSelection : public QWidget, public Ui::WdgGrowSelection
{
    Q_OBJECT

    public:
        WdgGrowSelection(QWidget *parent) : QWidget(parent) { setupUi(this); }
};

class DlgGrowSelection: public KDialog {
    typedef KDialog super;
    Q_OBJECT

public:

    DlgGrowSelection(QWidget * parent = 0, const char* name = 0);
    ~DlgGrowSelection();

    qint32 xradius();
    qint32 yradius();

private slots:

    void okClicked();

private:

    WdgGrowSelection * m_page;
};

#endif // DLG_GROW_SELECTION_H
