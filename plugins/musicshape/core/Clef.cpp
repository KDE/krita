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
#include "Clef.h"

namespace MusicCore {

class Clef::Private {
public:
    ClefShape shape;
    int line;
    int octaveChange;
};

Clef::Clef(Staff* staff, int startTime, Clef::ClefShape shape, int line, int octaveChange) : StaffElement(staff, startTime), d(new Private)
{
    d->shape = shape;
    d->line = line;
    d->octaveChange = octaveChange;
    setWidth(13);
}

Clef::~Clef()
{
    delete d;
}

int Clef::priority() const
{
    return 150;
}

Clef::ClefShape Clef::shape() const
{
    return d->shape;
}

void Clef::setShape(ClefShape shape)
{
    if (d->shape == shape) return;
    d->shape = shape;
    emit shapeChanged(shape);
}

int Clef::line() const
{
    return d->line;
}

void Clef::setLine(int line)
{
    if (d->line == line) return;
    d->line = line;
    emit lineChanged(line);
}

int Clef::octaveChange() const
{
    return d->octaveChange;
}

void Clef::setOctaveChange(int octaveChange)
{
    if (d->octaveChange == octaveChange) return;
    d->octaveChange = octaveChange;
    emit octaveChangeChanged(octaveChange);
}

int Clef::lineToPitch(int line) const
{
    int pitch = 0;
    switch (d->shape) {
        case GClef: pitch = 4; break;
        case FClef: pitch = -4; break;
        case CClef: pitch = 0; break;
    }
    // d->line is the line which has pitch 'pitch' (not counting spaces between lines)
    // 'line' is the position (including spaces between lines) of which to know the pitch
    return line - 2 * d->line + 2 + pitch;
}

int Clef::pitchToLine(int pitch) const
{
    int line = 0;
    switch (d->shape) {
        case GClef: line = 14; break;
        case FClef: line = 6; break;
        case CClef: line = 10; break;
    }
    line -= 2 * d->line;
    line -= pitch;
    return line;
}
    
} // namespace MusicCore

#include <Clef.moc>
