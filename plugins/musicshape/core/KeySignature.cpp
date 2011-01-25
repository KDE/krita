/* This file is part of the KDE project
 * Copyright (C) 2007 Marijn Kruisselbrink <mkruisselbrink@kde.org>
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
#include "KeySignature.h"
#include <cstdlib>

namespace MusicCore {

class KeySignature::Private {
public:
    int accidentalCount;
    int accidentals[7];
    int cancelCount;
    int cancel[7];
};

KeySignature::KeySignature(Staff* staff, int startTime, int accidentals, int cancel) : StaffElement(staff, startTime), d(new Private)
{
    d->accidentalCount = 9999; // just some random illegal value
    d->cancelCount = 9999;
    setAccidentals(accidentals);
    setCancel(cancel);
}

KeySignature::~KeySignature()
{
    delete d;
}

int KeySignature::priority() const
{
    return 100;
}

int KeySignature::accidentals() const
{
    int sum = 0;
    for (int i = 0; i < 7; i++) {
        sum += d->accidentals[i];
    }
    return sum;
}

void KeySignature::setAccidentals(int accidentals)
{
    if (d->accidentalCount == accidentals) return;

    d->accidentalCount = accidentals;

    // first zero the accidentals array
    for (int i = 0; i < 7; i++) {
        d->accidentals[i] = 0;
    }

    // now add sharps
    int idx = 3;
    for (int i = 0; i < accidentals; i++) {
        d->accidentals[idx]++;
        idx = (idx + 4) % 7;
    }

    // and flats
    idx = 6;
    for (int i = 0; i > accidentals; i--) {
        d->accidentals[idx]--;
        idx = (idx + 3) % 7;
    }

    setWidth(6 * std::abs(accidentals) + 6 * std::abs(d->cancelCount));

    emit accidentalsChanged(accidentals);
}

int KeySignature::accidentals(int pitch) const
{
    return d->accidentals[ ((pitch % 7) + 7) % 7 ];
}


int KeySignature::cancel() const
{
    int sum = 0;
    for (int i = 0; i < 7; i++) {
        sum += d->cancel[i];
    }
    return sum;
}

void KeySignature::setCancel(int cancel)
{
    if (d->cancelCount == cancel) return;
    
    d->cancelCount = cancel;
    
    // first zero the accidentals array
    for (int i = 0; i < 7; i++) {
        d->cancel[i] = 0;
    }
    
    // now add sharps
    int idx = 3;
    for (int i = 0; i < cancel; i++) {
        d->cancel[idx]++;
        idx = (idx + 4) % 7;
    }
    
    // and flats
    idx = 6;
    for (int i = 0; i > cancel; i--) {
        d->cancel[idx]--;
        idx = (idx + 3) % 7;
    }
    
    setWidth(6 * std::abs(d->accidentalCount) + 6 * std::abs(d->cancelCount));
}

int KeySignature::cancel(int pitch) const
{
    return d->cancel[ ((pitch % 7) + 7) % 7 ];
}

} // namespace MusicCore

