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


double NormeCircleSquare(double a, double b)
{
	return a*a + b * b;
}
double NormeSquareSquare(double a, double b)
{
	return QMIN(a*a,b*b);
}

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
	paramChanged();
}

void KisAutobrush::createBrush(Q_INT32 w, Q_INT32 h, Q_INT32 fh, Q_INT32 fv, NormeSquare Norme)
{
	m_brsh->create(w, h, 32 );
	int xcentre = w / 2;
	int ycentre = h / 2;
	double xcoef = ((double)w) /(QMAX(w,h) );
	double ycoef = ((double)h) /( QMAX(w,h) );
	double limit = QMAX(w,h);
	limit *= limit / 4;
	fh = w / 2 - fh;
	fv = h / 2 - fv;
	double xfadecoef = ((double)fh) /(QMAX(fh,fv) );
	double yfadecoef = ((double)fv) /( QMAX(fh,fv) );
	double limitfade = QMAX(fh,fv);
	limitfade *= limitfade;
// 	kdDebug() << "************************************************" << endl;
// 	kdDebug() << "xcentre = " << xcentre << " ycentre = " << ycentre << " xcoef =  " << xcoef << " ycoef = " << ycoef << endl;
	for(int j = 0; j < h; j++)
	{
		for(int i = 0; i < w; i++)
		{
			double norme = (*Norme)( (i - xcentre)*xcoef, (j - ycentre)*ycoef);
			uchar v;
			if( norme > limit )
			{
				v = 255;
			}
			else
			{
				double normeFade = (*Norme)( (i - xcentre) * xfadecoef, (j - ycentre) * yfadecoef );
				if( normeFade > limitfade)
				{
					v = (uchar)(255 * ( normeFade - limitfade) / (limit - limitfade ));
// 					kdDebug() << ">>> v="<< v << " r=" << ( normeFade - limitfade) / (norme - limitfade ) << " normeFade=" << normeFade << " limitFade=" << limitfade << endl;
				} else {
					v = 0;
				}
			}
			m_brsh->setPixel(i,j, qRgb(v,v,v));
// 			kdDebug() << "i=" << i << " j=" << j << "  (i - xcentre)*xcoef = " <<  ((i - xcentre)*xcoef) << "  (j - ycentre)*ycoef = " <<  ((j - ycentre)*ycoef)  << " norme = " << (*Norme)( (i - xcentre)*xcoef, (j - ycentre)*ycoef) << " limit = " << limit << " v=" << v << endl;
		}
	}
}

void KisAutobrush::paramChanged()
{
	NormeSquare f;
	if(comboBoxShape->currentText() == "circle")
	{
		f = &NormeCircleSquare;
	} else {
		f = &NormeSquareSquare;
	}
	Q_INT32 fh = QMIN( spinBoxWidth->value()/2, spinBoxHorizontal->value() ) ;
	Q_INT32 fv = QMIN( spinBoxHeigth->value()/2, spinBoxVertical->value() );
	createBrush( spinBoxWidth->value(),  spinBoxHeigth->value(), fh, fv, f);
	kdDebug() << " brush size : " << m_brsh->width() << " " << m_brsh->height() << endl;
	QPixmap p;
	p.convertFromImage(*m_brsh);
	brushPreview->setPixmap(p);
	brushPreview->adjustSize ();
	KisAutobrushResource * resource = new KisAutobrushResource(*m_brsh);
	emit(activatedResource(resource));
}
void KisAutobrush::spinBoxWidthChanged(int a)
{
	if(checkBoxSamesize->isChecked())
		spinBoxHeigth->setValue(a);
	this->paramChanged();
}
void KisAutobrush::spinBoxHeigthChanged(int a)
{
	if(checkBoxSamesize->isChecked())
		spinBoxWidth->setValue(a);
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
