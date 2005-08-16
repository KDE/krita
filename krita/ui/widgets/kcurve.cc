/* ============================================================ 
 * Copyright 2004-2005 by Gilles Caulier
 * Copyright 2005 by Casper Boemann (reworked to be generic)
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

KCurve::KCurve(QWidget *parent, const char *name, WFlags f)
            : QWidget(parent, name, f)
{
    m_grab_point     = NULL;
    m_readOnlyMode   = false;
    m_guideVisible   = false;
    m_dragging = false;
    m_pix = NULL;
    
    setMouseTracking(true);
    setPaletteBackgroundColor(Qt::NoBackground);
    setMinimumSize(150, 50);
    dpoint *p = new dpoint;
    p->x = 0.0;p->y=0.0;
    m_points.inSort(p);
    p = new dpoint;
    p->x = 1.0;p->y=1.0;
    m_points.inSort(p);
    setFocusPolicy(QWidget::StrongFocus);
}

KCurve::~KCurve()
{
    if(m_pix)
        delete m_pix;
}

void KCurve::reset(void)
{
    m_grab_point   = NULL;    
    m_guideVisible = false;
    repaint(false);
}

void KCurve::setCurveGuide(QColor color)
{
    m_guideVisible = true;
    m_colorGuide   = color;
    repaint(false);
}

void KCurve::setPixmap(QPixmap pix)
{
    m_pix = new QPixmap(pix);
    repaint(false);
}

void KCurve::keyPressEvent(QKeyEvent *e)
{
    if(e->key() == Qt::Key_Delete || e->key() == Qt::Key_Backspace)
    {
        if(m_grab_point)
            m_points.remove(m_grab_point);
        delete m_grab_point;
        m_grab_point = 0;
        repaint(false);
    }
}

void KCurve::paintEvent(QPaintEvent *)
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
    
    //  draw background
    if(m_pix)
    {
        p1.scale(1.0*wWidth/m_pix->width(), 1.0*wHeight/m_pix->height());
        p1.drawPixmap(0, 0, *m_pix);
        p1.resetXForm();
    }
    else
        pm.fill();
    
    // Draw grid separators.
    p1.setPen(QPen::QPen(Qt::gray, 1, Qt::SolidLine));
    p1.drawLine(wWidth/3, 0, wWidth/3, wHeight);                 
    p1.drawLine(2*wWidth/3, 0, 2*wWidth/3, wHeight);                 
    p1.drawLine(0, wHeight/3, wWidth, wHeight/3);                 
    p1.drawLine(0, 2*wHeight/3, wWidth, 2*wHeight/3);     

    // Draw curve.
    double curvePrevVal = getCurveValue(0.0);
    p1.setPen(QPen::QPen(Qt::black, 1, Qt::SolidLine));    
    for (x = 0 ; x < wWidth ; x++)
    {
        double curveX;
        double curveVal;
        
        curveX = (x + 0.5) / wWidth;
        
        curveVal = getCurveValue(curveX);
        
        p1.drawLine(x - 1, wHeight - int(curvePrevVal * wHeight),
            x,     wHeight - int(curveVal * wHeight));                             
        
        curvePrevVal = curveVal;
    }
    p1.drawLine(x - 1, wHeight - int(curvePrevVal * wHeight),
        x,     wHeight - int(getCurveValue(1.0) * wHeight));                             
    
    // Drawing curve handles.
    if ( !m_readOnlyMode )
    {
        dpoint *p = m_points.first();
        
        while(p)
        {
            double curveX = p->x;
            double curveY = p->y;
        
            if(p == m_grab_point)
            {
                p1.setPen(QPen::QPen(Qt::red, 3, Qt::SolidLine));
                p1.drawEllipse( int(curveX * wWidth) - 2, 
                    wHeight - 2 - int(curveY * wHeight), 4, 4 ); 
            }
            else
            {
                p1.setPen(QPen::QPen(Qt::red, 1, Qt::SolidLine));
            
                p1.drawEllipse( int(curveX * wWidth) - 3, 
                    wHeight - 3 - int(curveY * wHeight), 6, 6 ); 
            }
            
            p = m_points.next();
        }
    }
    
    p1.end();
    bitBlt(this, 0, 0, &pm);
}

void KCurve::mousePressEvent ( QMouseEvent * e )
{
    if (m_readOnlyMode) return;
    
    dpoint *closest_point=NULL;
    double distance;
    
    if (e->button() != Qt::LeftButton)
        return;
    
    double x = e->pos().x() / (float)width();
    double y = 1.0 - e->pos().y() / (float)height();
    
    distance = 1000; // just a big number
    
    dpoint *p = m_points.first();
    while(p)
    {
        if (fabs (x - p->x) < distance)
        {
            distance = fabs(x - p->x);
            closest_point = p;
        }
        p = m_points.next();
    }
    
        
    if(closest_point == NULL || distance * width() > 5)
    {
        closest_point = new dpoint;
        closest_point->x = x;
        closest_point->y = y;
        m_points.inSort(closest_point);
    }
    else
        if(fabs(y - closest_point->y) * width() > 5)
            return;
        
    
    m_grab_point = closest_point;
    m_grabOffsetX = m_grab_point->x - x;
    m_grabOffsetY = m_grab_point->y - y;
    m_grab_point->x = x + m_grabOffsetX;
    m_grab_point->y = y + m_grabOffsetY;
    m_dragging = true;

    setCursor( KCursor::crossCursor() );
    
    // Determine the leftmost and rightmost points.
    m_leftmost = 0;
    m_rightmost = 1;
    
    p = m_points.first();
    while(p)
    {
        if (p != m_grab_point)
        {
            if(p->x> m_leftmost && p->x < x)
                m_leftmost = p->x;
            if(p->x < m_rightmost && p->x > x)
                m_rightmost = p->x;
        }
        p = m_points.next();
    }
    repaint(false);
}

void KCurve::mouseReleaseEvent ( QMouseEvent * e )
{
    if (m_readOnlyMode) return;
    
    if (e->button() != Qt::LeftButton)
        return;
    
    setCursor( KCursor::arrowCursor() );    
    m_dragging = false;
    repaint(false);
    emit modified();
}

void KCurve::mouseMoveEvent ( QMouseEvent * e )
{
    if (m_readOnlyMode) return;
    
    int x1, x2, y1, y2;
    
    double x = e->pos().x() / (float)width();
    double y = 1.0 - e->pos().y() / (float)height();
        
    if (m_dragging == false)   // If no point is selected set the the cursor shape if on top
    {
        double distance = 1000;
        double ydistance = 1000;
        dpoint *p = m_points.first();
        while(p)
        {
            if (fabs (x - p->x) < distance)
            {
                distance = fabs(x - p->x);
                ydistance = fabs(y - p->y);
            }
            p = m_points.next();
        }
    
        if (distance * width() > 5 || ydistance * height() > 5)
            setCursor( KCursor::arrowCursor() );    
        else
            setCursor( KCursor::crossCursor() );
    }
    else  // Else, drag the selected point
    {
        setCursor( KCursor::crossCursor() );
        
        x += m_grabOffsetX;
        y += m_grabOffsetY;
        
        if (x <= m_leftmost)
            return;
            
        if(x >= m_rightmost)
            return;
        
        if(y > 1.0)
            y = 1.0;
            
        if(y < 0.0)
            y = 0.0;
            
        m_grab_point->x = x;
        m_grab_point->y = y;
        
        emit modified();
    }
        
    repaint(false);
}

double KCurve::getCurveValue(double x)
{
    double t;
    dpoint *p;
    dpoint *p0,*p1,*p2,*p3;
    double c0,c1,c2,c3;
    double val;
    
    if(m_points.count() == 0)
        return 0.5;
    
    // First find curve segment
    p = m_points.first();
    if(x < p->x)
        return p->y;
        
    p = m_points.last();
    if(x >= p->x)
        return p->y;
    
    // Find the four control points (two on each side of x)    
    p = m_points.first();
    while(x >= p->x)
    {
        p = m_points.next();
    }
    m_points.prev();
    
    if((p0 = m_points.prev()) == NULL)
        p1 = p0 = m_points.first();
    else
        p1 = m_points.next();
    
    p2 = m_points.next();
    if(p = m_points.next())
        p3 = p;
    else
        p3 = p2;
    
    // Calculate the value
    t = (x - p1->x) / (p2->x - p1->x);
    c2 = (p2->y - p0->y) * (p2->x-p1->x) / (p2->x-p0->x);
    c3 = p1->y;
    c0 = -2*p2->y + 2*c3 + c2 + (p3->y - p1->y) * (p2->x - p1->x) / (p3->x - p1->x);
    c1 = p2->y - c3 - c2 - c0;
    val = ((c0*t + c1)*t + c2)*t + c3;
    
    if(val < 0.0)
        val = 0.0;
    if(val > 1.0)
        val = 1.0;
    return val;
}

void KCurve::leaveEvent( QEvent * )
{
}

#include "kcurve.moc"
