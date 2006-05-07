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
#include <QMouseEvent>
#include <QKeyEvent>
#include <QPaintEvent>
#include <QList>

// KDE includes.

#include <kdebug.h>
#include <kcursor.h>
#include <klocale.h>

// Local includes.

#include "kcurve.h"

KCurve::KCurve(QWidget *parent, const char *name, Qt::WFlags f)
            : QWidget(parent, f)
{
    setObjectName(name);
    m_grab_point_index = -1;
    m_readOnlyMode   = false;
    m_guideVisible   = false;
    m_dragging = false;

    setMouseTracking(true);
    setAutoFillBackground(false);
    setAttribute(Qt::WA_OpaquePaintEvent);
    setMinimumSize(150, 50);
    QPair<double,double> p;
    p.first = 0.0; p.second=0.0;
    m_points.append(p);
    p.first = 1.0; p.second=1.0;
    m_points.append(p);
    setFocusPolicy(Qt::StrongFocus);
}

KCurve::~KCurve()
{
}

void KCurve::reset(void)
{
    m_grab_point_index = -1;
    m_guideVisible = false;
    repaint();
}

void KCurve::setCurveGuide(QColor color)
{
    m_guideVisible = true;
    m_colorGuide   = color;
    repaint();
}

void KCurve::setPixmap(QPixmap pix)
{
    m_pix = pix;
    repaint();
}

void KCurve::keyPressEvent(QKeyEvent *e)
{
    if(e->key() == Qt::Key_Delete || e->key() == Qt::Key_Backspace)
    {
        if(m_grab_point_index > 0 && m_grab_point_index < m_points.count() - 1)
        {
            //first find closest point to get focus afterwards
            double grab_point_x = m_points[m_grab_point_index].first;

            int left_of_grab_point_index = m_grab_point_index - 1;
            int right_of_grab_point_index = m_grab_point_index + 1;
            int new_grab_point_index;

            if (fabs(m_points[left_of_grab_point_index].first - grab_point_x) <
                fabs(m_points[right_of_grab_point_index].first - grab_point_x))
            {
                new_grab_point_index = left_of_grab_point_index;
            }
            else
            {
                new_grab_point_index = m_grab_point_index;
            }
            m_points.removeAt(m_grab_point_index);
            m_grab_point_index = new_grab_point_index;
        }
        repaint();
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
    QPainter p(this);

    //  draw background
    if(!m_pix.isNull())
    {
        p.scale(1.0*wWidth/m_pix.width(), 1.0*wHeight/m_pix.height());
        p.drawPixmap(0, 0, m_pix);
        p.resetMatrix();
    }
    else
        p.fillRect(rect(), palette().background());

    // Draw grid separators.
    p.setPen(QPen::QPen(Qt::gray, 1, Qt::SolidLine));
    p.drawLine(wWidth/3, 0, wWidth/3, wHeight);
    p.drawLine(2*wWidth/3, 0, 2*wWidth/3, wHeight);
    p.drawLine(0, wHeight/3, wWidth, wHeight/3);
    p.drawLine(0, 2*wHeight/3, wWidth, 2*wHeight/3);

    // Draw curve.
    double curvePrevVal = getCurveValue(0.0);
    p.setPen(QPen::QPen(Qt::black, 1, Qt::SolidLine));
    for (x = 0 ; x < wWidth ; x++)
    {
        double curveX;
        double curveVal;

        curveX = (x + 0.5) / wWidth;

        curveVal = getCurveValue(curveX);

        p.drawLine(x - 1, wHeight - int(curvePrevVal * wHeight),
            x,     wHeight - int(curveVal * wHeight));

        curvePrevVal = curveVal;
    }
    p.drawLine(x - 1, wHeight - int(curvePrevVal * wHeight),
        x,     wHeight - int(getCurveValue(1.0) * wHeight));

    // Drawing curve handles.
    if ( !m_readOnlyMode )
    {
        for(int i = 0; i < m_points.count(); ++i)
        {
            double curveX = m_points.at(i).first;
            double curveY = m_points.at(i).second;

            if (i == m_grab_point_index)
            {
                p.setPen(QPen::QPen(Qt::red, 3, Qt::SolidLine));
                p.drawEllipse( int(curveX * wWidth) - 2,
                    wHeight - 2 - int(curveY * wHeight), 4, 4 );
            }
            else
            {
                p.setPen(QPen::QPen(Qt::red, 1, Qt::SolidLine));

                p.drawEllipse( int(curveX * wWidth) - 3,
                    wHeight - 3 - int(curveY * wHeight), 6, 6 );
            }
        }
    }
}

int KCurve::nearestPointInRange(double x, double y) const
{
    double nearestDistanceSquared = 1000;
    int nearestIndex = -1;
    int i = 0;
    QPair<double,double> point;

    foreach (point, m_points)
    {
        double distanceSquared = (x - point.first) * (x - point.first) + (y - point.second) * (y - point.second);

        if (distanceSquared < nearestDistanceSquared)
        {
            nearestIndex = i;
            nearestDistanceSquared = distanceSquared;
        }
        ++i;
    }

    if (nearestIndex >= 0)
    {
        if (fabs(x - m_points[nearestIndex].first) * width() < 5 &&
            fabs(y - m_points[nearestIndex].second) * width() < 5)
        {
            return nearestIndex;
        }
    }

    return -1;
}

void KCurve::mousePressEvent ( QMouseEvent * e )
{
    if (m_readOnlyMode) return;

    if (e->button() != Qt::LeftButton)
        return;

    double x = e->pos().x() / (float)width();
    double y = 1.0 - e->pos().y() / (float)height();

    int closest_point_index = nearestPointInRange(x, y);

    if(closest_point_index < 0)
    {
        QPair<double,double> newPoint(x, y);
        m_points.append(newPoint);
        qSort(m_points);
        m_grab_point_index = m_points.indexOf(newPoint);
    }
    else
    {
        m_grab_point_index = closest_point_index;
    }

    m_grabOffsetX = m_points[m_grab_point_index].first - x;
    m_grabOffsetY = m_points[m_grab_point_index].second - y;
    m_points[m_grab_point_index].first = x + m_grabOffsetX;
    m_points[m_grab_point_index].second = y + m_grabOffsetY;
    m_dragging = true;

    setCursor( KCursor::crossCursor() );
    repaint();
}

void KCurve::mouseReleaseEvent ( QMouseEvent * e )
{
    if (m_readOnlyMode) return;

    if (e->button() != Qt::LeftButton)
        return;

    setCursor( KCursor::arrowCursor() );
    m_dragging = false;
    repaint();
    emit modified();
}

void KCurve::mouseMoveEvent ( QMouseEvent * e )
{
    if (m_readOnlyMode) return;

    double x = e->pos().x() / (float)width();
    double y = 1.0 - e->pos().y() / (float)height();

    if (m_dragging == false)   // If no point is selected set the the cursor shape if on top
    {
        int nearestPointIndex = nearestPointInRange(x, y);

        if (nearestPointIndex < 0)
            setCursor( KCursor::arrowCursor() );
        else
            setCursor( KCursor::crossCursor() );
    }
    else  // Else, drag the selected point
    {
        setCursor( KCursor::crossCursor() );

        x += m_grabOffsetX;
        y += m_grabOffsetY;

        if (m_grab_point_index == 0)
        {
            x = 0;
        }
        else if (m_grab_point_index == m_points.count() - 1)
        {
            x = 1;
        }
        else
        {
            Q_ASSERT(m_grab_point_index > 0 && m_grab_point_index < m_points.count() - 1);

            double leftX = m_points[m_grab_point_index - 1].first;
            double rightX = m_points[m_grab_point_index + 1].first;

            if (x <= leftX)
            {
                x = leftX + 1E-4; // the addition so we can grab the dot later.
            }
            else if (x >= rightX)
            {
                x = rightX - 1E-4;
            }
        }

        if(y > 1.0)
            y = 1.0;

        if(y < 0.0)
            y = 0.0;

        m_points[m_grab_point_index].first = x;
        m_points[m_grab_point_index].second = y;

        emit modified();
    }

    repaint();
}

double KCurve::getCurveValue(double x)
{
    return getCurveValue(m_points, x);
}

double KCurve::getCurveValue(const QList<QPair<double,double> > &curve, double x)
{
    double t;
    QPair<double,double> p;
    QPair<double,double> p0,p1,p2,p3;
    double c0,c1,c2,c3;
    double val;

    if(curve.count() == 0)
        return 0.5;

    // First find curve segment
    p = curve.first();
    if(x < p.first)
        return p.second;

    p = curve.last();
    if(x >= p.first)
        return p.second;

    // Find the four control points (two on each side of x)
    int i = 0;
    while(x >= curve.at(i).first)
    {
        ++i;
    }
    --i;

    if (i - 1 < 0)
    {
        p1 = p0 = curve.first();
    }
    else
    {
        p0 = curve.at(i - 1);
        p1 = curve.at(i);
    }

    if (i + 2 > curve.count() - 1)
    {
        p2 = p3 = curve.last();
    }
    else
    {
        p2 = curve.at(i + 1);
        p3 = curve.at(i + 2);
    }

    // Calculate the value
    t = (x - p1.first) / (p2.first - p1.first);
    c2 = (p2.second - p0.second) * (p2.first-p1.first) / (p2.first-p0.first);
    c3 = p1.second;
    c0 = -2*p2.second + 2*c3 + c2 + (p3.second - p1.second) * (p2.first - p1.first) / (p3.first - p1.first);
    c1 = p2.second - c3 - c2 - c0;
    val = ((c0*t + c1)*t + c2)*t + c3;

    if(val < 0.0)
        val = 0.0;
    if(val > 1.0)
        val = 1.0;
    return val;
}

QList<QPair<double,double> > KCurve::getCurve()
{
    return m_points;
}

void KCurve::setCurve(QList<QPair<double,double> >inlist)
{
    m_points = inlist;
}

void KCurve::leaveEvent( QEvent * )
{
}

#include "kcurve.moc"
