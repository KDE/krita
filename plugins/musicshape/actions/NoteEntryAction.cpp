/* This file is part of the KDE project
 * Copyright 2007 Marijn Kruisselbrink <m.Kruisselbrink@student.tue.nl>
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
#include "NoteEntryAction.h"

#include "../SimpleEntryTool.h"
#include "../MusicShape.h"
#include "../Renderer.h"
#include "../MusicCursor.h"

#include "../core/Staff.h"
#include "../core/Clef.h"
#include "../core/Voice.h"
#include "../core/Part.h"
#include "../core/VoiceBar.h"
#include "../core/Sheet.h"
#include "../core/Bar.h"
#include "../core/KeySignature.h"
#include "../core/Note.h"

#include "../commands/CreateChordCommand.h"
#include "../commands/AddNoteCommand.h"
#include "../commands/MakeRestCommand.h"

#include <kicon.h>
#include <kdebug.h>
#include <klocale.h>

#include <QKeyEvent>

using namespace MusicCore;

static KIcon getIcon(Duration duration, bool isRest)
{
    QString base = isRest ? "music-rest-" : "music-note-";
    switch (duration) {
        case BreveNote:          return KIcon(base + "breve");
        case WholeNote:          return KIcon(base + "whole");
        case HalfNote:           return KIcon(base + "half");
        case QuarterNote:        return KIcon(base + "quarter");
        case EighthNote:         return KIcon(base + "eighth");
        case SixteenthNote:      return KIcon(base + "16th");
        case ThirtySecondNote:   return KIcon(base + "32nd");
        case SixtyFourthNote:    return KIcon(base + "64th");
        case HundredTwentyEighthNote: return KIcon(base + "128th");
    }
    return KIcon();
}

static QString getText(Duration duration, bool isRest)
{
    QString base = isRest ? i18n("rest") : i18n("note");
    switch (duration) {
        case BreveNote:          return i18n("Double whole ") + base;
        case WholeNote:          return i18n("Whole ") + base;
        case HalfNote:           return i18n("Half ") + base;
        case QuarterNote:        return i18n("Quarter ") + base;
        case EighthNote:         return i18n("Eighth ") + base;
        case SixteenthNote:      return i18n("16th ") + base;
        case ThirtySecondNote:   return i18n("32nd ") + base;
        case SixtyFourthNote:    return i18n("64th ") + base;
        case HundredTwentyEighthNote: return i18n("128th ") + base;
    }
    return isRest ? i18n("Unknown rest") : i18n("Unknown note");
}

NoteEntryAction::NoteEntryAction(Duration duration, bool isRest, SimpleEntryTool* tool)
    : AbstractMusicAction(getIcon(duration, isRest), getText(duration, isRest), tool)
    , m_duration(duration), m_isRest(isRest)
{
    m_isVoiceAware = true;
}

void NoteEntryAction::renderPreview(QPainter& painter, const QPointF& point)
{
    if (!m_isRest) {
        qreal sl = 3.5;
        if (m_duration < SixteenthNote) sl += 1;
        if (m_duration < ThirtySecondNote) sl += 1;
        m_tool->shape()->renderer()->renderNote(painter, m_duration, point - QPointF(3, 0), sl * 5, Qt::gray);
    } else {
        m_tool->shape()->renderer()->renderRest(painter, m_duration, point, Qt::gray);
    }
}

void NoteEntryAction::mousePress(Staff* staff, int bar, const QPointF& pos)
{
    Clef* clef = staff->lastClefChange(bar);

    Voice* voice = staff->part()->voice(m_tool->voice());
    VoiceBar* vb = voice->bar(bar);

    // find element before which to insert the chord
    int before = 0;
    for (int i = 0; i < vb->elementCount(); i++) {
        VoiceElement* e = vb->element(i);
        if (e->x() >= pos.x()) break;
        before++;
    }

    int line = staff->line(pos.y());
    int pitch = 0, accidentals = 0;
    if (clef && !m_isRest) {
        pitch = clef->lineToPitch(line);
        // get correct accidentals for note
        KeySignature* ks = staff->lastKeySignatureChange(bar);
        if (ks) accidentals = ks->accidentals(pitch);
        for (int i = 0; i < before; i++) {
            Chord* c = dynamic_cast<Chord*>(vb->element(i));
            if (!c) continue;
            for (int n = 0; n < c->noteCount(); n++) {
                if (c->note(n)->pitch() == pitch) {
                    accidentals = c->note(n)->accidentals();
                }
            }
        }
    }
    
    Chord* join = NULL;
    if (before > 0) join = dynamic_cast<Chord*>(vb->element(before-1));
    if (join && join->x() + join->width() >= pos.x()) {
        if (clef && !m_isRest) {
            m_tool->addCommand(new AddNoteCommand(m_tool->shape(), join, staff, m_duration, pitch, accidentals));
        } else {
            m_tool->addCommand(new MakeRestCommand(m_tool->shape(), join));
        }
    } else {
        if (clef && !m_isRest) {
            m_tool->addCommand(new CreateChordCommand(m_tool->shape(), vb, staff, m_duration, before, pitch, accidentals));
        } else {
            m_tool->addCommand(new CreateChordCommand(m_tool->shape(), vb, staff, m_duration, before));
        }
    }
}

void NoteEntryAction::renderKeyboardPreview(QPainter& painter, const MusicCursor& cursor)
{
    Staff* staff = cursor.staff();
    Part* part = staff->part();
    Sheet* sheet = part->sheet();
    Bar* bar = sheet->bar(cursor.bar());
    QPointF p = bar->position() + QPointF(0, staff->top());
    Voice* voice = cursor.staff()->part()->voice(cursor.voice());
    VoiceBar* vb = voice->bar(bar);

    if (cursor.element() >= vb->elementCount()) {
        // cursor is past last element in bar, position of cursor is
        // halfway between last element and end of bar
        if (vb->elementCount() == 0) {
            // unless entire voicebar is still empty
            p.rx() += 15.0;
        } else {
            VoiceElement* ve = vb->element(vb->elementCount()-1);
            p.rx() += (ve->x() + bar->size()) / 2;
        }
    } else {
        // cursor is on an element, get the position of that element
        p.rx() += vb->element(cursor.element())->x();
    }

    p.ry() += (cursor.staff()->lineCount() - 1)* cursor.staff()->lineSpacing();
    p.ry() -= cursor.staff()->lineSpacing() * cursor.line() / 2;

    m_tool->shape()->renderer()->renderNote(painter, m_duration < QuarterNote ? QuarterNote : m_duration, p, 0, Qt::magenta);
}

void NoteEntryAction::keyPress(QKeyEvent* event, const MusicCursor& cursor)
{
    if (event->key() == Qt::Key_Enter || event->key() == Qt::Key_Return) {
        Staff* staff = cursor.staff();
        //Part* part = staff->part();
        //Sheet* sheet = part->sheet();
        //Bar* bar = sheet->bar(cursor.bar());
        Clef* clef = staff->lastClefChange(cursor.bar());
        int line = cursor.line();
        int pitch = 0, accidentals = 0;
        VoiceBar* vb = cursor.voiceBar();
        if (clef) {
            pitch = clef->lineToPitch(line);
            // get correct accidentals for note
            KeySignature* ks = staff->lastKeySignatureChange(cursor.bar());
            if (ks) accidentals = ks->accidentals(pitch);
            for (int i = 0; i < cursor.element(); i++) {
                Chord* c = dynamic_cast<Chord*>(vb->element(i));
                if (!c) continue;
                for (int n = 0; n < c->noteCount(); n++) {
                    if (c->note(n)->pitch() == pitch) {
                        accidentals = c->note(n)->accidentals();
                    }
                }
            }
        }

        Chord* join = 0;
        if (cursor.element() < vb->elementCount()) join = dynamic_cast<Chord*>(vb->element(cursor.element()));
        if (event->modifiers() & Qt::ShiftModifier || !join) {
            m_tool->addCommand(new CreateChordCommand(m_tool->shape(), vb, staff, m_duration, cursor.element(), pitch, accidentals));
        } else {
            m_tool->addCommand(new AddNoteCommand(m_tool->shape(), join, staff, join->duration(), pitch, accidentals));
        }
        event->accept();
    }
}
