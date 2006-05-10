/* This file is part of the KDE project
  Copyright (c) 1999 Matthias Elter (me@kde.org)
  Copyright (c) 2001 Igor Jansen (rm@kde.org)

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
*/

#ifndef __ko_framebutton_h__
#define __ko_framebutton_h__

#include <QLabel>
//Added by qt3to4:
#include <QPixmap>
#include <QMouseEvent>

class QPixmap;
class QString;

class KoFrameButton : public QLabel
{
  Q_OBJECT
public:
  KoFrameButton(QWidget *parent = 0, const char *name = 0);
  KoFrameButton(const QString &text, QWidget *parent = 0, const char *name = 0);
  KoFrameButton(const QPixmap &pixmap, QWidget *parent = 0, const char *name = 0);

  bool isOn() {return mActive; }
  void setOn(bool v);

  bool isToggleButton() {return mToggle; }
  void setToggleButton(bool v);

  QString text() {return mText; }
  virtual void setText(const QString &t);

signals:
  void clicked();
  void clicked(const QString &);

protected:
  void mousePressEvent(QMouseEvent *);
  void mouseReleaseEvent(QMouseEvent *);

private:
  bool     mActive;
  bool     mToggle;
  QString  mText;
};

#endif
