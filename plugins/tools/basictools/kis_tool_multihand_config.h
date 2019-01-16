/*
    Copyright (C) 2016  Scott Petrovic <scottpetrovic@gmail.com>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along
    with this program; if not, write to the Free Software Foundation, Inc.,
    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/

#ifndef KISTOOLMULTIHANDCONFIG_H
#define KISTOOLMULTIHANDCONFIG_H

#include "ui_wdgmultihandtool.h"


class KisToolMultiHandConfigWidget : public QWidget, public Ui::WdgMultiHandTool
{
    Q_OBJECT

public:
    KisToolMultiHandConfigWidget(QWidget *parent=0);
    ~KisToolMultiHandConfigWidget() override;

//Q_SIGNALS:


//public Q_SLOTS:
    //void cropTypeSelectableChanged();

//private:
    //KisToolCrop* m_cropTool;
};

#endif // KISTOOLMULTIHANDCONFIG_H
