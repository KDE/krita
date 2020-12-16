/*
 *  wdg_imagesplit.h -- part of KimageShop^WKrayon^WKrita
 *
 *  SPDX-FileCopyrightText: 2009 Boudewijn Rempt <boud@valdyas.org>
 *  SPDX-FileCopyrightText: 2011 Srikanth Tiyyagura <srikanth.tulasiram@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef WDG_IMAGESPLIT_H
#define WDG_IMAGESPLIT_H

#include <QWidget>
#include "ui_wdg_imagesplit.h"

class WdgImagesplit : public QWidget, public Ui::WdgImagesplit
{
    Q_OBJECT

public:

    WdgImagesplit(QWidget* parent);

};


#endif // WDG_IMAGESPLIT_H
