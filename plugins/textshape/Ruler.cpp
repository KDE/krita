/* This file is part of the KDE project
 * Copyright (C) 2008 Florian Merz <florianmerz@web.de>
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

#include "Ruler.h"

#include <KDebug>

#include <QColor>
#include <QPointF>
#include <QPainter>
#include <QRectF>

#include <limits>
#include <cmath>


Ruler::Ruler(QObject *parent)
    : QObject(parent),
    m_value(0.0),
    m_oldValue(0.0),
    m_stepValue(10.0),
    m_minValue(-std::numeric_limits<qreal>::infinity()),
    m_maxValue(std::numeric_limits<qreal>::infinity()),
    m_visible(true),
    m_active(false),
    m_focused(false),
    m_highlighted(false),
    m_options(noOptions)
{}

void Ruler::setUnit(KoUnit unit)
{
    m_unit = unit;

    // approximately 15 points seems to be a good value for the step size
    switch (m_unit.indexInList(false)) {
        case KoUnit::Millimeter:
            setStepValue(14.17325288515625502486); // 5.0 mm
            break;
        case KoUnit::Point:
            setStepValue(15.0); // 15 pt
            break;
        case KoUnit::Inch:
            setStepValue(14.399999999998848); // 0.2 inch
            break;
        case KoUnit::Centimeter:
            setStepValue(14.17325288515625502486); // 0.5 cm
            break;
        case KoUnit::Decimeter:
            setStepValue(14.17325288515625502486); // 0.05 dm
            break;
        case KoUnit::Pica:
            setStepValue(15.00000006000000024); // 1.25 pica
            break;
        case KoUnit::Cicero:
            setStepValue(12.84010270181826254741); // 1 cicero
            break;
        case KoUnit::Pixel:
        default:
            setStepValue(15.0);
            break;
    }
}

qreal Ruler::value() const
{
    return m_value;
}

QString Ruler::valueString() const
{
    QString ret;
    ret.append(m_unit.toUserStringValue(m_value));
    ret.append(KoUnit::unitName(m_unit));
    return ret;
}

void Ruler::setValue(qreal value)
{
    setOldValue(value);

    if (value != m_value) {
        m_value = value;
        emit needsRepaint();
    }
}

void Ruler::reset()
{
    setValue(oldValue());
}

void Ruler::moveRuler(const QPointF &point, bool smooth, QLineF baseline)
{
    QMatrix matrix;
    matrix.translate(baseline.p1().x(), baseline.p1().y());
    matrix.rotate(baseline.angle(QLineF(0.0, 0.0, 0.0, 1.0)));

    moveRuler(point, smooth, matrix);
}

void Ruler::moveRuler(const QPointF &point, bool smooth, QMatrix matrix)
{
    moveRuler(matrix.inverted().map(point).x(), smooth);
}

void Ruler::moveRuler(qreal value, bool smooth)
{
    qreal newValue;

    if (value < minimumValue())
        newValue = minimumValue();
    else if (value > maximumValue())
        newValue = maximumValue();
    else {
        if (smooth || m_stepValue == 0.0) {
            newValue = value;
        }
        else if (value > 0.0) {
            newValue = value - fmod(value + m_stepValue * 0.5, m_stepValue) + m_stepValue * 0.5;
        }
        else {
            newValue = value - fmod(value - m_stepValue * 0.5, m_stepValue) - m_stepValue * 0.5;
        }
    }

    if (newValue != m_value) {
        m_value = newValue;
        emit needsRepaint();
    }
}

qreal Ruler::minimumValue() const
{
    return m_minValue;
}
void Ruler::setMinimumValue(qreal value)
{
    m_minValue = value;
}

qreal Ruler::maximumValue() const
{
    return m_maxValue;
}

void Ruler::setMaximumValue(qreal value)
{
    m_maxValue = value;
}

void Ruler::setActive(bool active)
{
    if (m_active == true && active == false && m_oldValue != m_value) {
        m_active = false;
        m_oldValue = m_value;
        emit valueChanged(m_value);
    }
    else if (m_active != active) {
        m_active = active;
        emit needsRepaint();
    }
}

void Ruler::setFocused(bool focused)
{
    m_focused = focused;
    emit needsRepaint();
}

void Ruler::setHighlighted(bool highlighted)
{
    m_highlighted = highlighted;
    emit needsRepaint();
}

QLineF Ruler::labelConnector(const QLineF &baseline) const
{
    QMatrix matrix;
    matrix.translate(baseline.p1().x(), baseline.p1().y());
    matrix.rotate(baseline.angle(QLineF(0.0, 0.0, 0.0, 1.0)));

    return labelConnector(matrix, baseline.length());
}

QLineF Ruler::labelConnector(const QMatrix &matrix, qreal width) const
{
    return matrix.map(QLineF(value()/2.0, width/2.0, value()/2.0, width/2.0 + 1.0));
}

void Ruler::paint(QPainter &painter, const QLineF &baseline) const
{
    QMatrix matrix;
    matrix.translate(baseline.p1().x(), baseline.p1().y());
    matrix.rotate(baseline.angle(QLineF(0.0, 0.0, 0.0, 1.0)));

    paint(painter, matrix, baseline.length());
}

void Ruler::paint(QPainter & painter, const QMatrix &matrix, qreal width) const
{
    if (!isVisible()) {
        return;
    }

    painter.save();

    painter.setWorldMatrix(matrix, true);

    // draw dark gray square for old value

    painter.setPen(QPen(Qt::darkGray));

    if (value() != 0.0)
        painter.drawLine(QLineF(0.0, 0.0, 0.0, width));

    if (oldValue() != value() && oldValue() != 0.0)
        painter.drawLine(QLineF(oldValue(), 0.0, oldValue(), width));

    if (options() & drawSides) {
        painter.drawLine(QLineF(0.0, 0.0, oldValue(), 0.0));
        painter.drawLine(QLineF(0.0, width, oldValue(), width));
    }

    // draw arrow and ruler line for current value

    if (isActive())
        painter.setPen(activeColor());
    else if (isHighlighted())
        painter.setPen(highlightColor());
    else if (isFocused())
        painter.setPen(focusColor());
    else
        painter.setPen(normalColor());

    painter.drawLine(QLineF(value(), 0.0, value(), width));

    painter.drawLine(QLineF(0.0, width/2.0, value(), width/2.0));

    if (value() >= 0.0) {
        paintArrow(painter, QPointF(value(), width/2.0), 0.0, value());
    }
    else {
        paintArrow(painter, QPointF(value(), width/2.0), 180.0, -value());
    }

    painter.restore();
}

void Ruler::paintArrow(QPainter &painter, const QPointF &tip, const qreal angle, qreal value) const
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

bool Ruler::hitTest(const QPointF &point, const QLineF &baseline) const
{
    QMatrix matrix;
    matrix.translate(baseline.p1().x(), baseline.p1().y());
    matrix.rotate(baseline.angle(QLineF(0.0, 0.0, 0.0, 1.0)));

    return hitTest(point, matrix, baseline.length());
}

bool Ruler::hitTest(const QPointF &point, const QMatrix &matrix, qreal width) const
{
    QRectF rect(value() - 5.0, 0.0, 10.0, width);
    return rect.contains(matrix.inverted().map(point));
}

