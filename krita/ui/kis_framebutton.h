/*
 *  kis_framebutton - part of KImageShop
 *
 *  Copyright (c) 2000 Matthias Elter <elter@kde.org>
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


#ifndef __kis_framebutton_h__
#define __kis_framebutton_h__

#include <qlabel.h>
#include <qstring.h>
#include <qpixmap.h>

class KisFrameButton : public QLabel
{
  Q_OBJECT

 public:
  KisFrameButton( QWidget* parent = 0, const char* name = 0 );
  KisFrameButton( const QString& text, QWidget* parent = 0, const char* name = 0 );
  KisFrameButton( const QPixmap& pixmap, QWidget* parent = 0, const char* name = 0 );

  bool isOn() { return m_active; }
  bool isToggleButton() { return m_toggle; }
  void setOn(bool);
  void setToggleButton(bool);

  QString text() { return m_text; }
  virtual void setText ( const QString& );

 signals:
  void clicked();
  void clicked(const QString&);

 protected:
  void mousePressEvent( QMouseEvent * );
  void mouseReleaseEvent( QMouseEvent * );

 private:
  bool     m_active;
  bool     m_toggle;
  QString  m_text;
};

#endif // _kis_framebutton_h__
