/*
 *  kis_patternwidget.h - part of KImageShop
 *
 *  Copyright (c) 2000 Matthias Elter  <elter@kde.org>
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

#ifndef __kis_krayonwidget_h__
#define __kis_krayonwidget_h__

#include <qframe.h>

class KisKrayon;

class KisKrayonWidget : public QFrame
{
  Q_OBJECT

 public:
  KisKrayonWidget( QWidget* parent = 0, const char* name = 0 );

 public slots:
  void slotSetKrayon( const KisKrayon& );

 signals:
  void clicked();
  
 protected:
  virtual void drawContents ( QPainter * );
  virtual void mousePressEvent ( QMouseEvent * );

 private:
  const KisKrayon *m_pKrayon; 
};

#endif
