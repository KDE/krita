/* This file is part of the KDE project
 * Copyright 2007 Marijn Kruisselbrink <mkruisselbrink@kde.org>
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
#include "AbstractNoteMusicAction.h"

#include "../SimpleEntryTool.h"
#include "../MusicShape.h"
#include "../Renderer.h"

#include "../core/Staff.h"
#include "../core/Part.h"
#include "../core/Sheet.h"
#include "../core/Bar.h"
#include "../core/VoiceBar.h"
#include "../core/Chord.h"
#include "../core/Note.h"
#include "../core/Voice.h"
#include "../core/Clef.h"

#include <math.h>

using namespace MusicCore;

AbstractNoteMusicAction::AbstractNoteMusicAction(const KIcon& icon, const QString& text, SimpleEntryTool* tool)
    : AbstractMusicAction(icon, text, tool)
{
}

AbstractNoteMusicAction::AbstractNoteMusicAction(const QString& text, SimpleEntryTool* tool)
    : AbstractMusicAction(text, tool)
{
}

static inline qreal sqr(qreal a)
{
    return a * a;
}

void AbstractNoteMusicAction::mousePress(Staff* staff, int barIdx, const QPointF& pos)
{
    Part* part = staff->part();
    Sheet* sheet = part->sheet();
    Bar* bar = sheet->bar(barIdx);
    
    Clef* clef = staff->lastClefChange(barIdx, 0);
    
    // loop over all noteheads
    qreal closestDist = 1e9;
    Note* closestNote = 0;
    Chord* chord = 0;
    
    // outer loop, loop over all voices
    for (int v = 0; v < part->voiceCount(); v++) {
        Voice* voice = part->voice(v);
        VoiceBar* vb = voice->bar(bar);
        
        // next loop over all chords
        for (int e = 0; e < vb->elementCount(); e++) {
            Chord* c = dynamic_cast<Chord*>(vb->element(e));
            if (!c) continue;
            
            qreal centerX = c->x() + (c->width() / 2);
            
            // check if it is a rest
            if (c->noteCount() == 0) {
                qreal centerY = c->y() + (c->height() / 2);
                qreal dist = sqrt(sqr(centerX - pos.x()) + sqr(centerY - pos.y()));
                if (dist < closestDist) {
                    closestDist = dist;
                    closestNote = NULL;
                    chord = c;
                }
            }
            
            // lastly loop over all noteheads
            for (int n = 0; n < c->noteCount(); n++) {
                Note* note = c->note(n);
                if (note->staff() != staff) continue;
                
                int line = clef->pitchToLine(note->pitch());
                qreal centerY = line * staff->lineSpacing() / 2;
                
                qreal dist = sqrt(sqr(centerX - pos.x()) + sqr(centerY - pos.y()));
                if (dist < closestDist) {
                    closestDist = dist;
                    closestNote = note;
                    chord = c;
                }
            }
        }
    }
    
    StaffElement* se = 0;
    for (int e = 0; e < bar->staffElementCount(staff); e++) {
        StaffElement* elem = bar->staffElement(staff, e);
        qreal centerX = elem->x() + (elem->width() / 2);
        qreal centerY = elem->y() + (elem->height() / 2);
        qreal dist = sqrt(sqr(centerX - pos.x()) + sqr(centerY - pos.y()));
        if (dist < closestDist) {
            se = elem;
            closestDist = dist;
        }
    }
    
    if (se) {
        mousePress(se, closestDist, pos);
    } else {
        mousePress(chord, closestNote, closestDist, pos);
    }
}

void AbstractNoteMusicAction::mousePress(StaffElement*, qreal, const QPointF&)
{
    // empty default implementation
}

void AbstractNoteMusicAction::mouseMove(Staff* staff, int barIdx, const QPointF& pos)
{
    Part* part = staff->part();
    Sheet* sheet = part->sheet();
    Bar* bar = sheet->bar(barIdx);
    
    Clef* clef = staff->lastClefChange(barIdx, 0);
    
    // loop over all noteheads
    qreal closestDist = 1e9;
    Note* closestNote = 0;
    Chord* chord = 0;
    
    // outer loop, loop over all voices
    for (int v = 0; v < part->voiceCount(); v++) {
        Voice* voice = part->voice(v);
        VoiceBar* vb = voice->bar(bar);
        
        // next loop over all chords
        for (int e = 0; e < vb->elementCount(); e++) {
            Chord* c = dynamic_cast<Chord*>(vb->element(e));
            if (!c) continue;
            
            qreal centerX = c->x() + (c->width() / 2);
            
            // check if it is a rest
            if (c->noteCount() == 0) {
                qreal centerY = c->y() + (c->height() / 2);
                qreal dist = sqrt(sqr(centerX - pos.x()) + sqr(centerY - pos.y()));
                if (dist < closestDist) {
                    closestDist = dist;
                    closestNote = NULL;
                    chord = c;
                }
            }
            
            // lastly loop over all noteheads
            for (int n = 0; n < c->noteCount(); n++) {
                Note* note = c->note(n);
                if (note->staff() != staff) continue;
                
                int line = clef->pitchToLine(note->pitch());
                qreal centerY = line * staff->lineSpacing() / 2;
                
                qreal dist = sqrt(sqr(centerX - pos.x()) + sqr(centerY - pos.y()));
                if (dist < closestDist) {
                    closestDist = dist;
                    closestNote = note;
                    chord = c;
                }
            }
        }
    }
        
    StaffElement* se = 0;
    for (int e = 0; e < bar->staffElementCount(staff); e++) {
        StaffElement* elem = bar->staffElement(staff, e);
        qreal centerX = elem->x() + (elem->width() / 2);
        qreal centerY = elem->y() + (elem->height() / 2);
        qreal dist = sqrt(sqr(centerX - pos.x()) + sqr(centerY - pos.y()));
        if (dist < closestDist) {
            se = elem;
            closestDist = dist;
        }
    }
    
    if (se) {
        mouseMove(se, closestDist, pos);
    } else {
        mouseMove(chord, closestNote, closestDist, pos);
    }
}

void AbstractNoteMusicAction::mouseMove(Chord*, Note*, qreal, const QPointF&)
{
    // empty default implementation
}

void AbstractNoteMusicAction::mouseMove(StaffElement*, qreal, const QPointF&)
{
    // empty default implementation
}
