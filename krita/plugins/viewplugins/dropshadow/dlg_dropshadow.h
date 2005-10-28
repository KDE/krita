/*
 *  dlg_dropshadow.h -- part of KimageShop^WKrayon^WKrita
 *
 *  Copyright (c) 2005 Michael Thaler <michael.thaler@physik.tu-muenchen.de>
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
#ifndef DLG_DROPSHADOW
#define DLG_DROPSHADOW

#include <kdialogbase.h>
#include <kis_dropshadow.h>

class WdgDropshadow;
class QColor;

/**
 * This dialog allows the user to configure the decomposition of an image
 * into layers: one layer for each color channel.
 */
class DlgDropshadow: public KDialogBase {
    typedef KDialogBase super;
    Q_OBJECT

public:

    DlgDropshadow(const QString & imageCS, const QString & layerCS, QWidget * parent = 0,
             const char* name = 0);
    ~DlgDropshadow();

public:

    Q_INT32 getXOffset();
    Q_INT32 getYOffset();
    Q_INT32 getBlurRadius();
    Q_UINT8 getShadowOpacity();
    QColor getShadowColor();
    bool allowResizingChecked();
private slots:
    void okClicked();

private:

    WdgDropshadow * m_page;
};

#endif // DLG_DROPSHADOW
