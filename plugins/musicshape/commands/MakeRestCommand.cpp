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
#include "MakeRestCommand.h"
#include "../core/Chord.h"
#include "../core/Note.h"
#include "../core/VoiceBar.h"
#include "../MusicShape.h"

#include <klocalizedstring.h>

using namespace MusicCore;

MakeRestCommand::MakeRestCommand(MusicShape* shape, Chord* chord)
    : m_chord(chord), m_shape(shape)
{
    setText(kundo2_i18n("Convert chord to rest"));
    for (int i = 0; i < chord->noteCount(); i++) {
        m_notes.append(chord->note(i));
    }
}

void MakeRestCommand::redo()
{
    foreach (Note* n, m_notes) {
        m_chord->removeNote(n, false);
    }
    m_chord->voiceBar()->updateAccidentals();
    m_shape->engrave();
    m_shape->update();
}

void MakeRestCommand::undo()
{
    foreach (Note* n, m_notes) {
        m_chord->addNote(n);
    }
    m_chord->voiceBar()->updateAccidentals();
    m_shape->engrave();
    m_shape->update();
}
