/*
 * This file is part of Krita
 *
 * Copyright (c) 2006 Frederic Coiffier <fcoiffie@gmail.com>
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
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */
 
// C++ includes.

#include <cmath>
#include <cstdlib>

// Qt includes.

#include <qpixmap.h>
#include <qpainter.h>
#include <qpoint.h>
#include <qpen.h>

// Local includes.

#include "kgradientslider.h"

KGradientSlider::KGradientSlider(QWidget *parent, const char *name, WFlags f)
            : QWidget(parent, name, f)
{
    m_dragging = false;
    
    setMouseTracking(true);
    setPaletteBackgroundColor(Qt::NoBackground);
    setMaximumSize(255, 28);

    m_blackcursor = 0;
    m_whitecursor = 255;
    m_gamma = 1.0;
    m_gammaEnabled = false;
    setFocusPolicy(QWidget::StrongFocus);
}

KGradientSlider::~KGradientSlider()
{
}

void KGradientSlider::paintEvent(QPaintEvent *)
{
    int    x, y;
    int    wWidth = width();
    int    wHeight = height();

    int gradientHeight = (wHeight / 3);
    
    // A QPixmap is used for enable the double buffering.
    /*if (!m_dragging) {*/
        QPixmap pm(size());
        QPainter p1;
        p1.begin(&pm, this);

        pm.fill();

        // Draw first gradient
        y = 0;
        p1.setPen(QPen::QPen(QColor(0,0,0),1, Qt::SolidLine));
        for( x=0; x<255; ++x )
        {
            int gray = (255 * x) / wWidth;
            p1.setPen(QColor(gray, gray, gray));
            p1.drawLine(x, y, x, y + gradientHeight - 1);
        }
    
        // Draw second gradient
        y = (wHeight / 3);
        if (m_blackcursor > 0) {
            p1.fillRect(0, y, (int)m_blackcursor, gradientHeight, QBrush(Qt::black));
        }
        if (m_whitecursor < 255) {
            p1.fillRect((int)m_whitecursor, y, 255, gradientHeight, QBrush(Qt::white));
        }
        for(x = (int)m_blackcursor; x < (int)m_whitecursor; ++x )
        {
            double inten = (double)(x - m_blackcursor) / (double)(m_whitecursor - m_blackcursor);
            inten = pow (inten, (1.0 / m_gamma));
            int gray = (int)(255 * inten);
            p1.setPen(QColor(gray, gray, gray));
            p1.drawLine(x, y, x, y + gradientHeight - 1);
        }

        // Draw cursors
        y = (2 * wHeight / 3);
        QPointArray *a = new QPointArray(3);
        p1.setPen(Qt::black);

        a->setPoint(0, m_blackcursor, y);
        a->setPoint(1, m_blackcursor + 3, wHeight - 1);
        a->setPoint(2, m_blackcursor - 3, wHeight - 1);
        p1.setBrush(Qt::black);
        p1.drawPolygon(*a);

        if (m_gammaEnabled) {
            a->setPoint(0, m_gammacursor, y);
            a->setPoint(1, m_gammacursor + 3, wHeight - 1);
            a->setPoint(2, m_gammacursor - 3, wHeight - 1);
            p1.setBrush(Qt::gray);
            p1.drawPolygon(*a);
        }

        a->setPoint(0, m_whitecursor, y);
        a->setPoint(1, m_whitecursor + 3, wHeight - 1);
        a->setPoint(2, m_whitecursor - 3, wHeight - 1);
        p1.setBrush(Qt::white);
        p1.drawPolygon(*a);

    p1.end();
    bitBlt(this, 0, 0, &pm);
}

void KGradientSlider::mousePressEvent ( QMouseEvent * e )
{
    eCursor closest_cursor;
    int distance;
    
    if (e->button() != Qt::LeftButton)
        return;
    
    unsigned int x = e->pos().x();

    distance = 1000; // just a big number

    if (abs(x - m_blackcursor) < distance)
    {
        distance = abs(x - m_blackcursor);
        closest_cursor = BlackCursor;
    }

    if (abs(x - m_whitecursor) < distance)
    {
        distance = abs(x - m_whitecursor);
        closest_cursor = WhiteCursor;
    }

    if (m_gammaEnabled && (abs(x - m_gammacursor) < distance))
    {
        distance = abs(x - m_gammacursor);
        closest_cursor = GammaCursor;
    }

    if (distance > 20)
    {
        return;
    }


    m_dragging = true;

    // Determine cursor values and the leftmost and rightmost points.

    switch (closest_cursor) {
        case BlackCursor:
            m_blackcursor = x;
            m_grab_cursor = closest_cursor;
            m_leftmost = 0;
            m_rightmost = m_whitecursor;
            if (m_gammaEnabled) {
                double delta = (double) (m_whitecursor - m_blackcursor) / 2.0;
                double mid   = (double)m_blackcursor + delta;
                double tmp   = log10 (1.0 / m_gamma);
                m_gammacursor = (unsigned int)round(mid + delta * tmp);
            }
            break;
        case WhiteCursor:
            m_whitecursor = x;
            m_grab_cursor = closest_cursor;
            m_leftmost = m_blackcursor;
            m_rightmost = 255;
            if (m_gammaEnabled) {
                double delta = (double) (m_whitecursor - m_blackcursor) / 2.0;
                double mid   = (double)m_blackcursor + delta;
                double tmp   = log10 (1.0 / m_gamma);
                m_gammacursor = (unsigned int)round(mid + delta * tmp);
            }
            break;
        case GammaCursor:
            m_gammacursor = x;
            m_grab_cursor = closest_cursor;
            m_leftmost = m_blackcursor;
            m_rightmost = m_whitecursor;

            double delta = (double) (m_whitecursor - m_blackcursor) / 2.0;
            double mid = (double)m_blackcursor + delta;
            double tmp = (x - mid) / delta;
            m_gamma = 1.0 / pow (10, tmp);
            break;
       }
    repaint(false);
}

void KGradientSlider::mouseReleaseEvent ( QMouseEvent * e )
{
    if (e->button() != Qt::LeftButton)
        return;

    m_dragging = false;
    repaint(false);

    switch (m_grab_cursor) {
        case BlackCursor:
            emit modifiedBlack(m_blackcursor);
            break;
        case WhiteCursor:
            emit modifiedWhite(m_whitecursor);
            break;
        case GammaCursor:
            emit modifiedGamma(m_gamma);
            break;
    }
}

void KGradientSlider::mouseMoveEvent ( QMouseEvent * e )
{
    unsigned int x = abs(e->pos().x());

    if (m_dragging == true) // Else, drag the selected point
    {
        if (x <= m_leftmost)
            x = m_leftmost;

        if(x >= m_rightmost)
            x = m_rightmost;

        /*if(x > 255)
            x = 255;

        if(x < 0)
            x = 0;*/

        switch (m_grab_cursor) {
            case BlackCursor:
                if (m_blackcursor != x)
                {
                    m_blackcursor = x;
                    if (m_gammaEnabled) {
                        double delta = (double) (m_whitecursor - m_blackcursor) / 2.0;
                        double mid   = (double)m_blackcursor + delta;
                        double tmp   = log10 (1.0 / m_gamma);
                        m_gammacursor = (unsigned int)round(mid + delta * tmp);
                    }
                }
                break;
            case WhiteCursor:
                if (m_whitecursor != x)
                {
                    m_whitecursor = x;
                    if (m_gammaEnabled) {
                        double delta = (double) (m_whitecursor - m_blackcursor) / 2.0;
                        double mid   = (double)m_blackcursor + delta;
                        double tmp   = log10 (1.0 / m_gamma);
                        m_gammacursor = (unsigned int)round(mid + delta * tmp);
                    }
                }
                break;
            case GammaCursor:
                if (m_gammacursor != x)
                {
                    m_gammacursor = x;
                    double delta = (double) (m_whitecursor - m_blackcursor) / 2.0;
                    double mid = (double)m_blackcursor + delta;
                    double tmp = (x - mid) / delta;
                    m_gamma = 1.0 / pow (10, tmp);
                }
                break;
        }
    }

    repaint(false);
}

void KGradientSlider::leaveEvent( QEvent * )
{
}


void KGradientSlider::enableGamma(bool b)
{
    m_gammaEnabled = b;
    repaint(false);
}

double KGradientSlider::getGamma(void)
{
    return m_gamma;
}

void KGradientSlider::modifyBlack(int v) {
    if (v >= 0 && v <= (int)m_whitecursor) {
        m_blackcursor = (unsigned int)v;
        if (m_gammaEnabled) {
            double delta = (double) (m_whitecursor - m_blackcursor) / 2.0;
            double mid   = (double)m_blackcursor + delta;
            double tmp   = log10 (1.0 / m_gamma);
            m_gammacursor = (unsigned int)round(mid + delta * tmp);
        }
        repaint(false);
    }
}
void KGradientSlider::modifyWhite(int v) {
    if (v >= (int)m_blackcursor && v <= 255) {
        m_whitecursor = (unsigned int)v;
        if (m_gammaEnabled) {
            double delta = (double) (m_whitecursor - m_blackcursor) / 2.0;
            double mid   = (double)m_blackcursor + delta;
            double tmp   = log10 (1.0 / m_gamma);
            m_gammacursor = (unsigned int)round(mid + delta * tmp);
        }
        repaint(false);
    }
}
void KGradientSlider::modifyGamma(double v) {
    m_gamma = v;
    double delta = (double) (m_whitecursor - m_blackcursor) / 2.0;
    double mid   = (double)m_blackcursor + delta;
    double tmp   = log10 (1.0 / m_gamma);
    m_gammacursor = (unsigned int)round(mid + delta * tmp);
    repaint(false);
}

#include "kgradientslider.moc"
