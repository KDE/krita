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
#include "CreateChordCommand.h"

#include "../core/VoiceBar.h"
#include "../core/Chord.h"
#include "../MusicShape.h"

#include <klocalizedstring.h>

using namespace MusicCore;


CreateChordCommand::CreateChordCommand(MusicShape* shape, VoiceBar* voiceBar, Staff* staff, Duration duration, int before, int pitch, int accidentals)
    : m_shape(shape), m_voiceBar(voiceBar), m_before(before)
{
    setText(kundo2_i18n("Add chord"));
    m_chord = new Chord(staff, duration);
    m_chord->addNote(staff, pitch, accidentals);
}

CreateChordCommand::CreateChordCommand(MusicShape* shape, VoiceBar* voiceBar, Staff* staff, Duration duration, int before)
    : m_shape(shape), m_voiceBar(voiceBar), m_before(before)
{
    setText(kundo2_i18n("Add rest"));
    m_chord = new Chord(staff, duration);
}

void CreateChordCommand::redo()
{
    m_voiceBar->insertElement(m_chord, m_before);
    m_shape->engrave();
    m_shape->update();

}

void CreateChordCommand::undo()
{
    m_voiceBar->removeElement(m_chord, false);
    m_shape->engrave();
    m_shape->update();
}
