/*
 *  dlg_imagesplit.cc - part of KimageShop^WKrayon^WKrita
 *
 *  Copyright (c) 2009 Boudewijn Rempt <boud@valdyas.org>
 *  Copyright (c) 2011 Srikanth Tiyyagura <srikanth.tulasiram@gmail.com>
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
#include "wdg_imagesplit.h"

#include <QPainter>
#include <QStringList>
#include <QImage>
#include <QListWidgetItem>
#include <kis_debug.h>

#include "kis_config.h"

WdgImagesplit::WdgImagesplit(QWidget* parent)
    : QWidget(parent)
{
    setupUi(this);

    KisConfig cfg(true);

    intHorizontalSplitLines->setValue(cfg.horizontalSplitLines());
    intVerticalSplitLines->setValue(cfg.verticalSplitLines());

    chkHorizontal->setChecked(true);
    chkAutoSave->setChecked(true);
}


