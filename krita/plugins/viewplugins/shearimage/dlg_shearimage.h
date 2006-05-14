/*
 *  dlg_shearimage.h -- part of KimageShop^WKrayon^WKrita
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
#ifndef DLG_SHEARIMAGE
#define DLG_SHEARIMAGE

#include <kdialog.h>

#include "ui_wdg_shearimage.h"

class WdgShearImage : public QWidget, public Ui::WdgShearImage
{
    Q_OBJECT

    public:
        WdgShearImage(QWidget *parent) : QWidget(parent) { setupUi(this); }
};

class DlgShearImage: public KDialog {
    typedef KDialog super;
    Q_OBJECT

public:

    DlgShearImage(QWidget * parent = 0,
             const char* name = 0);
    ~DlgShearImage();

    void setAngleX(quint32 w);
    void setAngleY(quint32 w);
        qint32 angleX();
        qint32 angleY();

private slots:

    void okClicked();

private:

    WdgShearImage * m_page;
    double m_oldAngle;
    bool m_lock;

};

#endif // DLG_SHEARIMAGE
