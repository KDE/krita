/*
 *  kis_previewwidget.h - part of Krayon
 *
 *  Copyright (c) 2001 John Califf  <jcaliff@compuzone.net>
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

#ifndef __kis_previewwidget_h__
#define __kis_previewwidget_h__

#include <qframe.h>
#include <qpixmap.h>
#include "kis_layer.h"

class KisPreviewWidget : public QFrame
{
  Q_OBJECT

 public:
    KisPreviewWidget( QWidget* parent = 0, const char* name = 0 );

 public slots:
    void slotSetPreview(KisLayer * lay);

 signals:
    void clicked();
  
 protected:
    virtual void drawContents ( QPainter * );
    virtual void mousePressEvent ( QMouseEvent * );

 private:
   QPixmap pix; 
};

#endif
