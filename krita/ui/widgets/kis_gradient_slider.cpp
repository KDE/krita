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
#include "kis_gradient_slider.h"

// C++ includes.

#include <cmath>
#include <cstdlib>

// Qt includes.
#include <QDebug>
#include <QtGlobal>
#include <QPainter>
#include <QPoint>
#include <QPen>
#include <QMouseEvent>
#include <QBrush>
#include <QLinearGradient>

#define MARGIN 5
#define HANDLE_SIZE 10

KisGradientSlider::KisGradientSlider(QWidget *parent)
    : QWidget(parent)
    , m_leftmost(0)
    , m_rightmost(0)
    , m_scalingFactor(0)
    , m_blackCursor(0)
    , m_whiteCursor(0)
    , m_gammaCursor(0)
    , m_black(0)
    , m_white(255)
    , m_gamma(1.0)
    , m_gammaEnabled(false)
    , m_feedback(false)
{
    m_grabCursor = None;

    setMouseTracking(true);
    setFocusPolicy(Qt::StrongFocus);
}

KisGradientSlider::~KisGradientSlider()
{
}

int KisGradientSlider::black() const
{
    return m_black;
}

int KisGradientSlider::white() const
{
    return m_white;
}

void KisGradientSlider::paintEvent(QPaintEvent *e)
{
    QWidget::paintEvent(e);

    int x, y;
    int wWidth = width() - (2 * MARGIN);
    int wHeight = height();

    const int gradientHeight = qRound((double)wHeight / 7.0 * 2);

    QPainter p1(this);
    p1.fillRect(rect(), palette().background());
    p1.setPen(Qt::black);
    p1.drawRect(MARGIN, MARGIN, wWidth, height() - 2 * MARGIN - HANDLE_SIZE);

    // Draw first gradient
    QLinearGradient grayGradient(MARGIN, 0, wWidth, gradientHeight);
    grayGradient.setColorAt(0, Qt::black);
    grayGradient.setColorAt(1, Qt::white);
    p1.fillRect(MARGIN, 0, wWidth, gradientHeight, QBrush(grayGradient));

    // Draw second gradient
    y = gradientHeight;
    p1.fillRect(MARGIN, y, wWidth, gradientHeight, Qt::white);

    if (m_blackCursor > 0) {
        p1.fillRect(MARGIN, y, m_blackCursor, gradientHeight, Qt::black);
    }

    for (x = (int)m_blackCursor + MARGIN; x < (int)m_whiteCursor - MARGIN; ++x) {
        double inten = (double)(x - (m_blackCursor + MARGIN)) / (double)((m_whiteCursor - MARGIN) - (m_blackCursor + MARGIN));
        inten = pow(inten, (1.0 / m_gamma));
        int gray = (int)(255 * inten);
        p1.setPen(QColor(gray, gray, gray));
        p1.drawLine(x, y, x, y + gradientHeight - 1);
    }

    // Draw cursors
    y += gradientHeight;
    QPoint a[3];
    p1.setPen(Qt::darkGray);
    p1.setRenderHint(QPainter::Antialiasing, true);

    const int cursorHalfBase = (int)(gradientHeight / 1.5);

    a[0] = QPoint(m_blackCursor + MARGIN, y);
    a[1] = QPoint(m_blackCursor + MARGIN + cursorHalfBase, wHeight - 1);
    a[2] = QPoint(m_blackCursor + MARGIN - cursorHalfBase, wHeight - 1);
    p1.setBrush(Qt::black);
    p1.drawPolygon(a, 3);

    p1.setPen(Qt::black);
    if (m_gammaEnabled) {
        a[0] = QPoint(m_gammaCursor, y);
        a[1] = QPoint(m_gammaCursor + cursorHalfBase, wHeight - 1);
        a[2] = QPoint(m_gammaCursor - cursorHalfBase, wHeight - 1);
        p1.setBrush(Qt::gray);
        p1.drawPolygon(a, 3);
    }

    a[0] = QPoint(m_whiteCursor - MARGIN, y);
    a[1] = QPoint(m_whiteCursor - MARGIN + cursorHalfBase, wHeight - 1);
    a[2] = QPoint(m_whiteCursor - MARGIN - cursorHalfBase, wHeight - 1);
    p1.setBrush(Qt::white);
    p1.drawPolygon(a, 3);
}

void KisGradientSlider::resizeEvent(QResizeEvent *)
{
    m_scalingFactor = (double)(width() - MARGIN) / 255;
    calculateCursorPositions();
    update();
}

void KisGradientSlider::mousePressEvent(QMouseEvent * e)
{
    eCursor closest_cursor;
    int distance;

    if (e->button() != Qt::LeftButton)
        return;

    unsigned int x = e->pos().x();
    int xPlusMargin = x + MARGIN;

    distance = width() + 1; // just a big number

    if (abs((int)(xPlusMargin - m_blackCursor)) < distance) {
        distance = abs((int)(xPlusMargin - m_blackCursor));
        closest_cursor = BlackCursor;
    }

    if (abs((int)(xPlusMargin - m_whiteCursor)) < distance) {
        distance = abs((int)(xPlusMargin - m_whiteCursor));
        closest_cursor = WhiteCursor;
    }

    if (m_gammaEnabled) {
        int gammaDistance = (int)xPlusMargin - m_gammaCursor;

        if (abs(gammaDistance) < distance) {
            distance = abs((int)xPlusMargin - m_gammaCursor);
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
        m_blackCursor = x - MARGIN;
        m_grabCursor = closest_cursor;
        m_leftmost = 0;
        m_rightmost = m_whiteCursor - ((MARGIN + 1) * m_scalingFactor);
        if (m_gammaEnabled)
            m_gammaCursor = calculateGammaCursor();
        break;
    case WhiteCursor:
        m_whiteCursor = x + MARGIN;
        m_grabCursor = closest_cursor;
        m_leftmost = m_blackCursor + (MARGIN * m_scalingFactor);
        m_rightmost = width() - MARGIN ;
        if (m_gammaEnabled)
            m_gammaCursor = calculateGammaCursor();
        break;
    case GammaCursor:
        m_gammaCursor = x;
        m_grabCursor = closest_cursor;
        m_leftmost = m_blackCursor + (MARGIN * m_scalingFactor);
        m_rightmost = m_whiteCursor - (MARGIN * m_scalingFactor);
    {
        double delta = (double)(m_whiteCursor - m_blackCursor) / 2.0;
        double mid = (double)m_blackCursor + delta + MARGIN;
        double tmp = (x - mid) / delta;
        m_gamma = 1.0 / pow(10, tmp);
    }
        break;
    default:
        break;
    }
    update();
}

void KisGradientSlider::mouseReleaseEvent(QMouseEvent * e)
{
    if (e->button() != Qt::LeftButton)
        return;

    update();

    switch (m_grabCursor) {
    case BlackCursor:
        m_black = qRound( m_blackCursor / m_scalingFactor);
        m_feedback = true;
        emit sigModifiedBlack(m_black);
        break;
    case WhiteCursor:
        m_white = qRound( (m_whiteCursor - MARGIN) / m_scalingFactor);
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

void KisGradientSlider::mouseMoveEvent(QMouseEvent * e)
{
    int x = e->pos().x();

    if (m_grabCursor != None) { // Else, drag the selected point
        if (x + MARGIN <= m_leftmost)
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
                m_whiteCursor = x + MARGIN;
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

void KisGradientSlider::calculateCursorPositions()
{
    m_blackCursor = qRound(m_black * m_scalingFactor);
    m_whiteCursor = qRound(m_white * m_scalingFactor + MARGIN);

    m_gammaCursor = calculateGammaCursor();
}

unsigned int KisGradientSlider::calculateGammaCursor()
{
    double delta = (double)(m_whiteCursor - m_blackCursor) / 2.0;
    double mid   = (double)m_blackCursor + delta;
    double tmp   = log10(1.0 / m_gamma);
    return (unsigned int)qRound(mid + delta * tmp);
}


void KisGradientSlider::enableGamma(bool b)
{
    m_gammaEnabled = b;
    update();
}

double KisGradientSlider::getGamma(void)
{
    return m_gamma;
}

void KisGradientSlider::slotModifyBlack(int v)
{
    if (v >= 0 && v <= (int)m_white && !m_feedback) {
        m_black = v;
        m_blackCursor = qRound(m_black * m_scalingFactor);
        m_gammaCursor = calculateGammaCursor();
        update();
    }
}
void KisGradientSlider::slotModifyWhite(int v)
{
    if (v >= (int)m_black && v <= width() && !m_feedback) {
        m_white = v;
        m_whiteCursor = qRound(m_white * m_scalingFactor + MARGIN);
        m_gammaCursor = calculateGammaCursor();
        update();
    }
}
void KisGradientSlider::slotModifyGamma(double v)
{
    if (m_gamma != v) {
        emit sigModifiedGamma(v);
    }
    m_gamma = v;
    m_gammaCursor = calculateGammaCursor();
    update();
}

