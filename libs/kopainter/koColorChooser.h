/* This file is part of the KDE project
  Copyright (c) 1999 Matthias Elter (me@kde.org)
  Copyright (c) 2001-2002 Igor Jansen (rm@kde.org)

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

#ifndef __ko_ColorChooser_h__
#define __ko_ColorChooser_h__

#include <QWidget>
//Added by qt3to4:
#include <QLabel>
#include <Q3GridLayout>
#include "koColor.h"

class KoFrameButton;
class Q3GridLayout;
class QTabWidget;
class RGBWidget;
class HSVWidget;
class CMYKWidget;
class LABWidget;
class GreyWidget;
class KoColor;
class KoColorSlider;
class QLabel;
class QSpinBox;
class KHSSelector;
class KColorPatch;
class ColorWidget;

class KoColorChooser : public QWidget
{
  Q_OBJECT
public:
  KoColorChooser(QWidget *parent = 0L, const char *name = 0L);

  const KoColor &color() const {return mColor; }

public slots:
  void slotChangeColor(const KoColor &c);
  void slotChangeColor(const QColor &c);

signals:
  void colorChanged(const KoColor &c);

protected slots:
  void slotChangeXY(int h, int s);

private slots:
  void childColorChanged(const KoColor& c);
  void slotCurrentChanged(QWidget *current);

private:
  ColorWidget *m_current;
  KoColor           mColor;
  QTabWidget *m_tab;
  Q3GridLayout      *mGrid;
  KoFrameButton    *btnRGB;
  KoFrameButton    *btnHSV;
  KoFrameButton    *btnCMYK;
  KoFrameButton    *btnLAB;
  KoFrameButton    *btnGrey;
  RGBWidget        *mRGBWidget;
  HSVWidget        *mHSVWidget;
  QWidget       *mCMYKWidget;
  LABWidget        *mLABWidget;
  GreyWidget       *mGreyWidget;
  KHSSelector      *mColorSelector;
};

class ColorWidget : public QWidget {
	Q_OBJECT

public:
	ColorWidget(QWidget *parent = 0);
	virtual ~ColorWidget();

public slots:
	virtual void slotChangeColor(const KoColor& c);
	virtual void slotChangeColor(const QColor& c);
	virtual void slotRefreshColor() = 0;

protected:
	KoColor mColor;
};

class RGBWidget : public ColorWidget
{
  Q_OBJECT
public:
  RGBWidget(QWidget *parent = 0L);
  virtual ~RGBWidget() {}

public slots:
  virtual void slotRefreshColor();

protected slots:
  void slotRSliderChanged(int r);
  void slotGSliderChanged(int g);
  void slotBSliderChanged(int b);

  void slotRInChanged(int r);
  void slotGInChanged(int g);
  void slotBInChanged(int b);

  void slotPatchChanged(const QColor& clr);

signals:
  void colorChanged(const KoColor &c);

private:
  KoColorSlider *mRSlider;
  KoColorSlider *mGSlider;
  KoColorSlider *mBSlider;
  QLabel *mRLabel;
  QLabel *mGLabel;
  QLabel *mBLabel;
  QSpinBox *mRIn;
  QSpinBox *mGIn;
  QSpinBox *mBIn;
  KColorPatch *mColorPatch;
};

class HSVWidget : public ColorWidget
{
  Q_OBJECT
public:
  HSVWidget(QWidget *parent = 0L);
  virtual ~HSVWidget() {}

public slots:
  virtual void slotRefreshColor();

protected slots:
  void slotHSliderChanged(int h);
  void slotSSliderChanged(int s);
  void slotVSliderChanged(int v);

  void slotHInChanged(int h);
  void slotSInChanged(int s);
  void slotVInChanged(int v);

  void slotPatchChanged(const QColor& clr);

signals:
  void colorChanged(const KoColor &c);

private:
  KoColorSlider    *mHSlider;
  KoColorSlider    *mSSlider;
  KoColorSlider    *mVSlider;
  QLabel           *mHLabel;
  QLabel           *mSLabel;
  QLabel           *mVLabel;
  QSpinBox         *mHIn;
  QSpinBox         *mSIn;
  QSpinBox         *mVIn;
  KColorPatch *mColorPatch;
};

class GreyWidget : public ColorWidget
{
  Q_OBJECT
public:
  GreyWidget(QWidget *parent = 0L);
  virtual ~GreyWidget() {}

public slots:
  virtual void slotRefreshColor();

protected slots:
  void slotVSliderChanged(int v);
  void slotVInChanged(int v);
  void slotPatchChanged(const QColor& clr);

signals:
  void colorChanged(const KoColor &c);

protected:
  KoColorSlider *mVSlider;
  QLabel      *mVLabel;
  QSpinBox    *mVIn;
  KColorPatch *mColorPatch;
};

class LABWidget : public ColorWidget
{
  Q_OBJECT
public:
  LABWidget(QWidget *parent = 0L);
  virtual ~LABWidget() {}

public slots:
  virtual void slotRefreshColor();

protected slots:
  void slotLSliderChanged(int l);
  void slotASliderChanged(int a);
  void slotBSliderChanged(int b);

  void slotLInChanged(int l);
  void slotAInChanged(int a);
  void slotBInChanged(int b);

  void slotPatchChanged(const QColor& clr);

signals:
  void colorChanged(const KoColor &c);

private:
  KoColorSlider *mLSlider;
  KoColorSlider *mASlider;
  KoColorSlider *mBSlider;
  QLabel *mLLabel;
  QLabel *mALabel;
  QLabel *mBLabel;
  QSpinBox *mLIn;
  QSpinBox *mAIn;
  QSpinBox *mBIn;
  KColorPatch *mColorPatch;
};

#endif

