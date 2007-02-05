/*
 *  dlg_glsl.h -- part of KimageShop^WKrayon^WKrita
 *
 *  Copyright (c) 2004 Michael Thaler <michael.thaler@physik.tu-muenchen.de>
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
#ifndef DLG_GLSLIMAGE
#define DLG_GLSLIMAGE

#include <kdialog.h>

#include "kis_global.h"

#include "ui_wdg_glsl.h"

class KisView2;

class WdgGlsl : public QWidget, public Ui::WdgGlsl
{
    Q_OBJECT

    public:
        WdgGlsl(QWidget *parent) : QWidget(parent) { setupUi(this); }
};

class DlgGlsl: public KDialog {

    Q_OBJECT

public:

    DlgGlsl(KisView2 * parent = 0, const char* name = 0);
    ~DlgGlsl();


private slots:

    void okClicked();
    void resetPreview();

private:

    bool setupGL();

private:

    class Private;
    Private * m_d;


};

#endif // DLG_GLSLIMAGE
