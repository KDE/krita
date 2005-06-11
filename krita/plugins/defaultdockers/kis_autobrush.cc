/*
 *  Copyright (c) 2004 Cyrille Berger <cberger@cberger.net>
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
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */
 
#include "kis_autobrush.h"
#include <kdebug.h>
#include <qspinbox.h>
#include <qcheckbox.h>
#include <qimage.h>
#include <qcombobox.h>
#include <qlabel.h>


KisAutobrush::KisAutobrush(QWidget *parent, const char* name, const QString& caption) : KisWdgAutobrush(parent, name)
{
	setCaption(caption);
	connect((QObject*)comboBoxShape, SIGNAL(activated(int)), this, SLOT(paramChanged()));
	spinBoxWidth->setMinValue(1);
	connect(spinBoxWidth,SIGNAL(valueChanged(int)),this,SLOT(spinBoxWidthChanged(int)));
	spinBoxHeigth->setMinValue(1);
	connect(spinBoxHeigth,SIGNAL(valueChanged(int)),this,SLOT(spinBoxHeigthChanged(int)));
	spinBoxHorizontal->setMinValue(0);
	connect(spinBoxHorizontal,SIGNAL(valueChanged(int)),this,SLOT(spinBoxHorizontalChanged(int)));
	spinBoxVertical->setMinValue(0);
	connect(spinBoxVertical,SIGNAL(valueChanged(int)),this,SLOT(spinBoxVerticalChanged(int)));

	m_brsh = new QImage(1,1,32);
	Q_CHECK_PTR(m_brsh);

	paramChanged();
}



void KisAutobrush::paramChanged()
{
	Q_INT32 fh = QMIN( spinBoxWidth->value()/2, spinBoxHorizontal->value() ) ;
	Q_INT32 fv = QMIN( spinBoxHeigth->value()/2, spinBoxVertical->value() );
	KisAutobrushShape* kas;

	if(comboBoxShape->currentItem() == 0) // use index compare instead of comparing a translatable string
	{
		kas = new KisAutobrushCircleShape(spinBoxWidth->value(),  spinBoxHeigth->value(), fh, fv);
		Q_CHECK_PTR(kas);

	} else {
		kas = new KisAutobrushRectShape(spinBoxWidth->value(),  spinBoxHeigth->value(), fh, fv);
		Q_CHECK_PTR(kas);

	}
	kas->createBrush(m_brsh);
// 	kdDebug() << " brush size : " << m_brsh->width() << " " << m_brsh->height() << endl;
	QPixmap p;
	p.convertFromImage(*m_brsh);
	brushPreview->setPixmap(p);
	//brushPreview->adjustSize ();
	KisAutobrushResource * resource = new KisAutobrushResource(*m_brsh);
	Q_CHECK_PTR(resource);

	emit(activatedResource(resource));
	delete kas;
}
void KisAutobrush::spinBoxWidthChanged(int a)
{
	spinBoxHorizontal->setMaxValue(a/2);
	if(checkBoxSamesize->isChecked())
	{
		spinBoxHeigth->setValue(a);
		spinBoxVertical->setMaxValue(a/2);
	}
	this->paramChanged();
}
void KisAutobrush::spinBoxHeigthChanged(int a)
{
	spinBoxVertical->setMaxValue(a/2);
	if(checkBoxSamesize->isChecked())
	{
		spinBoxWidth->setValue(a);
		spinBoxHorizontal->setMaxValue(a/2);
	}
	this->paramChanged();
}
void KisAutobrush::spinBoxHorizontalChanged(int a)
{
	if(checkBoxSamefade->isChecked())
		spinBoxVertical->setValue(a);
	this->paramChanged();
}
void KisAutobrush::spinBoxVerticalChanged(int a)
{
	if(checkBoxSamefade->isChecked())
		spinBoxHorizontal->setValue(a);
	this->paramChanged();
}

#include "kis_autobrush.moc"
