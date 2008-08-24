/* This file is part of the KDE project
 * Copyright (C) 2008 Florian Merz <florianmerz@gmx.de>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "RulerFragment.h"

#include "Ruler.h"

bool RulerFragment::hitTest(const QPointF &point) const
{
    QRectF rect(m_ruler->value() - 5.0, 0.0, 10.0, m_width);
    return rect.contains(m_matrix.inverted().map(point));
}

void RulerFragment::moveTo(const QPointF &point, bool smoothMovement) const
{
    m_ruler->moveTo(m_matrix.inverted().map(point).x(), smoothMovement);
}

QLineF RulerFragment::labelConnector() const
{
    qreal value = m_ruler->value();
    return m_matrix.map(QLineF(value/2.0, m_width/2.0, value/2.0, m_width/2.0 + 1.0));
}

void RulerFragment::paint(QPainter &painter) const
{
    if (!m_visible) {
        return;
    }

    painter.save();

    painter.setWorldMatrix(m_matrix, true);

    qreal oldValue = m_ruler->oldValue();
    qreal value = m_ruler->value();

    // draw dark gray square for old value
    painter.setPen(QPen(Qt::darkGray));

    if (value != 0.0)
        painter.drawLine(QLineF(0.0, 0.0, 0.0, m_width));

    if (oldValue != value && oldValue != 0.0)
        painter.drawLine(QLineF(oldValue, 0.0, oldValue, m_width));

    if (m_ruler->options() & Ruler::drawSides) {
        painter.drawLine(QLineF(0.0, 0.0, oldValue, 0.0));
        painter.drawLine(QLineF(0.0, m_width, oldValue, m_width));
    }

    // draw arrow and ruler line for current value

    if (m_ruler->isActive())
        painter.setPen(m_ruler->activeColor());
    else if (m_ruler->isHighlighted())
        painter.setPen(m_ruler->highlightColor());
    else if (m_ruler->isFocused())
        painter.setPen(m_ruler->focusColor());
    else
        painter.setPen(m_ruler->normalColor());

    painter.drawLine(QLineF(value, 0.0, value, m_width));

    painter.drawLine(QLineF(0.0, m_width/2.0, value, m_width/2.0));

    if (value >= 0.0) {
        paintArrow(painter, QPointF(value, m_width/2.0), 0.0, value);
    }
    else {
        paintArrow(painter, QPointF(value, m_width/2.0), 180.0, -value);
    }

    painter.restore();
}

void RulerFragment::paintArrow(QPainter &painter, const QPointF &tip, const qreal angle, qreal value) const
{
    painter.save();

    painter.translate(tip);

    painter.rotate(angle);

    QLineF arrowLeft(-arrowDiagonal(), arrowDiagonal(), 0.0, 0.0);
    painter.drawLine(arrowLeft);

    QLineF arrowRight(-arrowDiagonal(), -arrowDiagonal(), 0.0, 0.0);
    painter.drawLine(arrowRight);

    if (value < arrowMinimumValue()) {
        QLineF arrowMiddle(-arrowSize(), 0.0, 0.0, 0.0);
        painter.drawLine(arrowMiddle);
    }

    painter.restore();
}

void RulerFragment::setBaseline(const QLineF &baseline)
{
    m_matrix.reset();
    m_matrix.translate(baseline.p1().x(), baseline.p1().y());
    m_matrix.rotate(baseline.angle(QLineF(0.0, 0.0, 0.0, 1.0)));

    m_width = baseline.length();
}

void RulerFragment::setVisible(bool visible)
{
    m_visible = visible;
}

