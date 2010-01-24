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
#include "Chord.h"
#include "VoiceBar.h"
#include "Note.h"

#include <QMap>

#include <math.h>
#include <limits.h>

namespace MusicCore {

class Staff::Private
{
public:
    qreal spacing;
    int lineCount;
    qreal lineSpacing;
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

qreal Staff::spacing() const
{
    return d->spacing;
}

void Staff::setSpacing(qreal spacing)
{
    if (d->spacing == spacing) return;
    d->spacing = spacing;
    emit spacingChanged(spacing);
}

qreal Staff::top()
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

qreal Staff::bottom()
{
    return top() + lineSpacing() * (lineCount() - 1);
}

qreal Staff::center()
{
    return (top() + bottom()) / 2;
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

qreal Staff::lineSpacing() const
{
    return d->lineSpacing;
}

void Staff::setLineSpacing(qreal lineSpacing)
{
    if (d->lineSpacing == lineSpacing) return;
    d->lineSpacing = lineSpacing;
    emit lineSpacingChanged(lineSpacing);
}

int Staff::line(qreal y) const
{
    y = (lineCount()-1) * lineSpacing() - y;
    y /= lineSpacing() / 2;
    return qRound(y);
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

void Staff::updateAccidentals(int fromBar)
{
    KeySignature* ks = lastKeySignatureChange(fromBar);
    for (int barIdx = fromBar, barCount = part()->sheet()->barCount(); barIdx < barCount; barIdx++) {
        Bar* bar = part()->sheet()->bar(barIdx);
        
        // process key signature changes in this bar
        for (int i = 0; i < bar->staffElementCount(this); i++) {
            StaffElement* e = bar->staffElement(this, i);
            KeySignature* curks = dynamic_cast<KeySignature*>(e);
            if (curks) {
                ks = curks;
            }
        }
        
        // current accidentals for all notes with pitches from -40..40
        // curAccidentals[pitch+40]; value is #accidentals + 100 (so 0 means no notes seen yet, use the keysig)
        int curAccidentals[81] = {0};
        // and a map for all other pitches (there shouldn't be much anyway)
        QMap<int, int> accidentalsMap;
        
        // now loop over all notes in all voices in this bar, and check if they are in this staff
        // if they are, set accidentals visibility and update current accidentals
        for (int v = 0; v < part()->voiceCount(); v++) {
            Voice* voice = part()->voice(v);
            VoiceBar* vb = bar->voice(voice);
            for (int e = 0; e < vb->elementCount(); e++) {
                Chord* c = dynamic_cast<Chord*>(vb->element(e));
                if (!c) continue;
                for (int nid = 0; nid < c->noteCount(); nid++) {
                    Note* note = c->note(nid);
                    if (note->staff() != this) continue;
                    int pitch = note->pitch();
                    int cur = 0;
                    if (pitch >= -40 && pitch <= 40) {
                        cur = curAccidentals[pitch+40];
                        if (cur == 0 && ks) {
                            cur = ks->accidentals(pitch);
                        } else {
                            cur -= 100;
                        }
                        
                        curAccidentals[pitch+40] = note->accidentals() + 100;
                    } else {
                        if (accidentalsMap.contains(pitch)) {
                            cur = accidentalsMap[pitch];
                        } else if (ks) {
                            cur = ks->accidentals(pitch);
                        } else {
                            cur = 0;
                        }
                        
                        accidentalsMap[pitch] = note->accidentals();
                    }
                    
                    note->setDrawAccidentals(note->accidentals() != cur);
                }
            }
        }
    }
}

void Staff::updateAccidentals(Bar* fromBar)
{
    Q_ASSERT( fromBar );
    updateAccidentals( part()->sheet()->indexOfBar(fromBar) );
}

} // namespace MusicCore

#include <Staff.moc>
