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

void KisAutobrushShape::createBrush( QImage* img)
{
	img->create(m_w, m_h, 32);
	for(int j = 0; j < m_h; j++)
	{
		for(int i = 0; i < m_w; i++)
		{
			Q_INT8 v = valueAt(i,j);
			img->setPixel( i, j, qRgb(v,v,v));
		}
	}
}

KisAutobrushCircleShape::KisAutobrushCircleShape(Q_INT32 w, Q_INT32 h, double fh, double fv)
	: KisAutobrushShape( w, h, w / 2.0 - fh, h / 2.0 - fv),
		m_xcentre ( w / 2.0 ),
		m_ycentre ( h / 2.0 ),
		m_xcoef ( 2.0 / w ),
		m_ycoef ( 2.0 / h ),
		m_xfadecoef ( (m_fh == 0) ? 1 : ( 1.0 / m_fh)),
		m_yfadecoef ( (m_fv == 0) ? 1 : ( 1.0 / m_fv))
{
};
Q_INT8 KisAutobrushCircleShape::valueAt(Q_INT32 x, Q_INT32 y)
{
	double xr = (x - m_xcentre);
	double yr = (y - m_ycentre);
	double n = norme( xr * m_xcoef, yr * m_ycoef);
	if( n > 1 )
	{
		return 255;
	}
	else
	{
		double normeFade = norme( xr * m_xfadecoef, yr * m_yfadecoef );
		if( normeFade > 1)
		{
			double xle, yle;
			// xle stands for x-coordinate limit exterior
			// yle stands for y-coordinate limit exterior
			// we are computing the coordinate on the external ellipse in order to compute
			// the fade value
			if( xr == 0 )
			{
				xle = 0;
				yle = yr > 0 ? 1/m_ycoef : -1/m_ycoef;
			} else {
				double c = yr / (double)xr;
				xle = sqrt(1 / norme( m_xcoef, c * m_ycoef ));
				xle = xr > 0 ? xle : -xle;
				yle = xle * c;
			}
			// On the internal limit of the fade area, normeFade is equal to 1
			double normeFadeLimitE = norme( xle * m_xfadecoef, yle * m_yfadecoef );
			return (uchar)(255 * ( normeFade - 1 ) / ( normeFadeLimitE - 1 ));
		} else {
			return 0;
		}
	}
}

KisAutobrushRectShape::KisAutobrushRectShape(Q_INT32 w, Q_INT32 h, double fh, double fv)
	: KisAutobrushShape( w, h, w / 2.0 - fh, h / 2.0 - fv),
		m_xcentre ( w / 2.0 ),
		m_ycentre ( h / 2.0 ),
		m_c( fv/fh)
{
}
Q_INT8 KisAutobrushRectShape::valueAt(Q_INT32 x, Q_INT32 y)
{
	double xr = QABS(x - m_xcentre);
	double yr = QABS(y - m_ycentre);
	if( xr > m_fh || yr > m_fv )
	{
		if( yr <= ((xr - m_fh) * m_c + m_fv )  )
		{
			return (uchar)(255 * (xr - m_fh) / (m_w - m_fh));
		} else {
			return (uchar)(255 * (yr - m_fv) / (m_w - m_fv));
		}
	}
	else {
		return 0;
	}
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



void KisAutobrush::paramChanged()
{
	Q_INT32 fh = QMIN( spinBoxWidth->value()/2, spinBoxHorizontal->value() ) ;
	Q_INT32 fv = QMIN( spinBoxHeigth->value()/2, spinBoxVertical->value() );
	KisAutobrushShape* kas;
	if(comboBoxShape->currentText() == "circle")
	{
		kas = new KisAutobrushCircleShape(spinBoxWidth->value(),  spinBoxHeigth->value(), fh, fv);
	} else {
		kas = new KisAutobrushRectShape(spinBoxWidth->value(),  spinBoxHeigth->value(), fh, fv);
	}
	kas->createBrush(m_brsh);
	kdDebug() << " brush size : " << m_brsh->width() << " " << m_brsh->height() << endl;
	QPixmap p;
	p.convertFromImage(*m_brsh);
	brushPreview->setPixmap(p);
	brushPreview->adjustSize ();
	KisAutobrushResource * resource = new KisAutobrushResource(*m_brsh);
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
