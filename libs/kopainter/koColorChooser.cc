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

#include "koColorChooser.h"

#include <QColor>
#include <QLayout>
#include <QSpinBox>
#include <qtabwidget.h>
//Added by qt3to4:
#include <QLabel>
#include <Q3GridLayout>

#include <klocale.h>
#include <kiconloader.h>
#include <kcolordialog.h>
#include <ktabctl.h>
#include "koFrameButton.h"
#include "koColorSlider.h"

KoColorChooser::KoColorChooser(QWidget *parent, const char *name) : QWidget(parent, name)
{
  m_current = 0;
  m_tab = new QTabWidget(this, "KoColorChooser tab");
  mGrid = new Q3GridLayout(this, 3, 5);
  mRGBWidget = new RGBWidget(m_tab);
  m_current = mRGBWidget;
  m_tab -> addTab(mRGBWidget, "RGB");
  mHSVWidget = new HSVWidget(m_tab);
  m_tab -> addTab(mHSVWidget, "HSV");
#if 0
  mCMYKWidget = new QWidget(m_tab);
  m_tab -> addTab(mCMYKWidget, "CMYK");
  mLABWidget = new LABWidget(m_tab);
  m_tab -> addTab(mLABWidget, "LAB");
#endif
  mGreyWidget = new GreyWidget(m_tab);
  m_tab -> addTab(mGreyWidget, i18n("Gray"));
  mColorSelector = new KHSSelector(this);
  mColorSelector->setFixedHeight(20);
  mGrid->addMultiCellWidget(m_tab, 0, 1, 0, 4);
  mGrid->addMultiCellWidget(mColorSelector, 2, 2, 0, 4);
  connect(mRGBWidget, SIGNAL(colorChanged(const KoColor &)), this, SLOT(childColorChanged(const KoColor &)));
  connect(mHSVWidget, SIGNAL(colorChanged(const KoColor &)), this, SLOT(childColorChanged(const KoColor &)));
//  connect(mLABWidget, SIGNAL(colorChanged(const KoColor &)), this, SLOT(childColorChanged(const KoColor &)));
  connect(mGreyWidget, SIGNAL(colorChanged(const KoColor &)), this, SLOT(childColorChanged(const KoColor &)));
  connect(mColorSelector, SIGNAL(valueChanged(int, int)), this, SLOT(slotChangeXY(int, int)));
  connect(m_tab, SIGNAL(currentChanged(QWidget*)), this, SLOT(slotCurrentChanged(QWidget*)));
  slotChangeColor(KoColor::black());
}

void KoColorChooser::slotCurrentChanged(QWidget *current)
{
  m_current = static_cast<ColorWidget*>(current);
  m_current -> slotChangeColor(mColor);
}

void KoColorChooser::slotChangeXY(int h, int s)
{
  KoColor c(h, s, 192, KoColor::csHSV);

  m_current -> slotChangeColor(c);
}

void KoColorChooser::slotChangeColor(const QColor &c)
{
  slotChangeColor(KoColor(c));
}

void KoColorChooser::childColorChanged(const KoColor& c)
{
  mColor.setRGB(c.R(), c.G(), c.B());
  emit colorChanged(mColor);
}

void KoColorChooser::slotChangeColor(const KoColor &c)
{
  mColor = c;
  m_current -> slotChangeColor(mColor);
  mColorSelector->setValues(c.H(), c.S());
}

/*           RGBWidget         */
RGBWidget::RGBWidget(QWidget *parent) : ColorWidget(parent)
{
  Q3GridLayout *mGrid = new Q3GridLayout(this, 4, 5);

  mColorPatch = new KColorPatch(this);

  /* setup color sliders */
  mRSlider = new KoColorSlider(this);
  mRSlider->setMaximumHeight(20);
  mRSlider->slotSetRange(0, 255);

  mGSlider = new KoColorSlider(this);
  mGSlider->setMaximumHeight(20);
  mGSlider->slotSetRange(0, 255);

  mBSlider = new KoColorSlider(this);
  mBSlider->setMaximumHeight(20);
  mBSlider->slotSetRange(0, 255);

  /* setup slider labels */
  mRLabel = new QLabel("R", this);
  mRLabel->setFixedWidth(16);
  mRLabel->setFixedHeight(20);
  mGLabel = new QLabel("G", this);
  mGLabel->setFixedWidth(16);
  mGLabel->setFixedHeight(20);
  mBLabel = new QLabel("B", this);
  mBLabel->setFixedWidth(16);
  mBLabel->setFixedHeight(20);

  /* setup spin box */
  mRIn = new QSpinBox(0, 255, 1, this);
  mRIn->setFixedWidth(42);
  mRIn->setFixedHeight(20);
  mGIn = new QSpinBox(0, 255, 1, this);
  mGIn->setFixedWidth(42);
  mGIn->setFixedHeight(20);
  mBIn = new QSpinBox(0, 255, 1, this);
  mBIn->setFixedWidth(42);
  mBIn->setFixedHeight(20);

  mGrid->addMultiCellWidget(mColorPatch, 0, 4, 0, 0);
  mGrid->addWidget(mRLabel, 1, 1);
  mGrid->addWidget(mGLabel, 2, 1);
  mGrid->addWidget(mBLabel, 3, 1);
  mGrid->addMultiCellWidget(mRSlider, 1, 1, 2, 3);
  mGrid->addMultiCellWidget(mGSlider, 2, 2, 2, 3);
  mGrid->addMultiCellWidget(mBSlider, 3, 3, 2, 3);
  mGrid->addWidget(mRIn, 1, 4);
  mGrid->addWidget(mGIn, 2, 4);
  mGrid->addWidget(mBIn, 3, 4);

  connect(mColorPatch, SIGNAL(colorChanged(const QColor &)), this, SLOT(slotPatchChanged(const QColor &)));

  /* connect color sliders */
  connect(mRSlider, SIGNAL(valueChanged(int)), this, SLOT(slotRSliderChanged(int)));
  connect(mGSlider, SIGNAL(valueChanged(int)), this, SLOT(slotGSliderChanged(int)));
  connect(mBSlider, SIGNAL(valueChanged(int)), this, SLOT(slotBSliderChanged(int)));

  /* connect spin box */
  connect(mRIn, SIGNAL(valueChanged(int)), this, SLOT(slotRInChanged(int)));
  connect(mGIn, SIGNAL(valueChanged(int)), this, SLOT(slotGInChanged(int)));
  connect(mBIn, SIGNAL(valueChanged(int)), this, SLOT(slotBInChanged(int)));
}

ColorWidget::ColorWidget(QWidget *parent) : QWidget(parent)
{
}

ColorWidget::~ColorWidget()
{
}

void ColorWidget::slotChangeColor(const KoColor& c)
{
  mColor.setRGB(c.R(), c.G(), c.B());
  slotRefreshColor();
}

void ColorWidget::slotChangeColor(const QColor& c)
{
  mColor.setColor(c);
  slotRefreshColor();
}

void RGBWidget::slotRefreshColor()
{
  int r = mColor.R();
  int g = mColor.G();
  int b = mColor.B();

  mRSlider->slotSetColor1(QColor(0, g, b));
  mRSlider->slotSetColor2(QColor(255, g, b));
  mRSlider->slotSetValue(r);
  mRIn->setValue(r);

  mGSlider->slotSetColor1(QColor(r, 0, b));
  mGSlider->slotSetColor2(QColor(r, 255, b));
  mGSlider->slotSetValue(g);
  mGIn->setValue(g);

  mBSlider->slotSetColor1(QColor(r, g, 0));
  mBSlider->slotSetColor2(QColor(r, g, 255));
  mBSlider->slotSetValue(b);
  mBIn->setValue(b);
  mColorPatch -> setColor(mColor.color());
}

void RGBWidget::slotRSliderChanged(int r)
{
  int g = mColor.G();
  int b = mColor.B();

  mColor.setRGB(r, g, b);
  slotRefreshColor();
  emit colorChanged(KoColor(r, g, b, KoColor::csRGB));
}

void RGBWidget::slotGSliderChanged(int g)
{
  int r = mColor.R();
  int b = mColor.B();

  mColor.setRGB(r, g, b);
  slotRefreshColor();
  emit colorChanged(KoColor( r, g, b, KoColor::csRGB));
}

void RGBWidget::slotBSliderChanged(int b)
{
  int r = mColor.R();
  int g = mColor.G();

  mColor.setRGB(r, g, b);
  slotRefreshColor();
  emit colorChanged(KoColor(r, g, b, KoColor::csRGB));
}

void RGBWidget::slotRInChanged(int r)
{
  int g = mColor.G();
  int b = mColor.B();

  mColor.setRGB(r, g, b);
  slotRefreshColor();
  emit colorChanged(KoColor(r, g, b, KoColor::csRGB));
}

void RGBWidget::slotGInChanged(int g)
{
  int r = mColor.R();
  int b = mColor.B();

  mColor.setRGB(r, g, b);
  slotRefreshColor();
  emit colorChanged(KoColor(r, g, b, KoColor::csRGB));
}

void RGBWidget::slotBInChanged(int b)
{
  int r = mColor.R();
  int g = mColor.G();

  mColor.setRGB(r, g, b);
  slotRefreshColor();
  emit colorChanged(KoColor(r, g, b, KoColor::csRGB));
}

void RGBWidget::slotPatchChanged(const QColor& clr)
{
  int r = clr.red();
  int g = clr.green();
  int b = clr.blue();

  mColor.setRGB(r, g, b);
  slotRefreshColor();
  emit colorChanged(KoColor(r, g, b, KoColor::csRGB));
}

/*           HSVWidget         */

HSVWidget::HSVWidget(QWidget *parent): ColorWidget(parent)
{
  Q3GridLayout *mGrid = new Q3GridLayout(this, 3, 3);

  mColorPatch = new KColorPatch(this);

  /* setup color sliders */
  mHSlider = new KoColorSlider(this);
  mHSlider->setMaximumHeight(20);
  mHSlider->slotSetRange(0, 359);

  mSSlider = new KoColorSlider(this);
  mSSlider->setMaximumHeight(20);
  mSSlider->slotSetRange(0, 255);

  mVSlider = new KoColorSlider(this);
  mVSlider->setMaximumHeight(20);
  mVSlider->slotSetRange(0, 255);

  /* setup slider labels */
  mHLabel = new QLabel("H", this);
  mHLabel->setFixedWidth(16);
  mHLabel->setFixedHeight(20);
  mSLabel = new QLabel("S", this);
  mSLabel->setFixedWidth(16);
  mSLabel->setFixedHeight(20);
  mVLabel = new QLabel("V", this);
  mVLabel->setFixedWidth(16);
  mVLabel->setFixedHeight(20);

  /* setup spin box */
  mHIn = new QSpinBox(0, 359, 1, this);
  mHIn->setFixedWidth(42);
  mHIn->setFixedHeight(20);
  mSIn = new QSpinBox(0, 255, 1, this);
  mSIn->setFixedWidth(42);
  mSIn->setFixedHeight(20);
  mVIn = new QSpinBox(0, 255, 1, this);
  mVIn->setFixedWidth(42);
  mVIn->setFixedHeight(20);

  mGrid->addMultiCellWidget(mColorPatch, 0, 4, 0, 0);
  mGrid->addWidget(mHLabel, 1, 1);
  mGrid->addWidget(mSLabel, 2, 1);
  mGrid->addWidget(mVLabel, 3, 1);
  mGrid->addMultiCellWidget(mHSlider, 1, 1, 2, 3);
  mGrid->addMultiCellWidget(mSSlider, 2, 2, 2, 3);
  mGrid->addMultiCellWidget(mVSlider, 3, 3, 2, 3);
  mGrid->addWidget(mHIn, 1, 4);
  mGrid->addWidget(mSIn, 2, 4);
  mGrid->addWidget(mVIn, 3, 4);

  connect(mColorPatch, SIGNAL(colorChanged(const QColor &)), this, SLOT(slotPatchChanged(const QColor &)));

  /* connect color sliders */
  connect(mHSlider, SIGNAL(valueChanged(int)), this, SLOT(slotHSliderChanged(int)));
  connect(mSSlider, SIGNAL(valueChanged(int)), this, SLOT(slotSSliderChanged(int)));
  connect(mVSlider, SIGNAL(valueChanged(int)), this, SLOT(slotVSliderChanged(int)));

  /* connect spin box */
  connect(mHIn, SIGNAL(valueChanged(int)), this, SLOT(slotHInChanged(int)));
  connect(mSIn, SIGNAL(valueChanged(int)), this, SLOT(slotSInChanged(int)));
  connect(mVIn, SIGNAL(valueChanged(int)), this, SLOT(slotVInChanged(int)));
}

void HSVWidget::slotRefreshColor()
{
  int h = mColor.H();
  int s = mColor.S();
  int v = mColor.V();

  mHSlider->slotSetColor1(KoColor(0, s, v, KoColor::csHSV).color());
  mHSlider->slotSetColor2(KoColor(359, s, v, KoColor::csHSV).color());
  mHSlider->slotSetValue(h);
  mHIn->setValue(h);

  mSSlider->slotSetColor1(KoColor(h, 0, v, KoColor::csHSV).color());
  mSSlider->slotSetColor2(KoColor(h, 255, v, KoColor::csHSV).color());
  mSSlider->slotSetValue(s);
  mSIn->setValue(s);

  mVSlider->slotSetColor1(KoColor(h, s, 0, KoColor::csHSV).color());
  mVSlider->slotSetColor2(KoColor(h, s, 255, KoColor::csHSV).color());
  mVSlider->slotSetValue(v);
  mVIn->setValue(v);
  mColorPatch -> setColor(mColor.color());
}

void HSVWidget::slotHSliderChanged(int h)
{
  int v = mColor.V();
  int s = mColor.S();

  mColor.setHSV(h, s, v);
  slotRefreshColor();
  emit colorChanged(mColor);
}

void HSVWidget::slotSSliderChanged(int s)
{
  int h = mColor.H();
  int v = mColor.V();

  mColor.setHSV(h, s, v);
  slotRefreshColor();
  emit colorChanged(mColor);
}

void HSVWidget::slotVSliderChanged(int v)
{
  int h = mColor.H();
  int s = mColor.S();

  mColor.setHSV(h, s, v);
  slotRefreshColor();
  emit colorChanged(mColor);
}

void HSVWidget::slotHInChanged(int h)
{
  int s = mColor.S();
  int v = mColor.V();

  mColor.setHSV(h, s, v);
  slotRefreshColor();
  emit colorChanged(mColor);
}

void HSVWidget::slotSInChanged(int s)
{
  int h = mColor.H();
  int v = mColor.V();

  mColor.setHSV(h, s, v);
  slotRefreshColor();
  emit colorChanged(mColor);
}

void HSVWidget::slotVInChanged(int v)
{
  int h = mColor.H();
  int s = mColor.S();

  mColor.setHSV(h, s, v);
  slotRefreshColor();
  emit colorChanged(mColor);
}

void HSVWidget::slotPatchChanged(const QColor& clr)
{
  int r = clr.red();
  int g = clr.green();
  int b = clr.blue();

  mColor.setRGB(r, g, b);
  slotRefreshColor();
  emit colorChanged(mColor);
}

/*          GreyWidget         */

GreyWidget::GreyWidget(QWidget *parent): ColorWidget(parent)
{
  Q3GridLayout *mGrid = new Q3GridLayout(this, 3, 3);

  mColorPatch = new KColorPatch(this);

  /* setup slider */
  mVSlider = new KoColorSlider(this);
  mVSlider->setMaximumHeight(20);
  mVSlider->slotSetRange(0, 255);
  mVSlider->slotSetColor1(QColor(255, 255, 255));
  mVSlider->slotSetColor2(QColor(0, 0, 0));

  /* setup slider label */
  mVLabel = new QLabel("K", this);
  mVLabel->setFixedWidth(18);
  mVLabel->setFixedHeight(20);

  /* setup spin box */
  mVIn = new QSpinBox(0, 255, 1, this);
  mVIn->setFixedWidth(42);
  mVIn->setFixedHeight(20);

  mGrid->addMultiCellWidget(mColorPatch, 0, 4, 0, 0);
  mGrid->addWidget(mVLabel, 1, 1);
  mGrid->addMultiCellWidget(mVSlider, 1, 1, 2, 3);
  mGrid->addWidget(mVIn, 1, 4);

  connect(mColorPatch, SIGNAL(colorChanged(const QColor &)), this, SLOT(slotPatchChanged(const QColor &)));

  /* connect color slider */
  connect(mVSlider, SIGNAL(valueChanged(int)), this, SLOT(slotVSliderChanged(int)));

  /* connect spin box */
  connect(mVIn, SIGNAL(valueChanged(int)), mVSlider, SLOT(slotSetValue(int)));
}

void GreyWidget::slotRefreshColor()
{
  double v = mColor.R() + mColor.G() + mColor.B();
  v /= 3.0;
  v = 255.0 - v;
  mVIn->setValue(static_cast<int>(v));
  mVSlider->slotSetValue(static_cast<int>(v));
  mColorPatch -> setColor(mColor.color());
}

void GreyWidget::slotVSliderChanged(int v)
{
  v = 255 - v;

  mColor.setRGB(v, v, v);
  slotRefreshColor();
  emit colorChanged(mColor);
}

void GreyWidget::slotVInChanged(int v)
{
  v = 255 - v;

  mColor.setRGB(v, v, v);
  slotRefreshColor();
  emit colorChanged(mColor);
}

void GreyWidget::slotPatchChanged(const QColor& clr)
{
  int gray = qGray(clr.red(), clr.green(), clr.blue());

  mColor.setRGB(gray, gray, gray);
  slotRefreshColor();
  emit colorChanged(mColor);
}

LABWidget::LABWidget(QWidget *parent) : ColorWidget(parent)
{
  Q3GridLayout *mGrid = new Q3GridLayout(this, 4, 5);

  mColorPatch = new KColorPatch(this);

  /* setup color sliders */
  mLSlider = new KoColorSlider(this);
  mLSlider->setMaximumHeight(20);
  mLSlider->slotSetRange(0, 255);

  mASlider = new KoColorSlider(this);
  mASlider->setMaximumHeight(20);
  mASlider->slotSetRange(0, 255);

  mBSlider = new KoColorSlider(this);
  mBSlider->setMaximumHeight(20);
  mBSlider->slotSetRange(0, 255);

  /* setup slider labels */
  mLLabel = new QLabel("L", this);
  mLLabel->setFixedWidth(16);
  mLLabel->setFixedHeight(20);
  mALabel = new QLabel("A", this);
  mALabel->setFixedWidth(16);
  mALabel->setFixedHeight(20);
  mBLabel = new QLabel("B", this);
  mBLabel->setFixedWidth(16);
  mBLabel->setFixedHeight(20);

  /* setup spin box */
  mLIn = new QSpinBox(0, 255, 1, this);
  mLIn->setFixedWidth(42);
  mLIn->setFixedHeight(20);
  mAIn = new QSpinBox(0, 255, 1, this);
  mAIn->setFixedWidth(42);
  mAIn->setFixedHeight(20);
  mBIn = new QSpinBox(0, 255, 1, this);
  mBIn->setFixedWidth(42);
  mBIn->setFixedHeight(20);

  mGrid->addMultiCellWidget(mColorPatch, 0, 4, 0, 0);
  mGrid->addWidget(mLLabel, 1, 1);
  mGrid->addWidget(mALabel, 2, 1);
  mGrid->addWidget(mBLabel, 3, 1);
  mGrid->addMultiCellWidget(mLSlider, 1, 1, 2, 3);
  mGrid->addMultiCellWidget(mASlider, 2, 2, 2, 3);
  mGrid->addMultiCellWidget(mBSlider, 3, 3, 2, 3);
  mGrid->addWidget(mLIn, 1, 4);
  mGrid->addWidget(mAIn, 2, 4);
  mGrid->addWidget(mBIn, 3, 4);

  connect(mColorPatch, SIGNAL(colorChanged(const QColor &)), this, SLOT(slotPatchChanged(const QColor &)));

  /* connect color sliders */
  connect(mLSlider, SIGNAL(valueChanged(int)), this, SLOT(slotLSliderChanged(int)));
  connect(mASlider, SIGNAL(valueChanged(int)), this, SLOT(slotASliderChanged(int)));
  connect(mBSlider, SIGNAL(valueChanged(int)), this, SLOT(slotBSliderChanged(int)));

  /* connect spin box */
  connect(mLIn, SIGNAL(valueChanged(int)), this, SLOT(slotLInChanged(int)));
  connect(mAIn, SIGNAL(valueChanged(int)), this, SLOT(slotAInChanged(int)));
  connect(mBIn, SIGNAL(valueChanged(int)), this, SLOT(slotBInChanged(int)));
}

void LABWidget::slotRefreshColor()
{
  int l = mColor.L();
  int a = mColor.a();
  int b = mColor.b();

  mLSlider->slotSetColor1(KoColor(0, a, b, KoColor::csLab).color());
  mLSlider->slotSetColor2(KoColor(255, a, b, KoColor::csLab).color());
  mLSlider->slotSetValue(l);
  mLIn->setValue(l);

  mASlider->slotSetColor1(KoColor(l, 0, b, KoColor::csLab).color());
  mASlider->slotSetColor2(KoColor(l, 255, b, KoColor::csLab).color());
  mASlider->slotSetValue(a);
  mAIn->setValue(a);

  mBSlider->slotSetColor1(KoColor(l, a, 0, KoColor::csLab).color());
  mBSlider->slotSetColor2(KoColor(l, a, 255, KoColor::csLab).color());
  mBSlider->slotSetValue(b);
  mBIn->setValue(b);
  mColorPatch -> setColor(mColor.color());
}

void LABWidget::slotLSliderChanged(int l)
{
  int a = mColor.a();
  int b = mColor.b();

  mColor.setLab(l, a, b);
  slotRefreshColor();
  emit colorChanged(mColor);

}

void LABWidget::slotASliderChanged(int a)
{
  int l = mColor.L();
  int b = mColor.b();

  mColor.setLab(l, a, b);
  slotRefreshColor();
  emit colorChanged(mColor);
}

void LABWidget::slotBSliderChanged(int b)
{
  int l = mColor.L();
  int a = mColor.a();

  mColor.setLab(l, a, b);
  slotRefreshColor();
  emit colorChanged(mColor);
}

void LABWidget::slotLInChanged(int l)
{
  int a = mColor.a();
  int b = mColor.b();

  mColor.setLab(l, a, b);
  slotRefreshColor();
  emit colorChanged(mColor);
}

void LABWidget::slotAInChanged(int a)
{
  int l = mColor.L();
  int b = mColor.b();

  mColor.setLab(l, a, b);
  slotRefreshColor();
  emit colorChanged(mColor);
}

void LABWidget::slotBInChanged(int b)
{
  int l = mColor.L();
  int a = mColor.a();

  mColor.setLab(l, a, b);
  slotRefreshColor();
  emit colorChanged(mColor);
}

void LABWidget::slotPatchChanged(const QColor& clr)
{
  int r = clr.red();
  int g = clr.green();
  int b = clr.blue();

  mColor.setRGB(r, g, b);
  slotRefreshColor();
  emit colorChanged(mColor);
}

#include "koColorChooser.moc"

