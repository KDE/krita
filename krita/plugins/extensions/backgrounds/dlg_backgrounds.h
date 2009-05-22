/*
 *  dlg_backgrounds.h -- part of KimageShop^WKrayon^WKrita
 *
 *  Copyright (c) 2009 Boudewijn Rempt <boud@valdyas.org>
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
#ifndef DLG_BACKGROUNDS
#define DLG_BACKGROUNDS

#include <kdialog.h>

class KisView2;
#include "wdg_backgrounds.h"
/**
 * This dialog allows the user to create a selection mask based
 * on a (range of) colors.
 */
class DlgBackgrounds: public KDialog
{

    Q_OBJECT

public:

    DlgBackgrounds(KisView2* view);
    ~DlgBackgrounds();

private slots:

    void applyClicked();

private:
    KisView2* m_view;
    WdgBackgrounds* m_page;
};

#endif // DLG_BACKGROUNDS
