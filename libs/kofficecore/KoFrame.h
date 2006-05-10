/* This file is part of the KDE project
   Copyright (C) 1998, 1999, 2000 Torben Weis <weis@kde.org>

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

#ifndef __koFrame_h__
#define __koFrame_h__

#include <QWidget>
//Added by qt3to4:
#include <QMouseEvent>
#include <QResizeEvent>
#include <QEvent>
#include <QPaintEvent>

class KoView;
class KoFramePrivate;

class KoFrame : public QWidget
{
  Q_OBJECT
public:
  enum State { Inactive, Selected, Active };

  KoFrame( QWidget *parent, const char *name = 0 );
  virtual ~KoFrame();

  virtual void setView( KoView *view );
  virtual KoView *view() const;

  virtual void setState( State s );
  virtual State state() const;

  virtual int leftBorder() const;
  virtual int rightBorder() const;
  virtual int topBorder() const;
  virtual int bottomBorder() const;

  virtual int border() const;

signals:
  void geometryChanged();

protected:
  virtual void paintEvent( QPaintEvent* );
  virtual void mousePressEvent( QMouseEvent* );
  virtual void mouseMoveEvent( QMouseEvent* );
  virtual void mouseReleaseEvent( QMouseEvent* );
  virtual void resizeEvent( QResizeEvent* );
  virtual bool eventFilter( QObject*, QEvent* );

private:
  KoFramePrivate *d;
};

#endif
