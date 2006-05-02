/* This file is part of the KDE project
  Copyright (c) 1999 Matthias Elter (me@kde.org)

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

#ifndef __ko_ColorSlider_h__
#define __ko_ColorSlider_h__

#include <q3frame.h>
#include <QPoint>
#include <QImage>
#include <QWidget>
//Added by qt3to4:
#include <QMouseEvent>
#include <QResizeEvent>
#include <QPaintEvent>
#include <QPixmap>
#include <koffice_export.h>
class SliderWidget;

class KOPAINTER_EXPORT KoColorFrame : public Q3Frame
{
  Q_OBJECT
public:
  KoColorFrame(QWidget *parent = 0L);

  const QColor colorAt(const QPoint &p);
  
protected:
  void drawContents(QPainter *p);
  void mousePressEvent(QMouseEvent *e);
  void mouseMoveEvent(QMouseEvent *e);
  void mouseReleaseEvent(QMouseEvent *e);
  
public slots:
  void slotSetColor1(const QColor &c);
  void slotSetColor2(const QColor &c);

signals:
  void clicked(const QPoint &p);
  void colorSelected(const QColor &c);

protected:
  QColor mC1;
  QColor mC2;
  QPixmap mPixmap;
  QImage  mImage;
  bool mColorChanged;
  bool mPixChanged;
  bool mDragging;
};

class KOPAINTER_EXPORT KoSliderWidget : public QWidget
{
  Q_OBJECT
public:
  KoSliderWidget(QWidget *parent = 0L);

protected:
  void mousePressEvent(QMouseEvent *e);
  void mouseReleaseEvent(QMouseEvent *e);
  void mouseMoveEvent(QMouseEvent *e);
  void paintEvent(QPaintEvent *);
  
signals:
  void positionChanged(int);

protected:
  bool mDragging;
  QPoint mPos;
};

class KOPAINTER_EXPORT KoColorSlider : public QWidget
{
  Q_OBJECT
public:
  KoColorSlider(QWidget *parent = 0L);
  virtual ~KoColorSlider();

  int minValue();
  int maxValue();

protected:
  void resizeEvent(QResizeEvent *);
  void mousePressEvent(QMouseEvent *);
  
public slots:
  void slotSetColor1(const QColor &c);
  void slotSetColor2(const QColor &c);

  void slotSetValue(int value);
  void slotSetRange(int min, int max);

protected slots:
  void slotSliderMoved(int x);
  void slotFrameClicked(const QPoint &p);

signals:
  void colorSelected(const QColor &c);
  void valueChanged(int value);

protected:
  KoSliderWidget *mSlider;
  KoColorFrame *mColorFrame;
  int mMin, mMax;
  int mValue;
};

#endif
