/*
 *  dlg_colorspaceconversion.h -- part of KimageShop^WKrayon^WKrita
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
#ifndef DLG_COLORSPACECONVERSION
#define DLG_COLORSPACECONVERSION

#include <QButtonGroup>

#include <kdialog.h>

#include <KoID.h>

#include "ui_wdgconvertcolorspace.h"

class WdgConvertColorSpace : public QWidget, public Ui::WdgConvertColorSpace
{
    Q_OBJECT

    public:
        WdgConvertColorSpace(QWidget *parent) : QWidget(parent) { setupUi(this); }
};

/**
 * XXX
 */
class DlgColorSpaceConversion: public KDialog {
    typedef KDialog super;
    Q_OBJECT

public:

    DlgColorSpaceConversion(QWidget * parent = 0, const char* name = 0);
    ~DlgColorSpaceConversion();

    WdgConvertColorSpace * m_page;

    QButtonGroup m_intentButtonGroup;

public slots:

    void okClicked();
    void fillCmbDestProfile(const KoID &);
};

#endif // DLG_COLORSPACECONVERSION
