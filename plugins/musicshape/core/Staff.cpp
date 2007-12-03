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
#include "Staff.h"
#include "Part.h"
#include "Sheet.h"
#include "Bar.h"
#include "StaffElement.h"
#include "Clef.h"
#include "KeySignature.h"
#include "TimeSignature.h"

#include <math.h>
#include <limits.h>

namespace MusicCore {

class Staff::Private
{
public:
    double spacing;
    int lineCount;
    double lineSpacing;
};

Staff::Staff(Part* part) : QObject(part), d(new Private)
{
    d->spacing = 60;
    d->lineCount = 5;
    d->lineSpacing = 5.0;
}

Staff::~Staff()
{
    delete d;
}

Part* Staff::part()
{
    return qobject_cast<Part*>(parent());
}

double Staff::spacing() const
{
    return d->spacing;
}

void Staff::setSpacing(double spacing)
{
    if (d->spacing == spacing) return;
    d->spacing = spacing;
    emit spacingChanged(spacing);
}

double Staff::top()
{
    if (!part()) return 0;
    int n = 0;
    for (int i = 0; i < part()->sheet()->partCount(); i++) {
        Part* p = part()->sheet()->part(i);
        if (p != part()) n += p->staffCount();
        else break;
    }
    for (int i = 0; i < part()->staffCount(); i++) {
        if (part()->staff(i) == this) return 30 + 50 * (n+i);
    }
    return 30;
}

double Staff::bottom()
{
    return top() + lineSpacing() * (lineCount() - 1);
}

double Staff::center()
{
    return top() + bottom() / 2;
}

int Staff::lineCount() const
{
    return d->lineCount;
}

void Staff::setLineCount(int lineCount)
{
    if (d->lineCount == lineCount) return;
    d->lineCount = lineCount;
    emit lineCountChanged(lineCount);
}

double Staff::lineSpacing() const
{
    return d->lineSpacing;
}

void Staff::setLineSpacing(double lineSpacing)
{
    if (d->lineSpacing == lineSpacing) return;
    d->lineSpacing = lineSpacing;
    emit lineSpacingChanged(lineSpacing);
}

int Staff::line(double y) const
{
    y = (lineCount()-1) * lineSpacing() - y;
    y /= lineSpacing() / 2;
    return (int) round(y);
}

Clef* Staff::lastClefChange(int bar, int time, Clef* oldClef)
{
    if (!part()) return NULL;
    
    if (time < 0) time = INT_MAX;
    for (int b = bar; b >= 0; b--) {
        Bar* curBar = part()->sheet()->bar(b);
        for (int i = curBar->staffElementCount(this)-1; i >= 0; i--) {
            StaffElement* e = curBar->staffElement(this, i);
            if (e->startTime() <= time) {
                Clef* c = dynamic_cast<Clef*>(e);
                if (c) return c;
            }
        }

        if (oldClef) return oldClef;
        time = INT_MAX;
    }
    return 0;
}

Clef* Staff::lastClefChange(Bar* bar, int time, Clef* oldClef)
{
    return lastClefChange(part()->sheet()->indexOfBar(bar), time, oldClef);
}

KeySignature* Staff::lastKeySignatureChange(int bar)
{
    if (!part()) return NULL;
    
    for (int b = bar; b >= 0; b--) {
        Bar* curBar = part()->sheet()->bar(b);
        for (int i = curBar->staffElementCount(this)-1; i >= 0; i--) {
            StaffElement* e = curBar->staffElement(this, i);
            KeySignature* ks = dynamic_cast<KeySignature*>(e);
            if (ks) return ks;
        }
    }
    return 0;
}

KeySignature* Staff::lastKeySignatureChange(Bar* bar)
{
    return lastKeySignatureChange(part()->sheet()->indexOfBar(bar));
}

TimeSignature* Staff::lastTimeSignatureChange(int bar)
{
    if (!part()) return NULL;
    
    for (int b = bar; b >= 0; b--) {
        Bar* curBar = part()->sheet()->bar(b);
        for (int i = curBar->staffElementCount(this)-1; i >= 0; i--) {
            StaffElement* e = curBar->staffElement(this, i);
            TimeSignature* ts = dynamic_cast<TimeSignature*>(e);
            if (ts) return ts;
        }
    }
    return 0;
}

TimeSignature* Staff::lastTimeSignatureChange(Bar* bar)
{
    return lastTimeSignatureChange(part()->sheet()->indexOfBar(bar));
}

} // namespace MusicCore

#include "Staff.moc"
