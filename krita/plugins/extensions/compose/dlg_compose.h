/*
 *  dlg_imagesize.h -- part of KimageShop^WKrayon^WKrita
 *
 *  Copyright (c) 2005 Boudewijn Rempt <boud@valdyas.org>
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
#ifndef DLG_COMPOSE
#define DLG_COMPOSE

#include <kdialog.h>
#include "ui_wdg_compose.h"
#include "kis_types.h"
class WdgCompose : public QWidget, public Ui::WdgCompose
{
public:
    WdgCompose(QWidget *parent) : QWidget(parent) {
        setupUi(this);
    }
};

/**
 * This dialog allows the user to configure the decomposition of an image
 * into layers: one layer for each color channel.
 */
class DlgCompose: public KDialog
{

    Q_OBJECT

public:

    DlgCompose(KisImageWSP image, QWidget * parent = 0);
    ~DlgCompose();

private slots:

    void okClicked();

private:

    WdgCompose * m_page;
    KisImageWSP m_image;

};

#endif // DLG_COMPOSE
