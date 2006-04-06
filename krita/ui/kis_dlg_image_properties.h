/*
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
#ifndef KIS_DLG_IMAGE_PROPERTIES_H_
#define KIS_DLG_IMAGE_PROPERTIES_H_

#include <kdialogbase.h>

#include <kis_types.h>
#include "wdgnewimage.h"

class Q3ButtonGroup;
class KisID;

class WdgNewImage : public QWidget, public Ui::WdgNewImage
{
    Q_OBJECT

    public:
        WdgNewImage(QWidget *parent) : QWidget(parent) { setupUi(this); }
};

class KisDlgImageProperties : public KDialogBase {
    typedef KDialogBase super;
    Q_OBJECT

public:
    KisDlgImageProperties(KisImageSP image,
                          QWidget *parent = 0,
                          const char *name = 0);
    virtual ~KisDlgImageProperties();

    int imageWidth();
    int imageHeight();
    int opacity();
    QString imageName();
    double resolution();
    QString description();
    KisColorSpace * colorSpace();
    KisProfile * profile();
    
private slots:

    void fillCmbProfiles(const KisID &);

private:

    WdgNewImage * m_page;
    KisImageSP m_image;
};



#endif // KIS_DLG_IMAGE_PROPERTIES_H_

