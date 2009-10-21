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

// Local includes.
#include "kgradientslider.h"

// C++ includes.

#include <cmath>
#include <cstdlib>

// Qt includes.

#include <QPixmap>
#include <QPainter>
#include <QPoint>
#include <QPen>
#include <QMouseEvent>
#include <QBrush>
#include <QLinearGradient>



KGradientSlider::KGradientSlider(QWidget *parent)
        : QWidget(parent), m_black(0), m_white(255), m_gamma(1.0), m_gammaEnabled(false)
{
    m_feedback = false;
    m_grabCursor = None;

    setMouseTracking(true);
    setFocusPolicy(Qt::StrongFocus);
}

KGradientSlider::~KGradientSlider()
{
}

void KGradientSlider::paintEvent(QPaintEvent *)
{
    int x, y;
    int wWidth = width();
    int wHeight = height();

    const int gradientHeight = (int)round((double)wHeight / 7.0 * 2);

    // A QPixmap is used for enable the double buffering.
    QPainter p1(this);

    // Draw first gradient
    QLinearGradient grayGradient(0, 0, wWidth - 1, gradientHeight);
    grayGradient.setColorAt(0, Qt::black);
    grayGradient.setColorAt(1, Qt::white);
    p1.fillRect(0, 0, wWidth, gradientHeight, QBrush(grayGradient));

    // Draw second gradient
    y = gradientHeight;
    if (m_blackCursor > 0) {
        p1.fillRect(0, y, (int)m_blackCursor, gradientHeight, QBrush(Qt::black));
    }
    if (m_whiteCursor < wWidth) {
        p1.fillRect((int)m_whiteCursor, y, wWidth, gradientHeight, QBrush(Qt::white));
    }
    for (x = (int)m_blackCursor; x < (int)m_whiteCursor; ++x) {
        double inten = (double)(x - m_blackCursor) / (double)(m_whiteCursor - m_blackCursor);
        inten = pow(inten, (1.0 / m_gamma));
        int gray = (int)(255 * inten);
        p1.setPen(QColor(gray, gray, gray));
        p1.drawLine(x, y, x, y + gradientHeight - 1);
    }

    // Draw cursors
    y += gradientHeight;
    QPoint a[3];
    p1.setPen(Qt::black);
    p1.setRenderHint(QPainter::Antialiasing, true);

    const int cursorHalfBase = (int)(gradientHeight / 1.5);

    a[0] = QPoint(m_blackCursor, y);
    a[1] = QPoint(m_blackCursor + cursorHalfBase, wHeight - 1);
    a[2] = QPoint(m_blackCursor - cursorHalfBase, wHeight - 1);
    p1.setBrush(Qt::black);
    p1.drawPolygon(a, 3);

    if (m_gammaEnabled) {
        a[0] = QPoint(m_gammaCursor, y);
        a[1] = QPoint(m_gammaCursor + cursorHalfBase, wHeight - 1);
        a[2] = QPoint(m_gammaCursor - cursorHalfBase, wHeight - 1);
        p1.setBrush(Qt::gray);
        p1.drawPolygon(a, 3);
    }

    a[0] = QPoint(m_whiteCursor, y);
    a[1] = QPoint(m_whiteCursor + cursorHalfBase, wHeight - 1);
    a[2] = QPoint(m_whiteCursor - cursorHalfBase, wHeight - 1);
    p1.setBrush(Qt::white);
    p1.drawPolygon(a, 3);
}

void KGradientSlider::resizeEvent(QResizeEvent *)
{
    m_scalingFactor = (double)width() / 255;
    calculateCursorPositions();
    update();
}

void KGradientSlider::mousePressEvent(QMouseEvent * e)
{
    eCursor closest_cursor;
    int distance;

    if (e->button() != Qt::LeftButton)
        return;

    unsigned int x = e->pos().x();

    distance = width() + 1; // just a big number

    if (abs((int)(x - m_blackCursor)) < distance) {
        distance = abs((int)(x - m_blackCursor));
        closest_cursor = BlackCursor;
    }

    if (abs((int)(x - m_whiteCursor)) < distance) {
        distance = abs((int)(x - m_whiteCursor));
        closest_cursor = WhiteCursor;
    }

    if (m_gammaEnabled) {
        int gammaDistance = (int)x - m_gammaCursor;

        if (abs(gammaDistance) < distance) {
            distance = abs((int)x - m_gammaCursor);
            closest_cursor = GammaCursor;
        } else if (abs(gammaDistance) == distance) {
            if ((closest_cursor == BlackCursor) && (gammaDistance > 0)) {
                distance = abs(gammaDistance);
                closest_cursor = GammaCursor;
            } else if ((closest_cursor == WhiteCursor) && (gammaDistance < 0)) {
                distance = abs(gammaDistance);
                closest_cursor = GammaCursor;
            }
        }
    }

    if (distance > 20) {
        m_grabCursor = None;
        return;
    }

    // Determine cursor values and the leftmost and rightmost points.

    switch (closest_cursor) {
    case BlackCursor:
        m_blackCursor = x;
        m_grabCursor = closest_cursor;
        m_leftmost = 0;
        m_rightmost = m_whiteCursor - 1;
        if (m_gammaEnabled)
            m_gammaCursor = calculateGammaCursor();
        break;
    case WhiteCursor:
        m_whiteCursor = x;
        m_grabCursor = closest_cursor;
        m_leftmost = m_blackCursor + 1;
        m_rightmost = width();
        if (m_gammaEnabled)
            m_gammaCursor = calculateGammaCursor();
        break;
    case GammaCursor:
        m_gammaCursor = x;
        m_grabCursor = closest_cursor;
        m_leftmost = m_blackCursor;
        m_rightmost = m_whiteCursor;
        {
            double delta = (double)(m_whiteCursor - m_blackCursor) / 2.0;
            double mid = (double)m_blackCursor + delta;
            double tmp = (x - mid) / delta;
            m_gamma = 1.0 / pow(10, tmp);
        }
        break;
    default:
        break;
    }
    update();
}

void KGradientSlider::mouseReleaseEvent(QMouseEvent * e)
{
    if (e->button() != Qt::LeftButton)
        return;

    update();

    switch (m_grabCursor) {
    case BlackCursor:
        m_black = (int)round(m_blackCursor / m_scalingFactor);
        m_feedback = true;
        emit sigModifiedBlack(m_black);
        break;
    case WhiteCursor:
        m_white = (int)round(m_whiteCursor / m_scalingFactor);
        m_feedback = true;
        emit sigModifiedWhite(m_white);
        break;
    case GammaCursor:
        emit sigModifiedGamma(m_gamma);
        break;
    default:
        break;
    }

    m_grabCursor = None;
    m_feedback = false;
}

void KGradientSlider::mouseMoveEvent(QMouseEvent * e)
{
    int x = e->pos().x();

    if (m_grabCursor != None) { // Else, drag the selected point
        if (x <= m_leftmost)
            x = m_leftmost;

        if (x >= m_rightmost)
            x = m_rightmost;

        switch (m_grabCursor) {
        case BlackCursor:
            if (m_blackCursor != x) {
                m_blackCursor = x;
                if (m_gammaEnabled) {
                    m_gammaCursor = calculateGammaCursor();
                }
            }
            break;
        case WhiteCursor:
            if (m_whiteCursor != x) {
                m_whiteCursor = x;
                if (m_gammaEnabled) {
                    m_gammaCursor = calculateGammaCursor();
                }
            }
            break;
        case GammaCursor:
            if (m_gammaCursor != x) {
                m_gammaCursor = x;
                double delta = (double)(m_whiteCursor - m_blackCursor) / 2.0;
                double mid = (double)m_blackCursor + delta;
                double tmp = (x - mid) / delta;
                m_gamma = 1.0 / pow(10, tmp);
            }
            break;
        default:
            break;
        }
    }

    update();
}

void KGradientSlider::calculateCursorPositions()
{
    m_blackCursor = (int)round(m_black * m_scalingFactor);
    m_whiteCursor = (int)round(m_white * m_scalingFactor);
    m_gammaCursor = calculateGammaCursor();
}

unsigned int KGradientSlider::calculateGammaCursor()
{
    double delta = (double)(m_whiteCursor - m_blackCursor) / 2.0;
    double mid   = (double)m_blackCursor + delta;
    double tmp   = log10(1.0 / m_gamma);
    return (unsigned int)qRound(mid + delta * tmp);
}


void KGradientSlider::enableGamma(bool b)
{
    m_gammaEnabled = b;
    update();
}

double KGradientSlider::getGamma(void)
{
    return m_gamma;
}

void KGradientSlider::slotModifyBlack(int v)
{
    if (v >= 0 && v <= (int)m_white && !m_feedback) {
        m_black = v;
        m_blackCursor = (int)round(m_black * m_scalingFactor);
        m_gammaCursor = calculateGammaCursor();
        update();
    }
}
void KGradientSlider::slotModifyWhite(int v)
{
    if (v >= (int)m_black && v <= width() && !m_feedback) {
        m_white = v;
        m_whiteCursor = (int)round(m_white * m_scalingFactor);
        m_gammaCursor = calculateGammaCursor();
        update();
    }
}
void KGradientSlider::slotModifyGamma(double v)
{
    if (m_gamma != v) {
        emit sigModifiedGamma(v);
    }
    m_gamma = v;
    m_gammaCursor = calculateGammaCursor();
    update();
}

#include "kgradientslider.moc"
