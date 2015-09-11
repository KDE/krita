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
#include "ChangePartDetailsCommand.h"
#include <klocalizedstring.h>
#include "../core/Part.h"
#include "../core/Staff.h"
#include "../core/Clef.h"
#include "../core/Bar.h"
#include "../core/Sheet.h"
#include "../core/VoiceBar.h"
#include "../core/VoiceElement.h"
#include "../core/Chord.h"
#include "../core/Note.h"
#include "../core/TimeSignature.h"

#include "../MusicShape.h"

using namespace MusicCore;

typedef QPair<VoiceElement*, Staff*> VoiceElementStaffPair;
typedef QPair<Note*, Staff*> NoteStaffPair;

ChangePartDetailsCommand::ChangePartDetailsCommand(MusicShape* shape, Part* part, const QString& name, const QString& abbr, int staffCount)
    : m_shape(shape), m_part(part), m_oldName(part->name()), m_newName(name), m_oldAbbr(part->shortName(false))
    , m_newAbbr(abbr), m_oldStaffCount(part->staffCount()), m_newStaffCount(staffCount)
{
    setText(kundo2_i18n("Change part details"));
    
    if (m_newStaffCount > m_oldStaffCount) {
        TimeSignature* ts = m_part->staff(0)->lastTimeSignatureChange(0);
        for (int i = 0; i < m_newStaffCount - m_oldStaffCount; i++) {
            Staff* s = new Staff(m_part);
            m_part->sheet()->bar(0)->addStaffElement(new Clef(s, 0, Clef::GClef, 2));
            if (ts) {
                m_part->sheet()->bar(0)->addStaffElement(new TimeSignature(s, 0, ts->beats(), ts->beat(), ts->type()));
            } else {
                m_part->sheet()->bar(0)->addStaffElement(new TimeSignature(s, 0, 4, 4));
            }
            m_staves.append(s);
        }
    } else if (m_newStaffCount < m_oldStaffCount) {
        for (int i = m_newStaffCount; i < m_oldStaffCount; i++) {
            m_staves.append(m_part->staff(i));
        }
        // now collect all elements that exist in one of the staves that is removed
        Sheet* sheet = part->sheet();
        for (int v = 0; v < part->voiceCount(); v++) {
            Voice* voice = part->voice(v);
            for (int b = 0; b < sheet->barCount(); b++) {
                VoiceBar* vb = sheet->bar(b)->voice(voice);
                for (int e = 0; e < vb->elementCount(); e++) {
                    VoiceElement* ve = vb->element(e);
                    int sid = part->indexOfStaff(ve->staff());
                    if (sid >= m_newStaffCount) {
                        m_elements.append(qMakePair(ve, ve->staff()));
                    }
                    Chord* c = dynamic_cast<Chord*>(ve);
                    if (c) {
                        for (int n = 0; n < c->noteCount(); n++) {
                            Note* note = c->note(n);
                            int sid = part->indexOfStaff(note->staff());
                            if (sid >= m_newStaffCount) {
                                m_notes.append(qMakePair(note, note->staff()));
                            }
                        }
                    }
                }
            }
        }
    }
}

void ChangePartDetailsCommand::redo()
{
    m_part->setName(m_newName);
    m_part->setShortName(m_newAbbr);
    if (m_newStaffCount > m_oldStaffCount) {
        foreach (Staff* s, m_staves) {
            m_part->addStaff(s);
        }
    } else if (m_newStaffCount < m_oldStaffCount) {
        foreach (Staff* s, m_staves) {
            m_part->removeStaff(s, false);
        }
        Staff* s = m_part->staff(m_newStaffCount - 1);
        foreach (const VoiceElementStaffPair & p, m_elements) {
            p.first->setStaff(s);
        }
        foreach (const NoteStaffPair & p, m_notes) {
            p.first->setStaff(s);
        }
    }
    if (m_newStaffCount != m_oldStaffCount) {
        m_shape->sheet()->setStaffSystemCount(0);
        m_shape->engrave();
        m_shape->update();
    }        
}

void ChangePartDetailsCommand::undo()
{
    m_part->setName(m_oldName);
    m_part->setShortName(m_oldAbbr);
    if (m_oldStaffCount > m_newStaffCount) {
        foreach (Staff* s, m_staves) {
            m_part->addStaff(s);
        }
        foreach (const VoiceElementStaffPair & p, m_elements) {
            p.first->setStaff(p.second);
        }
        foreach (const NoteStaffPair & p, m_notes) {
            p.first->setStaff(p.second);
        }        
    } else if (m_oldStaffCount < m_newStaffCount) {
        foreach (Staff* s, m_staves) {
            m_part->removeStaff(s, false);
        }
    }
    if (m_newStaffCount != m_oldStaffCount) {
        m_shape->sheet()->setStaffSystemCount(0);
        m_shape->engrave();
        m_shape->update();
    }        
}
