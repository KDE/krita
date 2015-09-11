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
#include "AddNoteCommand.h"

#include "../core/Note.h"
#include "../core/Chord.h"
#include "../core/VoiceBar.h"
#include "../MusicShape.h"

#include <klocalizedstring.h>

using namespace MusicCore;

AddNoteCommand::AddNoteCommand(MusicShape* shape, Chord* chord, Staff* staff, Duration duration, int pitch, int accidentals)
    : m_shape(shape), m_chord(chord), m_oldDuration(chord->duration()), m_newDuration(duration), m_oldDots(chord->dots()), m_note(0)
{
    bool exists = false;
    for (int i = 0; i < m_chord->noteCount(); i++) {
        Note* n = m_chord->note(i);
        if (n->staff() == staff && n->pitch() == pitch) {
            exists = true;
            break;
        }
    }
    if (exists) {
        setText(kundo2_i18n("Set chord duration"));
    } else {
        setText(kundo2_i18n("Add note"));
        m_note = new Note(m_chord, staff, pitch, accidentals);
    }
}

void AddNoteCommand::redo()
{
    m_chord->setDuration(m_newDuration);
    m_chord->setDots(0);
    if (m_note) {
        m_chord->addNote(m_note);
    }
    m_chord->voiceBar()->updateAccidentals();
    m_shape->engrave();
    m_shape->update();

}

void AddNoteCommand::undo()
{
    m_chord->setDuration(m_oldDuration);
    m_chord->setDots(m_oldDots);
    if (m_note) {
        m_chord->removeNote(m_note, false);
    }
    m_chord->voiceBar()->updateAccidentals();
    m_shape->engrave();
    m_shape->update();
}
