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

void Ruler::moveTo(qreal value, bool smooth)
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

void Ruler::resetValue()
{
    setValue(oldValue());
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

