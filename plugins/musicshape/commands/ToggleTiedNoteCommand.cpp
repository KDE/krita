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
#include "ToggleTiedNoteCommand.h"

#include "../MusicShape.h"

#include "../core/Note.h"
#include "../core/Chord.h"
#include "../core/VoiceBar.h"

#include <klocalizedstring.h>

ToggleTiedNoteCommand::ToggleTiedNoteCommand(MusicShape* shape, MusicCore::Note* note)
    : m_shape(shape), m_note(note)
{
    setText(kundo2_i18n("Toggle Note Tie"));
}

void ToggleTiedNoteCommand::redo()
{
    m_note->setStartTie(!m_note->isStartTie());
    m_note->chord()->voiceBar()->updateAccidentals();
    m_shape->update();
}

void ToggleTiedNoteCommand::undo()
{
    m_note->setStartTie(!m_note->isStartTie());
    m_note->chord()->voiceBar()->updateAccidentals();
    m_shape->update();
}
