/*
 *  kfloatingdialog.h - part of KImageShop
 *
 *  Copyright (c) 1999 Matthias Elter <me@kde.org>
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

#ifndef __kfloatingdialog_h__
#define __kfloatingdialog_h__

#include <qpoint.h>
#include <qcolor.h>
#include <qpixmap.h>
#include <qframe.h>
#include <qpushbutton.h>

#include <kpixmap.h>
#include <kpixmapeffect.h>

#define TITLE_HEIGHT 18
#define MIN_HEIGHT 18
#define MIN_WIDTH 60
#define GRADIENT_HEIGHT 14
#define FRAMEBORDER 2

class KFloatingDialog : public QFrame
{
  Q_OBJECT

 public:
  KFloatingDialog(QWidget *parent = 0, const char* _name = 0);
  ~KFloatingDialog();

  // usable client space:
  int _left() { return FRAMEBORDER; }
  int _top() { return TITLE_HEIGHT; }
  int _width() { return width() - 2*FRAMEBORDER; }
  int _height() { return height() - TITLE_HEIGHT - FRAMEBORDER; }

  void setShaded(bool);
  void setDocked(bool);

  void setBaseWidget(QWidget *);

  bool shaded() { return m_shaded; }
  bool docked() { return m_docked; }

 signals:
  void sigClosed();

 public slots:
  void slotClose();
  void slotMinimize();
  void slotDock();
  
 protected:
  virtual void paintEvent(QPaintEvent *);
  virtual void resizeEvent(QResizeEvent *);
  virtual void leaveEvent(QEvent *);
  virtual void focusInEvent(QFocusEvent *);
  virtual void focusOutEvent(QFocusEvent *);
  virtual void mousePressEvent(QMouseEvent *);
  virtual void mouseMoveEvent(QMouseEvent *);
  virtual void mouseReleaseEvent(QMouseEvent *);
  virtual void mouseDoubleClickEvent (QMouseEvent *); 

  const QRect titleRect()  { return QRect(0,0, width(), TITLE_HEIGHT); }
  const QRect bottomRect() { return QRect(FRAMEBORDER, height()-FRAMEBORDER, width()-2*FRAMEBORDER, FRAMEBORDER); }
  const QRect rightRect() { return QRect(width()-FRAMEBORDER, FRAMEBORDER, FRAMEBORDER, height() - 2*FRAMEBORDER); }
  const QRect lowerRightRect() { return QRect(width()-FRAMEBORDER, height()-FRAMEBORDER , FRAMEBORDER, FRAMEBORDER); }

  void readSettings();
  void writeSettings();

 protected:
  enum TitleLook { plain, gradient, pixmap };
  enum resizeMode { horizontal, vertical, diagonal };

 private:
  bool     m_dragging;
  bool     m_resizing;
  bool     m_shaded;
  bool     m_cursor;
  bool     m_docked;

  QColor   m_activeBlend, m_inactiveBlend;
  KPixmap  m_activeShadePm, m_inactiveShadePm;
  QPixmap  *m_pActivePm, *m_pInactivePm;

  TitleLook m_titleLook;
  KPixmapEffect::GradientType m_gradientType;

  QPoint   m_pos;
  QPoint   m_oldSize;
  QPoint   m_dockedPos;
  int      m_unshadedHeight;
  int      m_resizeMode;

  QWidget  *m_pParent;
  QWidget  *m_pBase;
  QPushButton *m_pCloseButton, *m_pDockButton, *m_pMinButton;
};

#endif
