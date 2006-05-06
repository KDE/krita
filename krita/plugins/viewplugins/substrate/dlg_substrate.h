/*
 *  dlg_substrate.h -- part of KimageShop^WKrayon^WKrita
 *
 *  Copyright (c) 2004 Boudewijn Rempt <boud@valdyas.org>
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
#ifndef DLG_SUBSTRATE
#define DLG_SUBSTRATE

#include <qpixmap.h>

#include <kdialogbase.h>

#include "wdgsubstrate.h"


/**
 * This dialog allows the user to modify a layer or a selection
 * by adding more colour in a particular channel or lighten or
 * darken an image.
 */
class DlgSubstrate: public KDialogBase {

    typedef KDialogBase super;
    Q_OBJECT

public:

    DlgSubstrate(QWidget * parent = 0,
              const char* name = 0);
    ~DlgSubstrate();

    /**
     * Set the initial preview pixmap
     */
    void setPixmap(QPixmap pix);

private slots:

    void okClicked();

private:

    WdgSubstrate * m_page;
    QPixmap m_previewPix;
};

#endif // DLG_SUBSTRATE
