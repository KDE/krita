/* This file is part of the KDE project
 * Copyright (C) 2007 Marijn Kruisselbrink <m.kruisselbrink@student.tue.nl>
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
#include "StaffElement.h"

namespace MusicCore {

class StaffElement::Private
{
public:
    Staff* staff;
    Bar* bar;
    int startTime;
    qreal x;
    qreal y;
    qreal width;
    qreal height;
};

StaffElement::StaffElement(Staff* staff, int startTime) : d(new Private)
{
    d->staff = staff;
    d->bar = NULL;
    d->startTime = startTime;
    d->x = 0;
    d->y = 0;
    d->width = 0;
    d->height = 0;
}

StaffElement::~StaffElement()
{
    delete d;
}

Staff* StaffElement::staff()
{
    return d->staff;
}

Bar* StaffElement::bar()
{
    return d->bar;
}

void StaffElement::setBar(Bar* bar)
{
    d->bar = bar;
}

qreal StaffElement::x() const
{
    return d->x;
}

void StaffElement::setX(qreal x)
{
    if (d->x == x) return;
    d->x = x;
    emit xChanged(x);
}

qreal StaffElement::y() const
{
    return d->y;
}

void StaffElement::setY(qreal y)
{
    if (d->y == y) return;
    d->y = y;
    emit yChanged(y);
}

qreal StaffElement::width() const
{
    return d->width;
}

void StaffElement::setWidth(qreal width)
{
    if (d->width == width) return;
    d->width = width;
    emit widthChanged(width);
}

qreal StaffElement::height() const
{
    return d->height;
}

void StaffElement::setHeight(qreal height)
{
    if (d->height == height) return;
    d->height = height;
    emit heightChanged(height);
}

int StaffElement::startTime() const
{
    return d->startTime;
}

void StaffElement::setStartTime(int startTime)
{
    if (d->startTime == startTime) return;
    d->startTime = startTime;
    emit startTimeChanged(startTime);
}

} // namespace MusicCore

#include <StaffElement.moc>
