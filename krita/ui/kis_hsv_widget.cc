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
#include "kis_colorwheel.h"

#include <kselect.h> 
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
	QGridLayout *mGrid = new QGridLayout(this, 4, 5, 5, 2);

	m_colorwheel = new KisColorWheel(this);
	m_colorwheel->setFixedSize( 120, 120); 
	m_VSelector = new KValueSelector(Qt::Vertical, this);
	m_VSelector-> setFixedWidth(30);

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

	mGrid->addMultiCellWidget(m_ColorButton, 0, 0, 0, 0, Qt::AlignTop);
	mGrid->addWidget(mHLabel, 1, 1);
	mGrid->addWidget(mSLabel, 2, 1);
	mGrid->addWidget(mVLabel, 3, 1);
	mGrid->addMultiCellWidget(m_colorwheel, 0, 0, 1, 3);
	mGrid->addWidget(mHIn, 1, 2);
	mGrid->addWidget(mSIn, 2, 2);
	mGrid->addWidget(mVIn, 3, 2);
	mGrid->addMultiCellWidget(m_VSelector, 0, 0, 4, 4);

	connect(m_ColorButton, SIGNAL(fgChanged(const QColor &)), this, SLOT(slotFGColorSelected(const QColor &)));
	connect(m_ColorButton, SIGNAL(bgChanged(const QColor &)), this, SLOT(slotBGColorSelected(const QColor &)));

	connect(m_VSelector, SIGNAL(valueChanged(int)), this, SLOT(slotVChanged(int)));
	connect(m_colorwheel, SIGNAL(valueChanged(const KoColor&)), this, SLOT(slotWheelChanged(const KoColor&)));

	/* connect spin box */
	connect(mHIn, SIGNAL(valueChanged(int)), this, SLOT(slotHChanged(int)));
	connect(mSIn, SIGNAL(valueChanged(int)), this, SLOT(slotSChanged(int)));
	connect(mVIn, SIGNAL(valueChanged(int)), this, SLOT(slotVChanged(int)));
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

void KisHSVWidget::slotWheelChanged(const KoColor& c)
{
	if (m_ColorButton->current() == KDualColorButton::Foreground){
		m_fgColor.setHSV(c.H(), c.S(), m_fgColor.V());
		m_ColorButton->setCurrent(KDualColorButton::Foreground);
		emit fgColorChanged(m_fgColor);
	}
	else{
		m_bgColor.setHSV(c.H(), c.S(), m_fgColor.V());
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

	mHIn->setValue(h);
	mSIn->setValue(s);
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
