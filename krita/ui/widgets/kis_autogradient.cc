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

#include <qpainter.h>
#include <qcombobox.h>

#include <kcolorbutton.h>
#include <knuminput.h>

#include "kis_gradient_slider_widget.h"

/************************** KisAutogradientResource **************************/

void KisAutogradientResource::createSegment( int interpolation, int colorInterpolation, double startOffset, double endOffset, double middleOffset, QColor left, QColor right )
{
	pushSegment(new KisGradientSegment(interpolation, colorInterpolation, startOffset, middleOffset, endOffset, Color( left, 1 ), Color( right, 1 )));

}

const QValueVector<double> KisAutogradientResource::getHandlePositions() const
{
	QValueVector<double> handlePositions;

	handlePositions.push_back(m_segments[0] -> startOffset());
	for (uint i = 0; i < m_segments.count(); i++)
	{
		handlePositions.push_back(m_segments[i] -> endOffset());
	}
	return handlePositions;
}

const QValueVector<double> KisAutogradientResource::getMiddleHandlePositions() const
{
	QValueVector<double> middleHandlePositions;

	for (uint i = 0; i < m_segments.count(); i++)
	{
		middleHandlePositions.push_back(m_segments[i] -> middleOffset());
	}
	return middleHandlePositions;
}

void KisAutogradientResource::moveSegmentStartOffset( KisGradientSegment* segment, double t)
{
	QValueVector<KisGradientSegment*>::iterator it = qFind( m_segments.begin(), m_segments.end(), segment );
	if ( it != m_segments.end() )
	{
		if ( it == m_segments.begin() )
		{
			segment -> setStartOffset( 0.0 );
			return;
		}
		KisGradientSegment* previousSegment = (*(it-1));
		if ( t > segment -> startOffset()  )
		{
			if( t > segment -> middleOffset() )
				t = segment -> middleOffset();
		}
		else {
			if( t < previousSegment -> middleOffset() )
				t = previousSegment -> middleOffset();
		}
		previousSegment -> setEndOffset( t );
		segment -> setStartOffset( t );
	}
}

void KisAutogradientResource::moveSegmentEndOffset( KisGradientSegment* segment, double t)
{
	QValueVector<KisGradientSegment*>::iterator it = qFind( m_segments.begin(), m_segments.end(), segment );
	if ( it != m_segments.end() )
	{
		if ( it+1 == m_segments.end() )
		{
			segment -> setEndOffset( 1.0 );
			return;
		}
		KisGradientSegment* followingSegment = (*(it+1));
		if ( t < segment -> endOffset() )
		{
			if( t < segment -> middleOffset() )
				t = segment -> middleOffset();
		}
		else {
			if( t > followingSegment -> middleOffset() )
				t = followingSegment -> middleOffset();
		}
		followingSegment -> setStartOffset( t );
		segment -> setEndOffset( t );
	}
}

void KisAutogradientResource::moveSegmentMiddleOffset( KisGradientSegment* segment, double t)
{
	if( segment )
	{
		if( t > segment -> endOffset() )
			segment -> setMiddleOffset( segment -> endOffset() );
		else if( t < segment -> startOffset() )
			segment -> setMiddleOffset( segment -> startOffset() );
		else
			segment -> setMiddleOffset( t );
	}
}

void KisAutogradientResource::splitSegment( KisGradientSegment* segment )
{
	Q_ASSERT(segment != 0);
	QValueVector<KisGradientSegment*>::iterator it = qFind( m_segments.begin(), m_segments.end(), segment );
	if ( it != m_segments.end() )
	{
		KisGradientSegment* newSegment = new KisGradientSegment(
				segment -> interpolation(), segment -> colorInterpolation(),
				segment  -> startOffset(),
				( segment -> middleOffset() - segment -> startOffset() ) / 2 + segment -> startOffset(),
				segment -> middleOffset(), 
				segment -> startColor(),
				segment -> colorAt( segment -> middleOffset() ) );
		m_segments.insert( it, newSegment );
		segment -> setStartColor( segment -> colorAt( segment -> middleOffset() ) );	
		segment -> setStartOffset( segment -> middleOffset() );
		segment -> setMiddleOffset( ( segment -> endOffset() - segment -> startOffset() ) / 2 + segment -> startOffset() );
	}
}

void KisAutogradientResource::duplicateSegment( KisGradientSegment* segment )
{
	Q_ASSERT(segment != 0);
	QValueVector<KisGradientSegment*>::iterator it = qFind( m_segments.begin(), m_segments.end(), segment );
	if ( it != m_segments.end() )
	{
		double middlePostionPercentage = ( segment -> middleOffset() - segment -> startOffset() ) / segment -> length();
		double center = segment -> startOffset() + segment -> length() / 2;
		KisGradientSegment* newSegment = new KisGradientSegment(
				segment -> interpolation(), segment -> colorInterpolation(),
				segment  -> startOffset(),
				segment -> length() / 2 * middlePostionPercentage + segment -> startOffset(),
				center, segment -> startColor(),
				segment -> endColor() );
		m_segments.insert( it, newSegment );
		segment -> setStartOffset( center );
		segment -> setMiddleOffset( segment -> length() * middlePostionPercentage  + segment -> startOffset() );
	}
}

void KisAutogradientResource::mirrorSegment( KisGradientSegment* segment )
{
	Q_ASSERT(segment != 0);
	Color tmpColor = segment -> startColor();
	segment -> setStartColor( segment -> endColor() );
	segment -> setEndColor( tmpColor );
	segment -> setMiddleOffset( segment -> endOffset() - ( segment -> middleOffset() - segment -> startOffset() ) );

	if( segment -> interpolation() == INTERP_SPHERE_INCREASING )
		segment -> setInterpolation( INTERP_SPHERE_DECREASING );
	else if( segment -> interpolation() == INTERP_SPHERE_DECREASING )
		segment -> setInterpolation( INTERP_SPHERE_INCREASING );

	if( segment -> colorInterpolation() == COLOR_INTERP_HSV_CW )
		segment -> setColorInterpolation( COLOR_INTERP_HSV_CCW );
	else if( segment -> colorInterpolation() == COLOR_INTERP_HSV_CCW )
		segment -> setColorInterpolation( COLOR_INTERP_HSV_CW );
}

KisGradientSegment* KisAutogradientResource::removeSegment( KisGradientSegment* segment )
{
	Q_ASSERT(segment != 0);
	if( m_segments.count() < 2 )
		return 0;
	QValueVector<KisGradientSegment*>::iterator it = qFind( m_segments.begin(), m_segments.end(), segment );
	if ( it != m_segments.end() )
	{
		double middlePostionPercentage;
		KisGradientSegment* nextSegment;
		if( it == m_segments.begin() )
		{
			nextSegment = (*(it+1));
			middlePostionPercentage = ( nextSegment -> middleOffset() - nextSegment -> startOffset() ) / nextSegment -> length();
			nextSegment -> setStartOffset( segment -> startOffset() );
			nextSegment -> setMiddleOffset( middlePostionPercentage * nextSegment -> length() + nextSegment -> startOffset() );
		}
		else
		{
			nextSegment = (*(it-1));
			middlePostionPercentage = ( nextSegment -> middleOffset() - nextSegment -> startOffset() ) / nextSegment -> length();
			nextSegment -> setEndOffset( segment -> endOffset() );
			nextSegment -> setMiddleOffset( middlePostionPercentage * nextSegment -> length() + nextSegment -> startOffset() );
		}

		delete segment;
		m_segments.erase( it );
		return nextSegment;
	}
	return 0;
}

bool KisAutogradientResource::removeSegmentPossible() const
{
	if( m_segments.count() < 2 )
		return false;
	return true;
}

/****************************** KisAutogradient ******************************/

KisAutogradient::KisAutogradient(QWidget *parent, const char* name, const QString& caption) : KisWdgAutogradient(parent, name)
{
	setCaption(caption);
	m_autogradientResource = new KisAutogradientResource();
	m_autogradientResource->createSegment( INTERP_LINEAR, COLOR_INTERP_RGB, 0.0, 1.0, 0.5, Qt::black, Qt::white );
	connect(gradientSlider, SIGNAL( sigSelectedSegment( KisGradientSegment* ) ), SLOT( slotSelectedSegment(KisGradientSegment*) ));
	gradientSlider->setGradientResource( m_autogradientResource );
	connect(comboBoxColorInterpolationType, SIGNAL( activated(int) ), SLOT( slotChangedColorInterpolation(int) ));
	connect(comboBoxInterpolationType, SIGNAL( activated(int) ), SLOT( slotChangedInterpolation(int) ));
	connect(leftColorButton, SIGNAL( changed(const QColor&) ), SLOT( slotChangedLeftColor(const QColor&) ));
	connect(rightColorButton, SIGNAL( changed(const QColor&) ), SLOT( slotChangedRightColor(const QColor&) ));

	intNumInputLeftOpacity -> setRange( 0, 100);
	connect(intNumInputLeftOpacity, SIGNAL( valueChanged(int) ), SLOT( slotChangedLeftOpacity(int) ));
	intNumInputRightOpacity -> setRange( 0, 100);
	connect(intNumInputRightOpacity, SIGNAL( valueChanged(int) ), SLOT( slotChangedRightOpacity(int) ));
}

void KisAutogradient::slotSelectedSegment(KisGradientSegment* segment)
{
	leftColorButton -> setColor( segment -> startColor().color() );
	rightColorButton -> setColor( segment -> endColor().color() );
	comboBoxColorInterpolationType -> setCurrentItem( segment -> colorInterpolation() );
	comboBoxInterpolationType -> setCurrentItem( segment -> interpolation() );

	int leftOpacity = qRound(segment -> startColor().alpha() * 100);
	intNumInputLeftOpacity -> setValue( leftOpacity );

	int rightOpacity = qRound(segment -> endColor().alpha() * 100);
	intNumInputRightOpacity -> setValue( rightOpacity );

	emit activatedResource( m_autogradientResource );
}

void KisAutogradient::slotChangedInterpolation(int type)
{
	KisGradientSegment* segment = gradientSlider -> selectedSegment();
	if(segment)
		segment -> setInterpolation( type );
	gradientSlider -> repaint();
}

void KisAutogradient::slotChangedColorInterpolation(int type)
{
	KisGradientSegment* segment = gradientSlider -> selectedSegment();
	if(segment)
		segment -> setColorInterpolation( type );
	gradientSlider -> repaint();
}

void KisAutogradient::slotChangedLeftColor( const QColor& color)
{
	KisGradientSegment* segment = gradientSlider -> selectedSegment();
	if(segment)
		segment -> setStartColor( Color( color, segment -> startColor().alpha() ) );
	gradientSlider -> repaint();
}

void KisAutogradient::slotChangedRightColor( const QColor& color)
{
	KisGradientSegment* segment = gradientSlider -> selectedSegment();
	if(segment)
		segment -> setEndColor( Color( color, segment -> endColor().alpha() ) );
	gradientSlider -> repaint();
}

void KisAutogradient::slotChangedLeftOpacity( int value )
{
	KisGradientSegment* segment = gradientSlider -> selectedSegment();
	if(segment)
		segment -> setStartColor( Color( segment -> startColor().color(), (double)value / 100 ) );
	gradientSlider -> repaint(false);
}

void KisAutogradient::slotChangedRightOpacity( int value )
{
	KisGradientSegment* segment = gradientSlider -> selectedSegment();
	if(segment)
		segment -> setEndColor( Color( segment -> endColor().color(), (double)value / 100 ) );
	gradientSlider -> repaint(false);
}