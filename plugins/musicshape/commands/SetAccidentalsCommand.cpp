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
#include "SetAccidentalsCommand.h"

#include "../MusicShape.h"

#include "../core/Note.h"
#include "../core/Chord.h"
#include "../core/VoiceBar.h"

#include <klocalizedstring.h>

SetAccidentalsCommand::SetAccidentalsCommand(MusicShape* shape, MusicCore::Note* note, int accidentals)
    : m_shape(shape), m_note(note), m_oldAccidentals(note->accidentals()), m_newAccidentals(accidentals)
{
    setText(kundo2_i18n("Set accidentals"));
}

void SetAccidentalsCommand::redo()
{
    m_note->setAccidentals(m_newAccidentals);
    m_note->chord()->voiceBar()->updateAccidentals();
    m_shape->engrave();
    m_shape->update();
}

void SetAccidentalsCommand::undo()
{
    m_note->setAccidentals(m_oldAccidentals);
    m_note->chord()->voiceBar()->updateAccidentals();
    m_shape->engrave();
    m_shape->update();
}
