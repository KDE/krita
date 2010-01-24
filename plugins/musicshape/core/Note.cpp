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
#include "Note.h"
#include "Chord.h"

namespace MusicCore {

class Note::Private {
public:
    Staff* staff;
    int pitch;
    int accidentals;
    bool tied;
    bool drawAccidentals;
};

Note::Note(Chord* chord, Staff* staff, int pitch, int accidentals) : QObject(chord), d(new Private)
{
    d->staff = staff;
    d->pitch = pitch;
    d->accidentals = accidentals;
    d->tied = false;
    d->drawAccidentals = false;
}

Note::~Note()
{
    delete d;
}

Staff* Note::staff()
{
    return d->staff;
}

Chord* Note::chord()
{
    return qobject_cast<Chord*>(parent());
}

void Note::setStaff(Staff* staff)
{
    d->staff = staff;
}

int Note::pitch() const
{
    return d->pitch;
}

int Note::accidentals() const
{
    return d->accidentals;
}

void Note::setAccidentals(int accidentals)
{
    d->accidentals = accidentals;
}

bool Note::drawAccidentals() const
{
    return d->drawAccidentals;
}

void Note::setDrawAccidentals(bool drawAccidentals)
{
    d->drawAccidentals = drawAccidentals;
}

bool Note::isStartTie() const
{
    return d->tied;
}

void Note::setStartTie(bool startTie)
{
    d->tied = startTie;
}

} // namespace MusicCore

#include <Note.moc>
