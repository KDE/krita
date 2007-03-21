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

#include <QPixmap>
#include <QPainter>
#include <QPoint>
#include <QPen>
#include <QEvent>
#include <QTimer>
#include <QRect>
#include <QFont>
#include <QFontMetrics>
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

KCurve::KCurve(QWidget *parent, Qt::WFlags f)
            : QWidget(parent, f)
{
    m_grab_point_index = -1;
    m_readOnlyMode   = false;
    m_guideVisible   = false;
    m_dragging = false;

    setMouseTracking(true);
    setAutoFillBackground(false);
    setAttribute(Qt::WA_OpaquePaintEvent);
    setMinimumSize(150, 50);
    QPointF p;
    p.rx() = 0.0; p.ry()=0.0;
    m_points.append(p);
    p.rx() = 1.0; p.ry()=1.0;
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

void KCurve::setPixmap(const QPixmap & pix)
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
            //rx() find closest point to get focus afterwards
            double grab_point_x = m_points[m_grab_point_index].rx();

            int left_of_grab_point_index = m_grab_point_index - 1;
            int right_of_grab_point_index = m_grab_point_index + 1;
            int new_grab_point_index;

            if (fabs(m_points[left_of_grab_point_index].rx() - grab_point_x) <
                fabs(m_points[right_of_grab_point_index].rx() - grab_point_x))
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
    else if(e->key() == Qt::Key_Escape && m_dragging)
    {
        m_points[m_grab_point_index].rx() = m_grabOriginalX;
        m_points[m_grab_point_index].ry() = m_grabOriginalY;
        setCursor( KCursor::arrowCursor() );
        m_dragging = false;
        repaint();
        emit modified();
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
    p.setRenderHint(QPainter::Antialiasing);

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

        p.drawLine(QLineF(x + 0.5 - 1, wHeight - curvePrevVal * wHeight + 0.5,
                          x + 0.5, wHeight - curveVal * wHeight + 0.5));
        curvePrevVal = curveVal;
    }
    p.drawLine(QLineF(x + 0.5 - 1, wHeight - curvePrevVal * wHeight + 0.5,
                      x + 0.5, wHeight - getCurveValue(1.0) * wHeight + 0.5));

    // Drawing curve handles.
    if ( !m_readOnlyMode )
    {
        for(int i = 0; i < m_points.count(); ++i)
        {
            double curveX = m_points.at(i).x();
            double curveY = m_points.at(i).y();

            if (i == m_grab_point_index)
            {
                p.setPen(QPen::QPen(Qt::red, 3, Qt::SolidLine));
                p.drawEllipse(QRectF(curveX * wWidth + 0.5 - 2,
                    wHeight - 2 - curveY * wHeight + 0.5, 4, 4));
            }
            else
            {
                p.setPen(QPen::QPen(Qt::red, 1, Qt::SolidLine));
                p.drawEllipse(QRectF(curveX * wWidth + 0.5 - 3,
                    wHeight - 3 - curveY * wHeight + 0.5, 6, 6));
            }
        }
    }
}

int KCurve::nearestPointInRange(QPointF pt) const
{
    double nearestDistanceSquared = 1000;
    int nearestIndex = -1;
    int i = 0;
    QPointF point;

    foreach (point, m_points)
    {
        double distanceSquared = (pt.x() - point.rx()) * (pt.x() - point.rx()) + (pt.y()
                                                 - point.ry()) * (pt.y() - point.ry());

        if (distanceSquared < nearestDistanceSquared)
        {
            nearestIndex = i;
            nearestDistanceSquared = distanceSquared;
        }
        ++i;
    }

    if (nearestIndex >= 0)
    {
        if (fabs(pt.x() - m_points[nearestIndex].x()) * width() < 5 &&
            fabs(pt.y() - m_points[nearestIndex].y()) * width() < 5)
        {
            return nearestIndex;
        }
    }

    return -1;
}

static bool pointLessThan(const QPointF &a, const QPointF &b)
{
    return a.x() < b.x();
}

void KCurve::mousePressEvent ( QMouseEvent * e )
{
    if (m_readOnlyMode) return;

    if (e->button() != Qt::LeftButton)
        return;

    double x = e->pos().x() / (float)width();
    double y = 1.0 - e->pos().y() / (float)height();

    int closest_point_index = nearestPointInRange(QPointF(x, y));

    if(closest_point_index < 0)
    {
        QPointF newPoint(x, y);
        m_points.append(newPoint);
        qSort(m_points.begin(), m_points.end(), pointLessThan);
        m_grab_point_index = m_points.indexOf(newPoint);
    }
    else
    {
        m_grab_point_index = closest_point_index;
    }

    m_grabOriginalX = m_points[m_grab_point_index].x();
    m_grabOriginalY = m_points[m_grab_point_index].y();
    m_grabOffsetX = m_points[m_grab_point_index].x() - x;
    m_grabOffsetY = m_points[m_grab_point_index].y() - y;
    m_points[m_grab_point_index].rx() = x + m_grabOffsetX;
    m_points[m_grab_point_index].ry() = y + m_grabOffsetY;
    m_dragging = true;
    m_draggedawaypointindex = -1;

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
        int nearestPointIndex = nearestPointInRange(QPointF(x, y));

        if (nearestPointIndex < 0)
            setCursor( KCursor::arrowCursor() );
        else
            setCursor( KCursor::crossCursor() );
    }
    else  // Else, drag the selected point
    {
        bool removepoint = e->pos().x() - width() >15 || e->pos().x()< -15 || e->pos().y() - height() >15 || e->pos().y()< -15;

        if (removepoint == false && m_draggedawaypointindex != -1)
        {
            // point is no longer dragged away so reinsert it
            QPointF newPoint(m_draggedawaypoint);
            m_points.insert(m_draggedawaypointindex, newPoint);
            m_grab_point_index = m_draggedawaypointindex;
            m_draggedawaypointindex = -1;
        }

        if (removepoint == true && m_draggedawaypointindex != -1)
            return;

        setCursor( KCursor::crossCursor() );

        x += m_grabOffsetX;
        y += m_grabOffsetY;

        double leftX;
        double rightX;
        if (m_grab_point_index == 0)
        {
            leftX = 0.0;
            if(m_points.count()>1)
                rightX = m_points[m_grab_point_index + 1].rx() - 1E-4;
            else
                rightX = 1.0;
        }
        else if (m_grab_point_index == m_points.count() - 1)
        {
            leftX = m_points[m_grab_point_index - 1].rx() + 1E-4;
            rightX = 1.0;
        }
        else
        {
            Q_ASSERT(m_grab_point_index > 0 && m_grab_point_index < m_points.count() - 1);

            // the 1E-4 addition so we can grab the dot later.
            leftX = m_points[m_grab_point_index - 1].rx() + 1E-4;
            rightX = m_points[m_grab_point_index + 1].rx() - 1E-4;
        }

        if (x <= leftX)
        {
            x = leftX;
        }
        else if (x >= rightX)
        {
            x = rightX;
        }

        if(y > 1.0)
            y = 1.0;

        if(y < 0.0)
            y = 0.0;

        m_points[m_grab_point_index].rx() = x;
        m_points[m_grab_point_index].ry() = y;

        if (removepoint)
        {
            m_draggedawaypoint.rx() = m_points[m_grab_point_index].rx();
            m_draggedawaypoint.ry() = m_points[m_grab_point_index].ry();
            m_draggedawaypointindex = m_grab_point_index;
            m_points.removeAt(m_grab_point_index);
        }

        emit modified();
    }

    repaint();
}

double KCurve::getCurveValue(double x)
{
    return getCurveValue(m_points, x);
}

double KCurve::getCurveValue(const QList<QPointF > &curve, double x)
{
    double t;
    QPointF p;
    QPointF p0,p1,p2,p3;
    double c0,c1,c2,c3;
    double val;

    if(curve.count() == 0)
        return 0.5;

    // First find curve segment
    p = curve.first();
    if(x < p.x())
        return p.y();

    p = curve.last();
    if(x >= p.x())
        return p.y();

    // Find the four control points (two on each side of x)
    int i = 0;
    while(x >= curve.at(i).x())
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
    t = (x - p1.x()) / (p2.x() - p1.x());
    c2 = (p2.y() - p0.y()) * (p2.x()-p1.x()) / (p2.x()-p0.x());
    c3 = p1.y();
    c0 = -2*p2.y() + 2*c3 + c2 + (p3.y() - p1.y()) * (p2.x() - p1.x()) / (p3.x() - p1.x());
    c1 = p2.y() - c3 - c2 - c0;
    val = ((c0*t + c1)*t + c2)*t + c3;

    if(val < 0.0)
        val = 0.0;
    if(val > 1.0)
        val = 1.0;
    return val;
}

QList<QPointF > KCurve::getCurve()
{
    return m_points;
}

void KCurve::setCurve(QList<QPointF >inlist)
{
    m_points = inlist;
}

void KCurve::leaveEvent( QEvent * )
{
}

#include "kcurve.moc"
