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
    QPair<double,double> *p = new QPair<double,double>;
    p->first = 0.0; p->second=0.0;
    m_points.append(p);
    p = new QPair<double,double>;
    p->first = 1.0; p->second=1.0;
    m_points.append(p);
    m_points.setAutoDelete(true);
    setFocusPolicy(QWidget::StrongFocus);
}

KCurve::~KCurve()
{
    if (m_pix) delete m_pix;
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
    if (m_pix) delete m_pix;
    m_pix = new QPixmap(pix);
    repaint(false);
}

void KCurve::keyPressEvent(QKeyEvent *e)
{
    if(e->key() == Qt::Key_Delete || e->key() == Qt::Key_Backspace)
    {
        QPair<double,double> *closest_point=NULL;
        if(m_grab_point)
        {
            //first find closest point to get focus afterwards
            QPair<double,double> *p = m_points.first();
            double distance = 1000; // just a big number
            while(p)
            {
                if(p!=m_grab_point)
                if (fabs (m_grab_point->first - p->first) < distance)
                {
                    distance = fabs(m_grab_point->first - p->first);
                    closest_point = p;
                }
                p = m_points.next();
            }
            m_points.remove(m_grab_point);
        }
        m_grab_point = closest_point;
        repaint(false);
    }
    else
        QWidget::keyPressEvent(e);
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
        QPair<double,double> *p = m_points.first();
        
        while(p)
        {
            double curveX = p->first;
            double curveY = p->second;
        
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
    
    QPair<double,double> *closest_point=NULL;
    double distance;
    
    if (e->button() != Qt::LeftButton)
        return;
    
    double x = e->pos().x() / (float)width();
    double y = 1.0 - e->pos().y() / (float)height();

    distance = 1000; // just a big number

    QPair<double,double> *p = m_points.first();
    int insert_pos,pos=0;
    while(p)
    {
        if (fabs (x - p->first) < distance)
        {
            distance = fabs(x - p->first);
            closest_point = p;
            if(x < p->first)
                insert_pos = pos;
            else
                insert_pos = pos + 1;
        }
        p = m_points.next();
        pos++;
    }


    if(closest_point == NULL)
    {
        closest_point = new QPair<double,double>;
        closest_point->first = x;
        closest_point->second = y;
        m_points.append(closest_point);
    }
    else if(distance * width() > 5)
    {
        closest_point = new QPair<double,double>;
        closest_point->first = x;
        closest_point->second = y;
        m_points.insert(insert_pos, closest_point);
    }
    else
        if(fabs(y - closest_point->second) * width() > 5)
            return;
    
    
    m_grab_point = closest_point;
    m_grabOffsetX = m_grab_point->first - x;
    m_grabOffsetY = m_grab_point->second - y;
    m_grab_point->first = x + m_grabOffsetX;
    m_grab_point->second = y + m_grabOffsetY;
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
            if(p->first> m_leftmost && p->first < x)
                m_leftmost = p->first;
            if(p->first < m_rightmost && p->first > x)
                m_rightmost = p->first;
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

    double x = e->pos().x() / (float)width();
    double y = 1.0 - e->pos().y() / (float)height();
    
    if (m_dragging == false)   // If no point is selected set the the cursor shape if on top
    {
        double distance = 1000;
        double ydistance = 1000;
        QPair<double,double> *p = m_points.first();
        while(p)
        {
            if (fabs (x - p->first) < distance)
            {
                distance = fabs(x - p->first);
                ydistance = fabs(y - p->second);
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
            x = m_leftmost + 1E-4; // the addition so we can grab the dot later.
            
        if(x >= m_rightmost)
            x = m_rightmost - 1E-4;
        
        if(y > 1.0)
            y = 1.0;
            
        if(y < 0.0)
            y = 0.0;
            
        m_grab_point->first = x;
        m_grab_point->second = y;
        
        emit modified();
    }
        
    repaint(false);
}

double KCurve::getCurveValue(double x)
{
    return getCurveValue(m_points, x);
}

double KCurve::getCurveValue(QPtrList<QPair<double,double> > &curve, double x)
{
    double t;
    QPair<double,double> *p;
    QPair<double,double> *p0,*p1,*p2,*p3;
    double c0,c1,c2,c3;
    double val;
    
    if(curve.count() == 0)
        return 0.5;
    
    // First find curve segment
    p = curve.first();
    if(x < p->first)
        return p->second;
        
    p = curve.last();
    if(x >= p->first)
        return p->second;
    
    // Find the four control points (two on each side of x)    
    p = curve.first();
    while(x >= p->first)
    {
        p = curve.next();
    }
    curve.prev();
    
    if((p0 = curve.prev()) == NULL)
        p1 = p0 = curve.first();
    else
        p1 = curve.next();
    
    p2 = curve.next();
    if( (p = curve.next()) )
        p3 = p;
    else
        p3 = p2;
    
    // Calculate the value
    t = (x - p1->first) / (p2->first - p1->first);
    c2 = (p2->second - p0->second) * (p2->first-p1->first) / (p2->first-p0->first);
    c3 = p1->second;
    c0 = -2*p2->second + 2*c3 + c2 + (p3->second - p1->second) * (p2->first - p1->first) / (p3->first - p1->first);
    c1 = p2->second - c3 - c2 - c0;
    val = ((c0*t + c1)*t + c2)*t + c3;
    
    if(val < 0.0)
        val = 0.0;
    if(val > 1.0)
        val = 1.0;
    return val;
}

QPtrList<QPair<double,double> > KCurve::getCurve()
{
    QPtrList<QPair<double,double> > outlist;
    QPair<double,double> *p;
    QPair<double,double> *outpoint;

    p = m_points.first();
    while(p)
    {
        outpoint = new QPair<double,double>(p->first, p->second);
        outlist.append(outpoint);
        p = m_points.next();
    }
    return outlist;
}

void KCurve::setCurve(QPtrList<QPair<double,double> >inlist)
{
    QPair<double,double> *p;
    QPair<double,double> *inpoint;

    m_points.clear();

    inpoint = inlist.first();
    while(inpoint)
    {
        p = new QPair<double,double>(inpoint->first, inpoint->second);
        m_points.append(p);
        inpoint = inlist.next();
    }
}

void KCurve::leaveEvent( QEvent * )
{
}

#include "kcurve.moc"
