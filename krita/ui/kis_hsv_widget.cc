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
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

#include "kis_hsv_widget.h"

#include <qlayout.h>
#include <qhbox.h>
#include <qlabel.h>
#include <qspinbox.h>
#include <koFrameButton.h>
#include <koColorSlider.h>
#include <kcolordialog.h>
#include <kdualcolorbutton.h>

KisHSVWidget::KisHSVWidget(QWidget *parent) : super(parent)
{  
	m_ColorButton = new KDualColorButton(this);
	m_ColorButton ->  setFixedSize(m_ColorButton->sizeHint());
	QGridLayout *mGrid = new QGridLayout(this, 3, 5, 5, 2);

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
	mHLabel->setFixedWidth(12);
	mHLabel->setFixedHeight(20);
	mSLabel = new QLabel("S", this);
	mSLabel->setFixedWidth(12);
	mSLabel->setFixedHeight(20);
	mVLabel = new QLabel("V", this);
	mVLabel->setFixedWidth(12);
	mVLabel->setFixedHeight(20);

	/* setup spin box */
	mHIn = new QSpinBox(0, 359, 1, this);
	mHIn->setFixedWidth(50);
	mHIn->setFixedHeight(20);
	mSIn = new QSpinBox(0, 255, 1, this);
	mSIn->setFixedWidth(50);
	mSIn->setFixedHeight(20);
	mVIn = new QSpinBox(0, 255, 1, this);
	mVIn->setFixedWidth(50);
	mVIn->setFixedHeight(20);

	mGrid->addMultiCellWidget(m_ColorButton, 0, 3, 0, 0, Qt::AlignTop);
	mGrid->addWidget(mHLabel, 0, 1);
	mGrid->addWidget(mSLabel, 1, 1);
	mGrid->addWidget(mVLabel, 2, 1);
	mGrid->addMultiCellWidget(mHSlider, 0, 0, 2, 3);
	mGrid->addMultiCellWidget(mSSlider, 1, 1, 2, 3);
	mGrid->addMultiCellWidget(mVSlider, 2, 2, 2, 3);
	mGrid->addWidget(mHIn, 0, 4);
	mGrid->addWidget(mSIn, 1, 4);
	mGrid->addWidget(mVIn, 2, 4);

	connect(m_ColorButton, SIGNAL(fgChanged(const QColor &)), this, SLOT(slotFGColorSelected(const QColor &)));
	connect(m_ColorButton, SIGNAL(bgChanged(const QColor &)), this, SLOT(slotBGColorSelected(const QColor &)));

	/* connect color sliders */
	connect(mHSlider, SIGNAL(valueChanged(int)), this, SLOT(slotHChanged(int)));
	connect(mSSlider, SIGNAL(valueChanged(int)), this, SLOT(slotSChanged(int)));
	connect(mVSlider, SIGNAL(valueChanged(int)), this, SLOT(slotVChanged(int)));

	/* connect spin box */
	connect(mHIn, SIGNAL(valueChanged(int)), this, SLOT(slotHChanged(int)));
	connect(mSIn, SIGNAL(valueChanged(int)), this, SLOT(slotSChanged(int)));
	connect(mVIn, SIGNAL(valueChanged(int)), this, SLOT(slotVChanged(int)));
}

void KisHSVWidget::slotSetFGColor(const KoColor& c)
{
	m_fgColor = c;
	m_ColorButton->setCurrent(KDualColorButton::Foreground);
	update();
}

void KisHSVWidget::slotSetBGColor(const KoColor& c)
{
	m_bgColor = c;
	m_ColorButton->setCurrent(KDualColorButton::Background);
	update();
}

void KisHSVWidget::slotHChanged(int h)
{
	if (m_ColorButton->current() == KDualColorButton::Foreground){
		m_fgColor.setHSV(h, m_fgColor.S(), m_fgColor.V());
		m_ColorButton->setCurrent(KDualColorButton::Foreground);
		emit fgColorChanged(m_fgColor);
	}
	else{
		m_bgColor.setHSV(h, m_bgColor.S(), m_bgColor.V());
		m_ColorButton->setCurrent(KDualColorButton::Background);
		emit bgColorChanged(m_bgColor);
	}
	update();
}

void KisHSVWidget::slotSChanged(int s)
{
	if (m_ColorButton->current() == KDualColorButton::Foreground){
		m_fgColor.setHSV(m_fgColor.H(), s, m_fgColor.V());
		m_ColorButton->setCurrent(KDualColorButton::Foreground);
		emit fgColorChanged(m_fgColor);
	}
	else{
		m_bgColor.setHSV(m_bgColor.H(), s, m_bgColor.V());
		m_ColorButton->setCurrent(KDualColorButton::Background);
		emit bgColorChanged(m_bgColor);
	}
	update();
}

void KisHSVWidget::slotVChanged(int v)
{
	if (m_ColorButton->current() == KDualColorButton::Foreground){
		m_fgColor.setHSV(m_fgColor.H(), m_fgColor.S(), v);
		m_ColorButton->setCurrent(KDualColorButton::Foreground);
		emit fgColorChanged(m_fgColor);
	}
	else{
		m_bgColor.setHSV(m_bgColor.H(), m_bgColor.S(), v);
		m_ColorButton->setCurrent(KDualColorButton::Background);
		emit bgColorChanged(m_bgColor);
	}
	update();
}

void KisHSVWidget::update()
{
	KoColor color = (m_ColorButton->current() == KDualColorButton::Foreground)? m_fgColor : m_bgColor;

	int h = color.H();
	int s = color.S();
	int v = color.V();

	disconnect(m_ColorButton, SIGNAL(fgChanged(const QColor &)), this, SLOT(slotFGColorSelected(const QColor &)));
	disconnect(m_ColorButton, SIGNAL(bgChanged(const QColor &)), this, SLOT(slotBGColorSelected(const QColor &)));
	
	m_ColorButton->setForeground( m_fgColor.color() );
	m_ColorButton->setBackground( m_bgColor.color() );

	connect(m_ColorButton, SIGNAL(fgChanged(const QColor &)), this, SLOT(slotFGColorSelected(const QColor &)));
	connect(m_ColorButton, SIGNAL(bgChanged(const QColor &)), this, SLOT(slotBGColorSelected(const QColor &)));

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
}

void KisHSVWidget::slotFGColorSelected(const QColor& c)
{
	m_fgColor = KoColor(c);
	emit fgColorChanged(m_fgColor);
	update();
}

void KisHSVWidget::slotBGColorSelected(const QColor& c)
{
	m_bgColor = KoColor(c);
	emit bgColorChanged(m_bgColor);
	update();
}

#include "kis_hsv_widget.moc"
