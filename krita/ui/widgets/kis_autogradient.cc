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
 
#include "kis_autogradient.h"
#include <qfontmetrics.h>
#include <qpainter.h>
#include <kfontcombo.h>
#include <qspinbox.h>
#include <qcheckbox.h> 
#include <klineedit.h>

#include "kis_gradient_slider_widget.h"

/************************** KisAutogradientResource **************************/

void KisAutogradientResource::createSegment( QString interpolation, QString colorInterpolation, double startOffset, double endOffset, double middleOffset, QColor left, QColor right )
{
	InterpolationStrategy *interpolator;
	if( interpolation == "Linear" )
	{
		interpolator =  LinearInterpolationStrategy::instance(); 
	} else if( interpolation == "Curved" ) {
		interpolator =  CurvedInterpolationStrategy::instance(); 
	} else if( interpolation == "Sine" ) {
		interpolator =  SineInterpolationStrategy::instance(); 
	} else if( interpolation == "Sphere Increasing" ) {
		interpolator =  SphereIncreasingInterpolationStrategy::instance(); 
	} else {
		interpolator =  SphereDecreasingInterpolationStrategy::instance(); 
	}
	
	ColorInterpolationStrategy *colorInterpolator;

	if( colorInterpolation == "RGB" )
	{
		colorInterpolator = RGBColorInterpolationStrategy::instance();
	} else if( colorInterpolation == "HSV CW" )
	{
		colorInterpolator = HSVCWColorInterpolationStrategy::instance();
	} else {
		colorInterpolator = HSVCCWColorInterpolationStrategy::instance();
	} 
	

	// pushSegment(new Segment(interpolator, colorInterpolator, startOffset, middleOffset, endOffset, Color( left, 1 ), Color( right, 0 )));

}

/****************************** KisAutogradient ******************************/

KisAutogradient::KisAutogradient(QWidget *parent, const char* name, const QString& caption) : KisWdgAutogradient(parent, name)
{
	setCaption(caption);
	m_autogradientResource = new KisAutogradientResource();
	m_autogradientResource->createSegment( "Linear", "RGB", 0.0, 1.0, 0.5, Qt::black, Qt::white );
	gradientSlider->setGradientResource( m_autogradientResource );
}
