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

#include "kis_rgb_widget.h"

#include <qlayout.h>
#include <qhbox.h>
#include <qlabel.h>
#include <qspinbox.h>
#include <koFrameButton.h>
#include <koColorSlider.h>
#include <kcolordialog.h>
#include <kdualcolorbutton.h>

KisRGBWidget::KisRGBWidget(QWidget *parent) : super(parent)
{  
	m_ColorButton = new KDualColorButton(this);
	m_ColorButton ->  setFixedSize(m_ColorButton->sizeHint());
	QGridLayout *mGrid = new QGridLayout(this, 3, 5, 5, 2);

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
	mRLabel->setFixedWidth(12);
	mRLabel->setFixedHeight(20);
	mGLabel = new QLabel("G", this);
	mGLabel->setFixedWidth(12);
	mGLabel->setFixedHeight(20);
	mBLabel = new QLabel("B", this);
	mBLabel->setFixedWidth(12);
	mBLabel->setFixedHeight(20);

	/* setup spin box */
	mRIn = new QSpinBox(0, 255, 1, this);
	mRIn->setFixedWidth(50);
	mRIn->setFixedHeight(20);
	mGIn = new QSpinBox(0, 255, 1, this);
	mGIn->setFixedWidth(50);
	mGIn->setFixedHeight(20);
	mBIn = new QSpinBox(0, 255, 1, this);
	mBIn->setFixedWidth(50);
	mBIn->setFixedHeight(20);

	mGrid->addMultiCellWidget(m_ColorButton, 0, 3, 0, 0, Qt::AlignTop);
	mGrid->addWidget(mRLabel, 0, 1);
	mGrid->addWidget(mGLabel, 1, 1);
	mGrid->addWidget(mBLabel, 2, 1);
	mGrid->addMultiCellWidget(mRSlider, 0, 0, 2, 3);
	mGrid->addMultiCellWidget(mGSlider, 1, 1, 2, 3);
	mGrid->addMultiCellWidget(mBSlider, 2, 2, 2, 3);
	mGrid->addWidget(mRIn, 0, 4);
	mGrid->addWidget(mGIn, 1, 4);
	mGrid->addWidget(mBIn, 2, 4);

	connect(m_ColorButton, SIGNAL(fgChanged(const QColor &)), this, SLOT(slotFGColorSelected(const QColor &)));
	connect(m_ColorButton, SIGNAL(bgChanged(const QColor &)), this, SLOT(slotBGColorSelected(const QColor &)));

	/* connect color sliders */
	connect(mRSlider, SIGNAL(valueChanged(int)), this, SLOT(slotRChanged(int)));
	connect(mGSlider, SIGNAL(valueChanged(int)), this, SLOT(slotGChanged(int)));
	connect(mBSlider, SIGNAL(valueChanged(int)), this, SLOT(slotBChanged(int)));

	/* connect spin box */
	connect(mRIn, SIGNAL(valueChanged(int)), this, SLOT(slotRChanged(int)));
	connect(mGIn, SIGNAL(valueChanged(int)), this, SLOT(slotGChanged(int)));
	connect(mBIn, SIGNAL(valueChanged(int)), this, SLOT(slotBChanged(int)));
}

void KisRGBWidget::slotRChanged(int r)
{
	if (m_ColorButton->current() == KDualColorButton::Foreground){
		m_fgColor.setRGB(r, m_fgColor.G(), m_fgColor.B());
		m_ColorButton->setCurrent(KDualColorButton::Foreground);
		emit fgColorChanged(m_fgColor);
	}
	else{
		m_bgColor.setRGB(r, m_bgColor.G(), m_bgColor.B());
		m_ColorButton->setCurrent(KDualColorButton::Background);
		emit bgColorChanged(m_bgColor);
	}
	update();
}

void KisRGBWidget::slotGChanged(int g)
{
	if (m_ColorButton->current() == KDualColorButton::Foreground){
		m_fgColor.setRGB(m_fgColor.R(), g, m_fgColor.B());
		m_ColorButton->setCurrent(KDualColorButton::Foreground);
		emit fgColorChanged(m_fgColor);
	}
	else{
		m_bgColor.setRGB(m_bgColor.R(), g, m_bgColor.B());
		m_ColorButton->setCurrent(KDualColorButton::Background);
		emit bgColorChanged(m_bgColor);
	}
	update();
}

void KisRGBWidget::slotBChanged(int b)
{
	if (m_ColorButton->current() == KDualColorButton::Foreground){
		m_fgColor.setRGB(m_fgColor.R(), m_fgColor.G(), b);
		m_ColorButton->setCurrent(KDualColorButton::Foreground);
		emit fgColorChanged(m_fgColor);
	}
	else{
		m_bgColor.setRGB(m_bgColor.R(), m_bgColor.G(), b);
		m_ColorButton->setCurrent(KDualColorButton::Background);
		emit bgColorChanged(m_bgColor);
	}
	update();
}

void KisRGBWidget::update()
{
	KoColor color = (m_ColorButton->current() == KDualColorButton::Foreground)? m_fgColor : m_bgColor;

	int r = color.R();
	int g = color.G();
	int b = color.B();

	disconnect(m_ColorButton, SIGNAL(fgChanged(const QColor &)), this, SLOT(slotFGColorSelected(const QColor &)));
	disconnect(m_ColorButton, SIGNAL(bgChanged(const QColor &)), this, SLOT(slotBGColorSelected(const QColor &)));
	
	m_ColorButton->setForeground( m_fgColor.color() );
	m_ColorButton->setBackground( m_bgColor.color() );

	connect(m_ColorButton, SIGNAL(fgChanged(const QColor &)), this, SLOT(slotFGColorSelected(const QColor &)));
	connect(m_ColorButton, SIGNAL(bgChanged(const QColor &)), this, SLOT(slotBGColorSelected(const QColor &)));

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
}

void KisRGBWidget::slotFGColorSelected(const QColor& c)
{
	m_fgColor = KoColor(c);
	emit fgColorChanged(m_fgColor);
	update();
}

void KisRGBWidget::slotBGColorSelected(const QColor& c)
{
	m_bgColor = KoColor(c);
	emit bgColorChanged(m_bgColor);
	update();
}

#include "kis_rgb_widget.moc"
