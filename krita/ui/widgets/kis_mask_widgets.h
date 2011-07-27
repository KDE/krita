/*
 *  Copyright (c) 2007 Boudewijn Rempt <boud@valdyas.org>
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
#ifndef KIS_MASK_WIDGETS_H
#define KIS_MASK_WIDGETS_H

#include "ui_wdgmasksource.h"
#include "ui_wdgmaskfromselection.h"
#include <kis_filter_strategy.h>
#include "widgets/kis_cmb_idlist.h"

#include <QCheckBox>
#include <QRadioButton>
#include <QLineEdit>
#include <QWidget>
#include <QString>

class WdgMaskSource : public QWidget, public Ui::WdgMaskSource
{
    Q_OBJECT

public:

    WdgMaskSource(QWidget *parent)
            : QWidget(parent) {
        setupUi(this);
    }
};

class WdgMaskFromSelection : public QWidget, public Ui::WdgMaskFromSelection
{
    Q_OBJECT

public:

    WdgMaskFromSelection(QWidget *parent)
            : QWidget(parent) {
        setupUi(this);
    }
};

#endif
