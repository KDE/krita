/*
 *  dlg_imagesplit.cc - part of KimageShop^WKrayon^WKrita
 *
 *  SPDX-FileCopyrightText: 2009 Boudewijn Rempt <boud@valdyas.org>
 *  SPDX-FileCopyrightText: 2011 Srikanth Tiyyagura <srikanth.tulasiram@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
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


