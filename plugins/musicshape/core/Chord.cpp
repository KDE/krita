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
#include "Chord.h"
#include "Note.h"
#include "Staff.h"
#include "VoiceBar.h"
#include "Clef.h"
#include "Bar.h"
#include "Sheet.h"

#include <QtCore/QList>

#include <climits>

namespace MusicCore {

namespace {
    struct Beam {
        Beam(Chord* chord) : beamStart(chord), beamEnd(chord), beamType(BeamFlag) {}
        Chord* beamStart;
        Chord* beamEnd;
        BeamType beamType;
    };
}

class Chord::Private {
public:
    Duration duration;
    int dots;
    QList<Note*> notes;
    StemDirection stemDirection;
    qreal stemLength;
    QList<Beam> beams;
};

static qreal calcStemLength(Duration duration)
{
    switch (duration) {
        case BreveNote:
        case WholeNote:
            return 0;
        case HalfNote:
        case QuarterNote:
        case EighthNote:
            return 3.5;
        case SixteenthNote:
            return 4;
        case ThirtySecondNote:
            return 4.75;
        case SixtyFourthNote:
            return 5.5;
        case HundredTwentyEighthNote:
            return 6.25;
    }
    return 0;
}

Chord::Chord(Duration duration, int dots) : VoiceElement(), d(new Private)
{
    d->duration = duration;
    d->dots = dots;
    d->stemLength = calcStemLength(duration);
    d->stemDirection = StemUp;

    int baseLength = durationToTicks(duration);
    int length = baseLength;
    for (int i = 0; i < dots; i++) {
        length += baseLength >> (i+1);
    }
    setLength(length);
}

Chord::Chord(Staff* staff, Duration duration, int dots) : d(new Private)
{
    d->duration = duration;
    d->dots = dots;
    d->stemLength = calcStemLength(duration);
    d->stemDirection = StemUp;

    int baseLength = durationToTicks(duration);
    int length = baseLength;
    for (int i = 0; i < dots; i++) {
        length += baseLength >> (i+1);
    }
    setLength(length);
    setStaff(staff);
}

Chord::~Chord()
{
    delete d;
}

qreal Chord::width() const
{
    qreal w = 7;

    int lastPitch = INT_MIN;
    bool hasConflict = false;
    bool haveAccidentals = false;

    foreach (Note* n, d->notes) {
        int pitch = n->pitch();
        if (pitch == lastPitch+1) {
            hasConflict = true;
        }
        lastPitch = pitch;

        if (n->drawAccidentals()) {
            haveAccidentals = true;
        }
    }

    if (hasConflict) w += 6;

    if (d->dots) {
        w += 2 + 3*d->dots;
    }

    if (haveAccidentals) {
        w += 10;
    }

    return w;
}

qreal Chord::beatline() const
{
    qreal bl = 0;

    int lastPitch = INT_MIN;
    bool hasConflict = false;
    bool haveAccidentals = false;

    foreach (Note* n, d->notes) {
        int pitch = n->pitch();
        if (pitch == lastPitch+1) {
            hasConflict = true;
        }
        lastPitch = pitch;

        if (n->drawAccidentals()) {
            haveAccidentals = true;
        }
    }

    if (hasConflict) bl += 6;
    if (haveAccidentals) {
        bl += 10;
    }

    return bl;
}

Duration Chord::duration() const
{
    return d->duration;
}

void Chord::setDuration(Duration duration)
{
    if (d->duration == duration) return;
    d->duration = duration;
    d->stemLength = calcStemLength(duration);
    int baseLength = durationToTicks(d->duration);
    int length = baseLength;
    for (int i = 0; i < d->dots; i++) {
        length += baseLength >> (i+1);
    }
    setLength(length);
    emit durationChanged(duration);
}

int Chord::dots() const
{
    return d->dots;
}

void Chord::setDots(int dots)
{
    if (d->dots == dots) return;
    d->dots = dots;
    int baseLength = durationToTicks(d->duration);
    int length = baseLength;
    for (int i = 0; i < dots; i++) {
        length += baseLength >> (i+1);
    }
    setLength(length);
    emit dotsChanged(dots);
}

int Chord::noteCount() const
{
    return d->notes.size();
}

Note* Chord::note(int index) const
{
    Q_ASSERT( index >= 0 && index < noteCount() );
    return d->notes[index];
}

Note* Chord::addNote(Staff* staff, int pitch, int accidentals)
{
    Note *n = new Note(this, staff, pitch, accidentals);
    addNote(n);
    return n;
}

void Chord::addNote(Note* note)
{
    Q_ASSERT( note );
    note->setParent(this);
    if (!staff()) setStaff(note->staff());
    for (int i = 0; i < d->notes.size(); i++) {
        if (d->notes[i]->pitch() > note->pitch()) {
            d->notes.insert(i, note);
            return;
        }
    }
    d->notes.append(note);
}

void Chord::removeNote(int index, bool deleteNote)
{
    Q_ASSERT( index >= 0 && index < noteCount() );
    Note* n = d->notes.takeAt(index);
    if (deleteNote) {
        delete n;
    }
}

void Chord::removeNote(Note* note, bool deleteNote)
{
    Q_ASSERT( note );
    int index = d->notes.indexOf(note);
    Q_ASSERT( index != -1 );
    removeNote(index, deleteNote);
}

qreal Chord::y() const
{
    if (d->notes.size() == 0) {
        return staff()->lineSpacing();
    }

    qreal top = 1e9;
    Clef* clef = staff()->lastClefChange(voiceBar()->bar(), 0);

    foreach (Note* n, d->notes) {
        int line = 10;
        if (clef) line = clef->pitchToLine(n->pitch());

        Staff* s = n->staff();
        line--;
        qreal y = s->top() + line * s->lineSpacing() / 2;
        if (y < top) top = y;
    }
    if (staff()) top -= staff()->top();
    return top;
}

qreal Chord::height() const
{
    if (d->notes.size() == 0) {
        return staff()->lineSpacing() * 2;
    }

    qreal top = 1e9;
    qreal bottom = -1e9;
    Clef* clef = staff()->lastClefChange(voiceBar()->bar(), 0);

    foreach (Note* n, d->notes) {
        int line = 10;
        if (clef) line = clef->pitchToLine(n->pitch());

        Staff* s = n->staff();
        line--;
        qreal y = s->top() + line * s->lineSpacing() / 2;
        if (y < top) top = y;
        line += 2;
        y = s->top() + line * s->lineSpacing() / 2;
        if (y > bottom) bottom = y;
    }
    if (staff()) {
        top -= staff()->top();
        bottom -= staff()->top();
    }
    return bottom - top;
}

qreal Chord::stemX() const
{
    int lastPitch = INT_MIN;
    bool hasConflict = false;
    bool haveAccidentals = false;

    foreach (Note* n, d->notes) {
        int pitch = n->pitch();
        if (pitch == lastPitch+1) {
            hasConflict = true;
        }
        lastPitch = pitch;

        if (n->drawAccidentals()) {
            haveAccidentals = true;
        }
    }

    if (hasConflict) {
        return x() + 6 + (haveAccidentals ? 10 : 0);
    } else {
        return x() + (d->stemDirection == StemUp ? 6 : 0) + (haveAccidentals ? 10 : 0);
    }
}

qreal Chord::centerX() const
{
    return x() + 3;
}

qreal Chord::topNoteY() const
{
    if (d->notes.size() == 0) {
        return staff()->lineSpacing() * 2 + staff()->top();
    }

    qreal top = 1e9;
    Clef* clef = staff()->lastClefChange(voiceBar()->bar(), 0);

    foreach (Note* n, d->notes) {
        int line = 10;
        if (clef) line = clef->pitchToLine(n->pitch());

        Staff* s = n->staff();
        qreal y = s->top() + line * s->lineSpacing() / 2;
        if (y < top) top = y;
    }
    return top;
}

qreal Chord::bottomNoteY() const
{
    if (d->notes.size() == 0) {
        return staff()->lineSpacing() * 2 + staff()->top();
    }

    qreal bottom = -1e9;
    Clef* clef = staff()->lastClefChange(voiceBar()->bar(), 0);

    foreach (Note* n, d->notes) {
        int line = 10;
        if (clef) line = clef->pitchToLine(n->pitch());

        Staff* s = n->staff();
        qreal y = s->top() + line * s->lineSpacing() / 2;
        if (y > bottom) bottom = y;
    }
    return bottom;
}

qreal Chord::stemEndY(bool interpolateBeams) const
{
    if (d->notes.size() == 0) return staff()->center();

    if (beamType(0) == BeamContinue && interpolateBeams) {
        // in the middle of a beam, interpolate stem length from beam
        qreal sx = beamStart(0)->stemX(), ex = beamEnd(0)->stemX();
        qreal sy = beamStart(0)->stemEndY(), ey = beamEnd(0)->stemEndY();
        qreal dydx = (ey-sy) / (ex-sx);

        return (stemX() - sx) * dydx + sy;
    }

    Staff* topStaff = NULL;
    Staff* bottomStaff = NULL;
    qreal top = 1e9, bottom = -1e9;
    Clef* clef = staff()->lastClefChange(voiceBar()->bar(), 0);

    foreach (Note* n, d->notes) {
        int line = 10;
        if (clef) line = clef->pitchToLine(n->pitch());

        Staff* s = n->staff();
        qreal y = s->top() + line * s->lineSpacing() / 2;
        if (y > bottom) {
            bottom = y;
            bottomStaff = s;
        }
        if (y < top) {
            top = y;
            topStaff = s;
        }
    }

    Q_ASSERT( topStaff );
    Q_ASSERT( bottomStaff );

    if (stemDirection() == StemUp) {
        qreal pos = top - topStaff->lineSpacing() * stemLength();
        if (pos > topStaff->center() && beamType(0) == BeamFlag) {
            pos = topStaff->center();
        }
        return pos;
    } else {
        qreal pos = bottom + bottomStaff->lineSpacing() * stemLength();
        if (pos < bottomStaff->center() && beamType(0) == BeamFlag) {
            pos = bottomStaff->center();
        }
        return pos;
    }
}

qreal Chord::beamDirection() const
{
    if (beamType(0) == BeamStart || beamType(0) == BeamEnd || beamType(0) == BeamContinue) {
        qreal sx = beamStart(0)->stemX(), ex = beamEnd(0)->stemX();
        qreal sy = beamStart(0)->stemEndY(), ey = beamEnd(0)->stemEndY();
        qreal dydx = (ey-sy) / (ex-sx);
        return dydx;
    } else {
        return 0;
    }
}

StemDirection Chord::stemDirection() const
{
    return d->stemDirection;
}

void Chord::setStemDirection(StemDirection direction)
{
    d->stemDirection = direction;
}

StemDirection Chord::desiredStemDirection() const
{
    VoiceBar* vb = voiceBar();
    Bar* bar = vb->bar();
    int barIdx = bar->sheet()->indexOfBar(bar);

    int topLine = 0, bottomLine = 0;
    qreal topy = 1e9, bottomy = -1e9;
    for (int n = 0; n < noteCount(); n++) {
        Note* note = this->note(n);
        Staff * s = note->staff();
        Clef* clef = s->lastClefChange(barIdx);
        int line = clef->pitchToLine(note->pitch());
        qreal ypos = s->top() + line * s->lineSpacing() / 2;
        if (ypos < topy) {
            topy = ypos;
            topLine = line;
        }
        if (ypos > bottomy) {
            bottomy = ypos;
            bottomLine = line;
        }
    }
    qreal center = (bottomLine + topLine) * 0.5;
    return (center < 4 ? StemDown : StemUp);
}

qreal Chord::stemLength() const
{
    return d->stemLength;
}

void Chord::setStemLength(qreal stemLength)
{
    d->stemLength = stemLength;
}

qreal Chord::desiredStemLength() const
{
    return calcStemLength(d->duration);
}

int Chord::beamCount() const
{
    switch (d->duration) {
        case HundredTwentyEighthNote:   return 5;
        case SixtyFourthNote:           return 4;
        case ThirtySecondNote:          return 3;
        case SixteenthNote:             return 2;
        case EighthNote:                return 1;
        default:                        return 0;
    }
}

const Chord* Chord::beamStart(int index) const
{
    if (d->beams.size() <= index) return this;
    return d->beams[index].beamStart;
}

const Chord* Chord::beamEnd(int index) const
{
    if (d->beams.size() <= index) return this;
    return d->beams[index].beamEnd;
}

Chord* Chord::beamStart(int index)
{
    if (d->beams.size() <= index) return this;
    return d->beams[index].beamStart;
}

Chord* Chord::beamEnd(int index)
{
    if (d->beams.size() <= index) return this;
    return d->beams[index].beamEnd;
}

BeamType Chord::beamType(int index) const
{
    if (d->beams.size() <= index) return BeamFlag;
    return d->beams[index].beamType;
}

void Chord::setBeam(int index, Chord* beamStart, Chord* beamEnd, BeamType type)
{
    Q_ASSERT( index < beamCount() );
    while (d->beams.size() <= index) {
        d->beams.append(Beam(this));
    }
    d->beams[index].beamStart = beamStart;
    d->beams[index].beamEnd = beamEnd;
    if (beamStart == this && beamEnd == this) {
        if (type != BeamFlag && type != BeamForwardHook && type != BeamBackwardHook) type = BeamFlag;
        d->beams[index].beamType = type;
    } else if (beamStart == this) d->beams[index].beamType = BeamStart;
    else if (beamEnd == this) d->beams[index].beamType = BeamEnd;
    else d->beams[index].beamType = BeamContinue;
}

} // namespace MusicCore

#include <Chord.moc>
