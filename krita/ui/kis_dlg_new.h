/*
 *  newdialog.h - part of KImageShop
 *
 *  Copyright (c) 1999 Sven Fischer    <herpes@kawo2.rwth-aachen.de>
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
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#ifndef __newdialog_h__
#define __newdialog_h__

#include <qdialog.h>
#include <qspinbox.h>

#include "kis_global.h"

class QRadioButton;

class NewDialog : public QDialog
{
    Q_OBJECT

public:

    NewDialog( QWidget *parent = 0, const char *name = 0 );

    bgMode backgroundMode();
    cMode colorMode();

    int newwidth() { return iwidth->value(); };
    int newheight() { return iheight->value(); };

private:
    QRadioButton* cmode[6];
    QRadioButton* bground[4];
    QSpinBox* iwidth;
    QSpinBox* iheight;
};

#endif // __newdialog.h__
