/*
 *  dlg_rotateimage.h -- part of KimageShop^WKrayon^WKrita
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
#ifndef DLG_ROTATEIMAGE
#define DLG_ROTATEIMAGE

#include <kdialogbase.h>

#include <kis_global.h>

class WdgRotateImage;

enum enumRotationDirection {
    CLOCKWISE,
    COUNTERCLOCKWISE
};


class DlgRotateImage: public KDialogBase {
    typedef KDialogBase super;
    Q_OBJECT

public:

    DlgRotateImage(QWidget * parent = 0,
             const char* name = 0);
    ~DlgRotateImage();

    void setAngle(Q_UINT32 w);
    Q_INT32 angle();

    void setDirection (enumRotationDirection direction);
    enumRotationDirection direction();

private slots:

    void okClicked();
    void resetPreview();
    void slotAngleValueChanged( int );

private:

    WdgRotateImage * m_page;
    double m_oldAngle;
    bool m_lock;

};

#endif // DLG_ROTATEIMAGE
