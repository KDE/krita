/*
 *  Copyright (c) 2002 Patrick Julien <freak@codepimps.org>
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
#ifndef KIS_DLG_CREATE_IMG_H_
#define KIS_DLG_CREATE_IMG_H_

#include <qspinbox.h>

#include <kdialogbase.h>

#include <qcolor.h>

#include "kis_global.h"
#include "wdgnewimage.h"

class KisProfile;
class KisID;
class QButtonGroup;


class KisDlgCreateImg : public KDialogBase {
    typedef KDialogBase super;
    Q_OBJECT

public:
    KisDlgCreateImg(Q_INT32 maxWidth, Q_INT32 defWidth,
            Q_INT32 maxHeight, Q_INT32 defHeight,
            QString colorSpaceName,
            QString imageName,
            QWidget *parent = 0, const char *name = 0);
    virtual ~KisDlgCreateImg();

public:
    QColor backgroundColor() const;
    Q_UINT8 backgroundOpacity() const;
    KisID colorSpaceID() const;
    Q_INT32 imgWidth() const;
    Q_INT32 imgHeight() const;
    QString imgName() const;
    double imgResolution() const;
    QString imgDescription() const;
    QString profileName() const;

private slots:

    void fillCmbProfiles(const KisID &);

private:

    WdgNewImage * m_page;
};



#endif // KIS_DLG_CREATE_IMG_H_

