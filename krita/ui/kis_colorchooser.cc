/*
 *  kis_colorchooser.cc - part of Krayon
 *
 *  Copyright (c) 1999 Matthias Elter  <elter@kde.org>
 *  Copyright (c) 2001 John Califf  <jcaliff@compuzone.net>
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
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include <qframe.h>
#include <qpushbutton.h>
#include <qpainter.h>
#include <qlayout.h>
#include <qlabel.h>
#include <qspinbox.h>
#include <qbuttongroup.h>

#include <klocale.h>
#include <kdebug.h>
#include <kglobal.h>
#include <kimageeffect.h>
#include <kpalette.h>

#include "colorframe.h"
#include "colorslider.h"
#include "kis_colorchooser.h"

const int lh = 18;

KisColorChooser::KisColorChooser(QWidget *parent) : QWidget(parent)
{
    // init with defaults
    m_fg = KisColor::white();
    m_bg = KisColor::black();
    m_active = ac_Foreground;
  
    // setup color frame
    m_pColorFrame = new ColorFrame(this);

    // connect color frame
    connect(m_pColorFrame, SIGNAL(colorSelected(const QColor &)), this,
		  SLOT(slotColorFrameChanged(const QColor &)));

    // setup color widgets
    m_pRGBWidget = new RGBWidget(this);
    m_pGreyWidget = new GreyWidget(this);

    // connect color widgets
    connect(m_pRGBWidget, SIGNAL(colorChanged(const KisColor &)), this,
		  SLOT(slotRGBWidgetChanged(const KisColor &)));

    //connect(m_pGreyWidget, SIGNAL(colorChanged(const KisColor &)), this,
    //		  SLOT(slotGreyWidgetChanged(const KisColor &)));

    // show RGB as default 
    slotShowRGB();
}


void KisColorChooser::slotRGBWidgetChanged(const KisColor& c)
{
    if( m_active == ac_Foreground )
	    m_pColorFrame->slotSetColor1(c.color());
    else
	    m_pColorFrame->slotSetColor2(c.color());

    emit colorChanged(c);
    m_pGreyWidget->slotSetColor(c);
}


void KisColorChooser::slotGreyWidgetChanged(const KisColor& c)
{
    if( m_active == ac_Foreground )
	    m_pColorFrame->slotSetColor1(c.color());
    else
	    m_pColorFrame->slotSetColor2(c.color());

    emit colorChanged(c);
    m_pRGBWidget->slotSetColor(c);
}


void KisColorChooser::slotColorFrameChanged(const QColor& qc)
{
    KisColor c(qc);

    emit colorChanged(c);

    m_pGreyWidget->slotSetColor(c);
    m_pRGBWidget->slotSetColor(c);
}


void KisColorChooser::slotSetFGColor(const KisColor& c)
{
    m_fg = c;

    if (m_active == ac_Foreground)
	{
	    m_pColorFrame->slotSetColor1(c.color());
  	    m_pRGBWidget->slotSetColor(c);
	    m_pGreyWidget->slotSetColor(c);
	}
}


void KisColorChooser::slotSetBGColor(const KisColor& c)
{
    m_bg = c;

    if (m_active == ac_Background)
	{
	    m_pColorFrame->slotSetColor1(c.color());
	    m_pRGBWidget->slotSetColor(c);
	    m_pGreyWidget->slotSetColor(c);
	}
}


void KisColorChooser::slotShowGrey()
{
    m_pRGBWidget->hide();
    m_pGreyWidget->show();
}


void KisColorChooser::slotShowRGB()
{
    m_pGreyWidget->hide();
    m_pRGBWidget->show();
}


void KisColorChooser::slotShowHSB()
{
    m_pGreyWidget->hide();
    m_pRGBWidget->hide();
}


void KisColorChooser::slotShowCMYK()
{
    m_pGreyWidget->hide();
    m_pRGBWidget->hide();
}


void KisColorChooser::slotShowLAB()
{
    m_pGreyWidget->hide();
    m_pRGBWidget->hide();
}


void KisColorChooser::resizeEvent(QResizeEvent *)
{
    int w = width();
    int h = height();

    m_pRGBWidget->setGeometry( 2, 2,     w-4,  h-16);
    m_pGreyWidget->setGeometry(2, 2,     w-4,  h/2);
    m_pColorFrame->setGeometry(2, h-14,  w-4,  12);    
}


RGBWidget::RGBWidget(QWidget *parent) : QWidget(parent)
{
    // init with defaults
    m_c = KisColor::white();

    // setup RGB color sliders
    m_pRSlider = new ColorSlider(this);
    m_pRSlider->setMaximumHeight(lh);
    m_pRSlider->slotSetRange(0, 255);

    m_pGSlider = new ColorSlider(this);
    m_pGSlider->setMaximumHeight(lh);
    m_pGSlider->slotSetRange(0, 255);

    m_pBSlider = new ColorSlider(this);
    m_pBSlider->setMaximumHeight(lh);
    m_pBSlider->slotSetRange(0, 255);
 
    // setup HSV color sliders
    m_pHSlider = new ColorSlider(this, 1);
    m_pHSlider->setMaximumHeight(lh);
    m_pHSlider->slotSetRange(0, 359);

    m_pSSlider = new ColorSlider(this, 2);
    m_pSSlider->setMaximumHeight(lh);
    m_pSSlider->slotSetRange(0, 255);

    m_pVSlider = new ColorSlider(this, 3);
    m_pVSlider->setMaximumHeight(lh);
    m_pVSlider->slotSetRange(0, 255);

    // setup RGB slider labels
    m_pRLabel = new QLabel("R", this);
    m_pRLabel->setFixedWidth(16);
    m_pRLabel->setFixedHeight(lh);
    m_pGLabel = new QLabel("G", this);
    m_pGLabel->setFixedWidth(16);
    m_pGLabel->setFixedHeight(lh);
    m_pBLabel = new QLabel("B", this);
    m_pBLabel->setFixedWidth(16);
    m_pBLabel->setFixedHeight(lh);
 
    // setup HSV slider labels
    m_pHLabel = new QLabel("H", this);
    m_pHLabel->setFixedWidth(16);
    m_pHLabel->setFixedHeight(lh);
    m_pSLabel = new QLabel("S", this);
    m_pSLabel->setFixedWidth(16);
    m_pSLabel->setFixedHeight(lh);
    m_pVLabel = new QLabel("V", this);
    m_pVLabel->setFixedWidth(16);
    m_pVLabel->setFixedHeight(lh);

    // setup RGB spin box
    m_pRIn = new QSpinBox(0, 255, 1, this);
    m_pRIn->setFixedWidth(42);
    m_pRIn->setFixedHeight(lh);
    m_pGIn = new QSpinBox(0, 255, 1, this);
    m_pGIn->setFixedWidth(42);
    m_pGIn->setFixedHeight(lh);
    m_pBIn = new QSpinBox(0, 255, 1, this);
    m_pBIn->setFixedWidth(42);
    m_pBIn->setFixedHeight(lh);

    // setup HSV spin box
    m_pHIn = new QSpinBox(0, 359, 1, this);
    m_pHIn->setFixedWidth(42);
    m_pHIn->setFixedHeight(lh);
    m_pSIn = new QSpinBox(0, 255, 1, this);
    m_pSIn->setFixedWidth(42);
    m_pSIn->setFixedHeight(lh);
    m_pVIn = new QSpinBox(0, 255, 1, this);
    m_pVIn->setFixedWidth(42);
    m_pVIn->setFixedHeight(lh);

    // RGB complementary sliders & spins

    // connect color sliders
    connect(m_pRSlider, SIGNAL(valueChanged(int)), this,
		  SLOT(slotRSliderChanged(int)));
    connect(m_pGSlider, SIGNAL(valueChanged(int)), this,
		  SLOT(slotGSliderChanged(int)));
    connect(m_pBSlider, SIGNAL(valueChanged(int)), this,
		  SLOT(slotBSliderChanged(int)));

    // connect spin box
    connect(m_pRIn, SIGNAL(valueChanged (int)), this,
		  SLOT(slotRInChanged(int)));
    connect(m_pGIn, SIGNAL(valueChanged (int)), this,
		  SLOT(slotGInChanged(int)));
    connect(m_pBIn, SIGNAL(valueChanged (int)), this,
		  SLOT(slotBInChanged(int)));

    // HSV complementary sliders & spins

    // connect color sliders
    connect(m_pHSlider, SIGNAL(valueChanged(int)), this,
		  SLOT(slotHSliderChanged(int)));
    connect(m_pSSlider, SIGNAL(valueChanged(int)), this,
		  SLOT(slotSSliderChanged(int)));
    connect(m_pVSlider, SIGNAL(valueChanged(int)), this,
		  SLOT(slotVSliderChanged(int)));

    // connect spin box
    connect(m_pHIn, SIGNAL(valueChanged (int)), this,
		  SLOT(slotHInChanged(int)));
    connect(m_pSIn, SIGNAL(valueChanged (int)), this,
		  SLOT(slotSInChanged(int)));
    connect(m_pVIn, SIGNAL(valueChanged (int)), this,
		  SLOT(slotVInChanged(int)));
}


void RGBWidget::resizeEvent(QResizeEvent *)
{
    // I know a QGridLayout would look nicer,
    // but it does not use the space as good as I want it to.

    // ############# RGB section ############
    
    int halfheight = height()/2;
    int middle = halfheight/2;   
    int sliderheight = m_pRSlider->height();
    
    m_pRLabel->move(2, 2);
    m_pGLabel->move(2, middle - m_pRLabel->height()/2);
    m_pBLabel->move(2, halfheight - 2 - m_pRLabel->height());

    int x1 = m_pRLabel->pos().x() + m_pRLabel->width();

    m_pRIn->move(width() - m_pRIn->width(),  2);
    m_pGIn->move(width() - m_pRIn->width(), middle - m_pRIn->height()/2 );
    m_pBIn->move(width() - m_pRIn->width(), halfheight - 2 - m_pRIn->height());

    int x2 = width() - m_pRIn->width() - 2;

    m_pRSlider->resize(QSize(x2 - x1, sliderheight));
    m_pGSlider->resize(QSize(x2 - x1, sliderheight));
    m_pBSlider->resize(QSize(x2 - x1, sliderheight));

    m_pRSlider->move(x1,  2 );
    m_pGSlider->move(x1, middle - m_pRSlider->height()/2);
    m_pBSlider->move(x1, halfheight - m_pRSlider->height() - 2);
    
    // ############# HSV section ############

    int yHSV = height()/2; 

    m_pHLabel->move(2, 2 + yHSV);
    m_pSLabel->move(2, middle - m_pRLabel->height()/2 + yHSV);
    m_pVLabel->move(2, halfheight - 2 - m_pRLabel->height() + yHSV);

    m_pHIn->move(width() - m_pRIn->width(),  2 + yHSV);
    m_pSIn->move(width() - m_pRIn->width(), middle - m_pRIn->height()/2 + yHSV);
    m_pVIn->move(width() - m_pRIn->width(), halfheight - 2 - m_pRIn->height() + yHSV);

    m_pHSlider->resize(QSize(x2 - x1, sliderheight));
    m_pSSlider->resize(QSize(x2 - x1, sliderheight));
    m_pVSlider->resize(QSize(x2 - x1, sliderheight));

    m_pHSlider->move(x1,  2 + yHSV);
    m_pSSlider->move(x1, middle - m_pRSlider->height()/2 + yHSV);
    m_pVSlider->move(x1, halfheight - m_pRSlider->height() - 2 + yHSV);
}

/*
     set Color for both RGB and HSV widgets when color changes
*/    
void RGBWidget::slotSetColor(const KisColor&c)
{
    // set color member
    m_c = c;

    // RGB settings
    int r = c.R();
    int g = c.G();
    int b = c.B();
    
    m_pRSlider->slotSetColor1( QColor( 0, g, b ) );
    m_pRSlider->slotSetColor2( QColor( 255, g, b ) );
    m_pRSlider->slotSetValue( r );
    m_pRIn->setValue( r );

    m_pGSlider->slotSetColor1( QColor( r, 0, b ) );
    m_pGSlider->slotSetColor2( QColor( r, 255, b ) );
    m_pGSlider->slotSetValue( g );
    m_pGIn->setValue( g );

    m_pBSlider->slotSetColor1( QColor( r, g, 0 ) );
    m_pBSlider->slotSetColor2( QColor( r, g, 255 ) );
    m_pBSlider->slotSetValue( b );
    m_pBIn->setValue( b );
    
    // HSV settings
    int h = c.h();
    int s = c.s();
    int v = c.v();
    
    // hue slide has a constant pixmap - never changes
    m_pHSlider->slotSetValue( h );    
    m_pHIn->setValue( h );

    // saturation slider is colored by hue
    m_pSSlider->slotSetHue( h );
    m_pSSlider->slotSetValue( s );
    m_pSIn->setValue( s );

    // value slider is colored by hue also
    m_pVSlider->slotSetHue( h );
    m_pVSlider->slotSetValue( v );
    m_pVIn->setValue( v );
}
  

// RGB slider slots

void RGBWidget::slotRSliderChanged(int r)
{
    int g = m_c.G();
    int b = m_c.B();

    m_c = KisColor( r, g, b, cs_RGB );

    m_pGSlider->slotSetColor1( QColor( r, 0, b ) );
    m_pGSlider->slotSetColor2( QColor( r, 255, b ) );

    m_pBSlider->slotSetColor1( QColor( r, g, 0 ) );
    m_pBSlider->slotSetColor2( QColor( r, g, 255 ) );

    m_pRIn->setValue( r );
    emit colorChanged(m_c);
}


void RGBWidget::slotGSliderChanged(int g)
{
    int r = m_c.R();
    int b = m_c.B();

    m_c = KisColor( r, g, b, cs_RGB );

    m_pRSlider->slotSetColor1( QColor( 0, g, b ) );
    m_pRSlider->slotSetColor2( QColor( 255, g, b ) );

    m_pBSlider->slotSetColor1( QColor( r, g, 0 ) );
    m_pBSlider->slotSetColor2( QColor( r, g, 255 ) );

    m_pGIn->setValue( g );
    emit colorChanged(m_c);
}


void RGBWidget::slotBSliderChanged(int b)
{
    int r = m_c.R();
    int g = m_c.G();

    m_c = KisColor( r, g, b, cs_RGB );

    m_pRSlider->slotSetColor1( QColor( 0, g, b ) );
    m_pRSlider->slotSetColor2( QColor( 255, g, b ) );

    m_pGSlider->slotSetColor1( QColor( r, 0, b ) );
    m_pGSlider->slotSetColor2( QColor( r, 255, b ) );

    m_pBIn->setValue( b );
    emit colorChanged(m_c);
}


// HSV Slider slots - note : we have subclassed colorslider 
// and colorframe here for hue, saturation and value

void RGBWidget::slotHSliderChanged(int h)
{
    int s = m_c.s();
    int v = m_c.v();

    m_c = KisColor( h, s, v, cs_HSV );

    kdDebug() << " slotHSliderChanged: hue is " << h << endl;
     
    if(m_pSSlider)
    {
        m_pSSlider->slotSetHue( h );
    }
        
    if(m_pVSlider)
    {
        m_pVSlider->slotSetHue( h );
    }

    m_pHIn->setValue( h );
    emit colorChanged(m_c);
}

/*
    Saturation slider changed
*/
void RGBWidget::slotSSliderChanged(int s)
{
    int h = m_c.h();
    int v = m_c.v();

    m_c = KisColor( h, s, v, cs_HSV );

    m_pSIn->setValue( s );
    emit colorChanged(m_c);
}


void RGBWidget::slotVSliderChanged(int v)
{
    int h = m_c.h();
    int s = m_c.s();

    m_c = KisColor( h, s, v, cs_HSV );

    m_pVIn->setValue( v );
    emit colorChanged(m_c);
}


void RGBWidget::slotRInChanged(int r)
{
    int g = m_c.G();
    int b = m_c.B();

    m_c = KisColor( r, g, b, cs_RGB );

    m_pGSlider->slotSetColor1( QColor( r, 0, b ) );
    m_pGSlider->slotSetColor2( QColor( r, 255, b ) );

    m_pBSlider->slotSetColor1( QColor( r, g, 0 ) );
    m_pBSlider->slotSetColor2( QColor( r, g, 255 ) );

    m_pRSlider->slotSetValue( r );
    emit colorChanged(m_c);
}


void RGBWidget::slotGInChanged(int g)
{
    int r = m_c.R();
    int b = m_c.B();

    m_c = KisColor( r, g, b, cs_RGB );

    m_pRSlider->slotSetColor1( QColor( 0, g, b ) );
    m_pRSlider->slotSetColor2( QColor( 255, g, b ) );

    m_pBSlider->slotSetColor1( QColor( r, g, 0 ) );
    m_pBSlider->slotSetColor2( QColor( r, g, 255 ) );

    m_pGSlider->slotSetValue( g );
    emit colorChanged(m_c);
}


void RGBWidget::slotBInChanged(int b)
{
    int r = m_c.R();
    int g = m_c.G();

    m_c = KisColor( r, g, b, cs_RGB );

    m_pRSlider->slotSetColor1( QColor( 0, g, b ) );
    m_pRSlider->slotSetColor2( QColor( 255, g, b ) );

    m_pGSlider->slotSetColor1( QColor( r, 0, b ) );
    m_pGSlider->slotSetColor2( QColor( r, 255, b ) );

    m_pBSlider->slotSetValue( b );

    emit colorChanged(m_c);
}

/*
     Hue Integer Widget changed
*/

void RGBWidget::slotHInChanged(int h)
{
    int s = m_c.s();
    int v = m_c.v();

    m_c = KisColor( h, s, v, cs_HSV );

    kdDebug() << " slotHInChanged: hue is " << h << endl;

    if(m_pSSlider)
    {
        m_pSSlider->slotSetHue( h );
    }
        
    if(m_pVSlider)
    {
        m_pVSlider->slotSetHue( h );
    }

    m_pHSlider->slotSetValue( h );
    emit colorChanged(m_c);
}

/*
     Saturation Integer Widget changed
*/


void RGBWidget::slotSInChanged(int s)
{
    int h = m_c.h();
    int v = m_c.v();

    m_c = KisColor( h, s, v, cs_HSV );

    m_pSSlider->slotSetValue( s );
    emit colorChanged(m_c);
}

/*
     Value Integer Widget changed
*/

void RGBWidget::slotVInChanged(int v)
{
    int h = m_c.h();
    int s = m_c.s();

    m_c = KisColor( h, s, v, cs_HSV );

    m_pVSlider->slotSetValue( v );
    emit colorChanged(m_c);
}

/*-------------- GreyWidget ------------------------*/

GreyWidget::GreyWidget(QWidget *parent) : QWidget(parent)
{
    // init with defaults
    m_c = KisColor::white();

    // setup slider
    m_pVSlider = new ColorSlider(this);
    m_pVSlider->setMaximumHeight(20);
    m_pVSlider->slotSetRange(0, 255);
    m_pVSlider->slotSetColor1(QColor(255, 255, 255));
    m_pVSlider->slotSetColor2(QColor(0, 0, 0));
 
    // setup slider label
    m_pVLabel = new QLabel("K", this);
    m_pVLabel->setFixedWidth(lh);
    m_pVLabel->setFixedHeight(20);
  
    // setup spin box
    m_pVIn = new QSpinBox(0, 255, 1, this);
    m_pVIn->setFixedWidth(42);
    m_pVIn->setFixedHeight(20);

    // connect color slider
    connect(m_pVSlider, SIGNAL(valueChanged(int)), this,
  		  SLOT(slotVSliderChanged(int)));

    // connect spin box
    connect(m_pVIn, SIGNAL(valueChanged(int)), this,
  		  SLOT(slotVInChanged(int)));
}


void GreyWidget::resizeEvent(QResizeEvent *)
{
    // I know a QGridLayout would look nicer,
    // but it does not use the space as good as I want it to.

    int y = height()/2;

    int labelY = y - m_pVLabel->height()/2 - 4;
    if (labelY < 0) labelY = 0;

    m_pVLabel->move(2, 0 + labelY);

    int x1 = m_pVLabel->pos().x() + m_pVLabel->width();

    int inY =y - m_pVIn->height()/2 - 4;
    if (inY < 0) 
	    inY = 0;

    m_pVIn->move(width() - m_pVIn->width(), 0 + inY);

    int x2 = width() - m_pVIn->width() - 2;

    m_pVSlider->resize(QSize(x2 - x1, y));
    m_pVSlider->move(x1, y - m_pVSlider->height()/2);
}

void GreyWidget::slotSetColor(const KisColor&c)
{
    m_c = c;
  
    float v = c.R() + c.G() + c.B();
    v /= 3;
    v = 255 - v;
    m_pVIn->setValue(static_cast<int>(v));
    m_pVSlider->slotSetValue(static_cast<int>(v));
}
  
void GreyWidget::slotVSliderChanged(int v)
{
    m_pVIn->setValue(v);
    v = 255 - v;
    emit colorChanged( KisColor(v, v, v, cs_RGB));
}

void GreyWidget::slotVInChanged(int v)
{
    m_pVSlider->slotSetValue(v);
    v = 255 - v;
    emit colorChanged(KisColor(v, v, v, cs_RGB));
}

#include "kis_colorchooser.moc"


