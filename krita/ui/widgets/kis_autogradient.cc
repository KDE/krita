/*
 *  Copyright (c) 2004 Cyrille Berger <cberger@cberger.net>
 *                2004 Sven Langkamp <longamp@reallygood.de>
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
 
#include "kis_autogradient.h"
#include <qfontmetrics.h>
#include <qpainter.h>
#include <kfontcombo.h>
#include <qspinbox.h>
#include <qcheckbox.h> 
#include <klineedit.h>
#include <kcolorbutton.h>

#include "kis_gradient_slider_widget.h"

/************************** KisAutogradientResource **************************/

void KisAutogradientResource::createSegment( int interpolation, int colorInterpolation, double startOffset, double endOffset, double middleOffset, QColor left, QColor right )
{
	pushSegment(new KisGradientSegment(interpolation, colorInterpolation, startOffset, middleOffset, endOffset, Color( left, 1 ), Color( right, 1 )));

}

/****************************** KisAutogradient ******************************/

KisAutogradient::KisAutogradient(QWidget *parent, const char* name, const QString& caption) : KisWdgAutogradient(parent, name)
{
	setCaption(caption);
	m_autogradientResource = new KisAutogradientResource();
	m_autogradientResource->createSegment( INTERP_LINEAR, COLOR_INTERP_RGB, 0.0, 1.0, 0.5, Qt::black, Qt::white );
	gradientSlider->setGradientResource( m_autogradientResource );
	connect(gradientSlider, SIGNAL( sigSelectedSegment( KisGradientSegment* ) ), SLOT( slotSelectedSegment(KisGradientSegment*) ));
	connect(comboBoxColorInterpolationType, SIGNAL( activated(int) ), SLOT( slotChangedColorInterpolation(int) ));
	connect(comboBoxInterpolationType, SIGNAL( activated(int) ), SLOT( slotChangedInterpolation(int) ));
	connect(leftColorButton, SIGNAL( changed(const QColor&) ), SLOT( slotChangedLeftColor(const QColor&) ));
	connect(rightColorButton, SIGNAL( changed(const QColor&) ), SLOT( slotChangedRightColor(const QColor&) ));
}

void KisAutogradient::slotSelectedSegment(KisGradientSegment* segment)
{
	leftColorButton -> setColor( segment -> startColor().color() );
	rightColorButton -> setColor( segment -> endColor().color() );
	comboBoxColorInterpolationType -> setCurrentItem( segment -> colorInterpolation() );
	comboBoxInterpolationType -> setCurrentItem( segment -> interpolation() );
	emit activatedResource( m_autogradientResource );
}

void KisAutogradient::slotChangedInterpolation(int type)
{
	KisGradientSegment* segment = gradientSlider -> currentSegment();
	if(segment)
		segment -> setInterpolation( type );
	gradientSlider -> repaint();
}

void KisAutogradient::slotChangedColorInterpolation(int type)
{
	KisGradientSegment* segment = gradientSlider -> currentSegment();
	if(segment)
		segment -> setColorInterpolation( type );
	gradientSlider -> repaint();
}

void KisAutogradient::slotChangedLeftColor( const QColor& color)
{
	KisGradientSegment* segment = gradientSlider -> currentSegment();
	if(segment)
		segment -> setStartColor( Color( color, 1 ) );
	gradientSlider -> repaint();
}

void KisAutogradient::slotChangedRightColor( const QColor& color)
{
	KisGradientSegment* segment = gradientSlider -> currentSegment();
	if(segment)
		segment -> setEndColor( Color( color, 1 ) );
	gradientSlider -> repaint();
}
