/*
 *  kis_brushchooser.h - part of KImageShop
 *
 *  A chooser for KisBrushes. Makes use of the IconChooser class and maintains
 *  all available brushes for KIS.
 *
 *  Copyright (c) 1999 Carsten Pfeiffer <pfeiffer@kde.org>
 *  Copyright (c) 2000 Matthias Elter   <elter@kde.org>
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

#ifndef __kis_brushchooser_h__
#define __kis_brushchooser_h__

#include <qwidget.h>
#include <qframe.h>
#include "kfloatingdialog.h"
#include "kis_brush.h"

class QHBox;
class QLabel;
class IconChooser;
class IntegerWidget;

class KisBrushChooser : public KFloatingDialog
{
  Q_OBJECT

public:
    KisBrushChooser( QWidget *parent, const char *name = 0 );
    ~KisBrushChooser();

    KisBrush  *currentBrush();
    void  setCurrentBrush( KisBrush * );

protected:
    void initGUI();
    IconChooser	*chooser;

private:
    QHBox 	*frame;
    QWidget *container;
    QLabel 	*lbSpacing;
    IntegerWidget *slSpacing;

private slots:
    void slotItemSelected( IconItem * );
    void slotSetBrushSpacing( int );

signals:
    void selected(KisBrush * );

};

#endif //__kis_brushchooser_h__
