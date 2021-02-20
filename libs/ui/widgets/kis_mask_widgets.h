/*
 *  SPDX-FileCopyrightText: 2007 Boudewijn Rempt <boud@valdyas.org>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
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
