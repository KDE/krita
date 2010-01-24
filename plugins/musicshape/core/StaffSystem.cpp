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
#include "StaffSystem.h"
#include "Clef.h"
#include "Sheet.h"

namespace MusicCore {

class StaffSystem::Private
{
public:
    qreal top;
    qreal height;
    int firstBar;
    qreal indent;
    qreal lineWidth;
    QList<Clef*> clefs;
};

StaffSystem::StaffSystem(Sheet* sheet)
    : QObject(sheet), d(new Private)
{
    d->top = 0.0;
    d->height = 100.0;
    d->firstBar = 0;
    d->indent = 0;
    d->lineWidth = 100;
}

StaffSystem::~StaffSystem()
{
    delete d;
}

qreal StaffSystem::top() const
{
    return d->top;
}

qreal StaffSystem::height() const
{
    return d->height;
}

void StaffSystem::setHeight(qreal height)
{
    d->height = height;
}

void StaffSystem::setTop(qreal top)
{
    if (d->top == top) return;
    d->top = top;
    emit topChanged(top);
}

int StaffSystem::firstBar() const
{
    return d->firstBar;
}

void StaffSystem::setFirstBar(int bar)
{
    if (d->firstBar == bar) return;
    d->firstBar = bar;
    emit firstBarChanged(bar);
}

qreal StaffSystem::indent() const
{
    return d->indent;
}

void StaffSystem::setIndent(qreal indent)
{
    d->indent = indent;
}

void StaffSystem::setLineWidth(qreal width)
{
    d->lineWidth = width;
}

QList<Clef*> StaffSystem::clefs() const
{
    return d->clefs;
}

Clef* StaffSystem::clef(Staff* staff) const
{
    foreach (Clef* c, d->clefs) {
        if (c->staff() == staff) {
            return c;
        }
    }
    return NULL;
}

void StaffSystem::setClefs(QList<Clef*> clefs)
{
    d->clefs = clefs;
}

} // namespace MusicCore

#include <StaffSystem.moc>
