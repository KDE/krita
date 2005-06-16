/* ============================================================
 * File  : curveswidget.cpp
 * Author: Gilles Caulier <caulier dot gilles at free.fr>
 * Date  : 2004-12-01
 * Description : 
 * 
 * Copyright 2004-2005 by Gilles Caulier
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation;
 * either version 2, or (at your option)
 * any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * ============================================================ */
 
// C++ includes.

#include <cmath>
#include <cstdlib>

// Qt includes.

#include <qpixmap.h>
#include <qpainter.h>
#include <qpoint.h>
#include <qpen.h>
#include <qevent.h>
#include <qtimer.h>
#include <qrect.h> 
#include <qfont.h> 
#include <qfontmetrics.h> 

// KDE includes.

#include <kdebug.h>
#include <kcursor.h>
#include <klocale.h>

// Local includes.

#include "kcurve.h"

KCurve::KCurve(int w, int h,
                           ImageCurves *curves, QWidget *parent, 
                           bool readOnly)
            : QWidget(parent, 0, Qt::WDestructiveClose)
{
    m_curves         = curves;
    m_grab_point     = -1;    
    m_last           = 0;
    m_readOnlyMode   = readOnly;
    m_guideVisible   = false;
    
    setMouseTracking(true);
    setPaletteBackgroundColor(Qt::NoBackground);
    setMinimumSize(w, h);
}

KCurve::~KCurve()
{
}

void KCurve::reset(void)
{
	m_grab_point   = -1;    
	m_guideVisible = false;
	repaint(false);
}

void KCurve::setCurveGuide(QColor color)
{
	m_guideVisible = true;
	m_colorGuide   = color;
	repaint(false);
}

void KCurve::curveTypeChanged(CurveType curveType)
{
	switch ( curveType )
	{
		case CURVE_SMOOTH:
			//  pick representative points from the curve and make them control points
		
			for (int i = 0; i <= 8; i++)
			{
				int index = CLAMP0255 (i * 32);
			
				m_curves->setCurvePoint(i * 2, QPoint::QPoint(index, 
					m_curves->getCurveValue(	index)) );
			}
			
			m_curves->curvesCalculateCurve();
			break;
         
		case CURVE_FREE:
			break;
	}
	
	repaint(false);             
	emit signalCurvesChanged();        
}

void KCurve::paintEvent( QPaintEvent * )
{
	int    x, y;
	int    wWidth = width();
	int    wHeight = height();
	
	x  = 0; 
	y  = 0;
	
	// Drawing selection or all histogram values.
	// A QPixmap is used for enable the double buffering.
	
	QPixmap pm(size());
	QPainter p1;
	p1.begin(&pm, this);
	
	int curvePrevVal = 0;
	
	for (x = 0 ; x < wWidth ; x++)
	{
		int    i, j;
		int    curveVal;
		
		i = (x * 256) / wWidth;
		j = ((x + 1) * 256) / wWidth;
		
		curveVal = m_curves->getCurveValue(i);
		
		// Drawing curves.   
		
		p1.setPen(QPen::QPen(Qt::black, 1, Qt::SolidLine));
		
		p1.drawLine(x - 1, wHeight - ((curvePrevVal * wHeight) / 256),
			x,     wHeight - ((curveVal * wHeight) / 256));                             
		
		curvePrevVal = curveVal;
	}
	
	// Drawing curves points.
	if ( !m_readOnlyMode && m_curveType == CURVE_SMOOTH )
	{      
		p1.setPen(QPen::QPen(Qt::red, 3, Qt::SolidLine));
		
		for (int p = 0 ; p < 17 ; p++)
		{
			QPoint curvePoint = m_curves->getCurvePoint(p);
		
			if (curvePoint.x() >= 0)
			{
				p1.drawEllipse( ((curvePoint.x() * wWidth) / 256) - 2, 
				wHeight - 2 - ((curvePoint.y() * 256) / wHeight),
				4, 4 ); 
			}
		}
	}
	
	// Draw grid separators.
	p1.setPen(QPen::QPen(Qt::gray, 1, Qt::SolidLine));
	p1.drawLine(wWidth/3, 0, wWidth/3, wHeight);                 
	p1.drawLine(2*wWidth/3, 0, 2*wWidth/3, wHeight);                 
	p1.drawLine(0, wHeight/3, wWidth, wHeight/3);                 
	p1.drawLine(0, 2*wHeight/3, wWidth, 2*wHeight/3);     
	
	p1.end();
	bitBlt(this, 0, 0, &pm);
}

void KCurve::mousePressEvent ( QMouseEvent * e )
{
	if (m_readOnlyMode) return;
	
	int i;
	int closest_point;
	int distance;
	
	if (e->button() != Qt::LeftButton)
		return;
	
	int x = CLAMP0255( (int)(e->pos().x()*(255.0/(float)width())) );
	int y = CLAMP0255( (int)(e->pos().y()*(255.0/(float)height())) );
	
	distance = 65536;
	
	for (i = 0, closest_point = 0 ; i < 17 ; i++)
	{
		if (m_curves->getCurvePointX(i) != -1)
		{
			if (abs (x - m_curves->getCurvePointX(i)) < distance)
			{
				distance = abs (x - m_curves->getCurvePointX(i));
				closest_point = i;
			}
		}
	}
	
	if (distance > 8)
		closest_point = (x + 8) / 16;   
	
	setCursor( KCursor::crossCursor() );
	
	switch( m_curveType )
	{
		case CURVE_SMOOTH:
			// Determine the leftmost and rightmost points.
			m_leftmost = -1;
			
			for (i = closest_point - 1 ; i >= 0 ; i--)
			{
				if (m_curves->getCurvePointX(i) != -1)
				{
					m_leftmost = m_curves->getCurvePointX(i);
					break;
				}
			}
			
			m_rightmost = 256;
			
			for (i = closest_point + 1 ; i < 17 ; i++)
			{
				if (m_curves->getCurvePointX(i) != -1)
				{
					m_rightmost = m_curves->getCurvePointX(i);
					break;
				}
			}
			
			m_grab_point = closest_point;
			m_curves->setCurvePoint(m_grab_point, QPoint::QPoint(x, 255 - y));
			
			break;
		
		case CURVE_FREE:
			m_curves->setCurveValue(x, 255 - y);
			m_grab_point = x;
			m_last = y;
			break;
	}
	
	m_curves->curvesCalculateCurve();
	repaint(false);
}

void KCurve::mouseReleaseEvent ( QMouseEvent * e )
{
	if (m_readOnlyMode) return;
	
	if (e->button() != Qt::LeftButton)
		return;
	
	setCursor( KCursor::arrowCursor() );    
	m_grab_point = -1;
	m_curves->curvesCalculateCurve();
	repaint(false);
	emit signalCurvesChanged();
}

void KCurve::mouseMoveEvent ( QMouseEvent * e )
{
	if (m_readOnlyMode) return;
	
	int i;
	int closest_point;
	int x1, x2, y1, y2;
	int distance;
		
	int x = CLAMP0255( (int)(e->pos().x()*(255.0/(float)width())) );
	int y = CLAMP0255( (int)(e->pos().y()*(255.0/(float)height())) );

	distance = 65536;
   
	for (i = 0, closest_point = 0 ; i < 17 ; i++)
	{
		if (m_curves->getCurvePointX(i) != -1)
		{
			if (abs (x - m_curves->getCurvePointX(i)) < distance)
			{
				distance = abs (x - m_curves->getCurvePointX(i));
				closest_point = i;
			}
		}
	}
	
	if (distance > 8)
		closest_point = (x + 8) / 16;   
   
	switch ( m_curveType )
	{
		case CURVE_SMOOTH:
			if (m_grab_point == -1)   // If no point is grabbed... 
			{
				if ( m_curves->getCurvePointX(closest_point) != -1 )
					setCursor( KCursor::arrowCursor() );    
				else
					setCursor( KCursor::crossCursor() );
			}
			else  // Else, drag the grabbed point
			{
				setCursor( KCursor::crossCursor() );
				
				m_curves->setCurvePointX(m_grab_point, -1);
				
				if (x > m_leftmost && x < m_rightmost)
				{
					closest_point = (x + 8) / 16;
					
					if (m_curves->getCurvePointX(closest_point) == -1)
						m_grab_point = closest_point;
					
					m_curves->setCurvePoint(m_grab_point, QPoint::QPoint(x, 255 - y));
				}
				
			m_curves->curvesCalculateCurve();
			emit signalCurvesChanged();
			}
			
			break;
		
		case CURVE_FREE:
			if (m_grab_point != -1)
			{
				if (m_grab_point > x)
				{
					x1 = x;
					x2 = m_grab_point;
					y1 = y;
					y2 = m_last;
				}
				else
				{
					x1 = m_grab_point;
					x2 = x;
					y1 = m_last;
					y2 = y;
				}

				if (x2 != x1)
				{
					for (i = x1 ; i <= x2 ; i++)
						m_curves->setCurveValue(i, 255 - (y1 + ((y2 - y1) * (i - x1)) / (x2 - x1)));
				}
				else
					m_curves->setCurveValue(x, 255 - y);
				
				m_grab_point = x;
				m_last = y;
			}
			
			emit signalCurvesChanged();
			break;
		}
		
		emit signalMouseMoved(x, 255 - y);
		repaint(false);
}

void KCurve::leaveEvent( QEvent * )
{
	emit signalMouseMoved(-1, -1);
}

#include "kcurve.moc"
