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
#ifndef CREATECHORDCOMMAND_H
#define CREATECHORDCOMMAND_H

#include <kundo2command.h>

#include "../core/Global.h"

namespace MusicCore {
    class Staff;
    class VoiceBar;
    class Chord;
}
class MusicShape;

class CreateChordCommand : public KUndo2Command {
public:
    CreateChordCommand(MusicShape* shape, MusicCore::VoiceBar* voiceBar, MusicCore::Staff* staff, MusicCore::Duration duration, int before, int pitch, int accidentals);
    CreateChordCommand(MusicShape* shape, MusicCore::VoiceBar* voiceBar, MusicCore::Staff* staff, MusicCore::Duration duration, int before);
    virtual void redo();
    virtual void undo();
private:
    MusicShape* m_shape;
    MusicCore::VoiceBar* m_voiceBar;
    int m_before;
    MusicCore::Chord* m_chord;
};

#endif // CREATECHORDCOMMAND_H
