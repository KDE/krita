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

#include "kis_gray_widget.h"

#include <qlayout.h>
#include <qhbox.h>
#include <qlabel.h>
#include <qspinbox.h>
#include <koFrameButton.h>
#include <koColorSlider.h>
#include <kcolordialog.h>
#include <kdualcolorbutton.h>

KisGrayWidget::KisGrayWidget(QWidget *parent) : super(parent)
{  
	m_ColorButton = new KDualColorButton(this);
	m_ColorButton ->  setFixedSize(m_ColorButton->sizeHint());
	QGridLayout *mGrid = new QGridLayout(this, 3, 5, 5, 2);

	/* setup color sliders */
	mSlider = new KoColorSlider(this);
	mSlider->setMaximumHeight(20);
	mSlider->slotSetRange(0, 255);
	mSlider->slotSetColor1(QColor(255, 255, 255));
	mSlider->slotSetColor2(QColor(0, 0, 0));

	/* setup slider labels */
	mLabel = new QLabel("K", this);
	mLabel->setFixedWidth(12);
	mLabel->setFixedHeight(20);

	/* setup spin box */
	mIn = new QSpinBox(0, 255, 1, this);
	mIn->setFixedWidth(50);
	mIn->setFixedHeight(20);

	mGrid->addMultiCellWidget(m_ColorButton, 0, 3, 0, 0, Qt::AlignTop);
	mGrid->addWidget(mLabel, 0, 1);
	mGrid->addMultiCellWidget(mSlider, 0, 0, 2, 3);
	mGrid->addWidget(mIn, 0, 4);

	connect(m_ColorButton, SIGNAL(fgChanged(const QColor &)), this, SLOT(slotFGColorSelected(const QColor &)));
	connect(m_ColorButton, SIGNAL(bgChanged(const QColor &)), this, SLOT(slotBGColorSelected(const QColor &)));

	/* connect color slider */
	connect(mSlider, SIGNAL(valueChanged(int)), this, SLOT(slotChanged(int)));

	/* connect spin box */
	connect(mIn, SIGNAL(valueChanged(int)), this, SLOT(slotChanged(int)));
}

void KisGrayWidget::slotChanged(int v)
{
	v = 255 - v;

	if (m_ColorButton->current() == KDualColorButton::Foreground){
		m_fgColor.setRGB(v, v, v);
		m_ColorButton->setCurrent(KDualColorButton::Foreground);
		emit fgColorChanged(m_fgColor);
	}
	else{
		m_bgColor.setRGB(v, v, v);
		m_ColorButton->setCurrent(KDualColorButton::Background);
		emit bgColorChanged(m_bgColor);
	}
	update();
}

void KisGrayWidget::update()
{
	KoColor color = (m_ColorButton->current() == KDualColorButton::Foreground)? m_fgColor : m_bgColor;

	disconnect(m_ColorButton, SIGNAL(fgChanged(const QColor &)), this, SLOT(slotFGColorSelected(const QColor &)));
	disconnect(m_ColorButton, SIGNAL(bgChanged(const QColor &)), this, SLOT(slotBGColorSelected(const QColor &)));
	disconnect(mSlider, SIGNAL(valueChanged(int)), this, SLOT(slotChanged(int)));
	disconnect(mIn, SIGNAL(valueChanged(int)), this, SLOT(slotChanged(int)));
	m_ColorButton->setForeground( m_fgColor.color() );
	m_ColorButton->setBackground( m_bgColor.color() );

	double v = color.R() + color.G() + color.B();
	v /= 3.0;
	v = 255.0 - v;
	mIn->setValue(static_cast<int>(v));
	mSlider->slotSetValue(static_cast<int>(v));

	connect(m_ColorButton, SIGNAL(fgChanged(const QColor &)), this, SLOT(slotFGColorSelected(const QColor &)));
	connect(m_ColorButton, SIGNAL(bgChanged(const QColor &)), this, SLOT(slotBGColorSelected(const QColor &)));
	connect(mSlider, SIGNAL(valueChanged(int)), this, SLOT(slotChanged(int)));
	connect(mIn, SIGNAL(valueChanged(int)), this, SLOT(slotChanged(int)));
}

void KisGrayWidget::slotFGColorSelected(const QColor& c)
{
	m_fgColor = KoColor(c);
	emit fgColorChanged(m_fgColor);
	update();
}

void KisGrayWidget::slotBGColorSelected(const QColor& c)
{
	m_bgColor = KoColor(c);
	emit bgColorChanged(m_bgColor);
	update();
}

#include "kis_gray_widget.moc"
