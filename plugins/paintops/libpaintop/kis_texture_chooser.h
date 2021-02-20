/*
  *  SPDX-FileCopyrightText: 2017 Scott Petrovic <scottpetrovic@gmail.com>
  *
  *  SPDX-License-Identifier: GPL-2.0-or-later
  */

#ifndef _KIS_TEXTURE_CHOOSER_H_
#define _KIS_TEXTURE_CHOOSER_H_

#include "ui_wdgtexturechooser.h"

class KisTextureChooser : public QWidget, public Ui::KisWdgTextureChooser
{
    Q_OBJECT

public:
    KisTextureChooser(QWidget *parent = 0);
    ~KisTextureChooser();
};

#endif
