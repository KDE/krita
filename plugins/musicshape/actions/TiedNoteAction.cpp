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
#include "TiedNoteAction.h"

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

#include "../commands/ToggleTiedNoteCommand.h"

#include <KoIcon.h>

#include <klocalizedstring.h>

#include <math.h>

using namespace MusicCore;

TiedNoteAction::TiedNoteAction(SimpleEntryTool* tool)
    : AbstractNoteMusicAction(koIcon("music-tiednote"), i18n("Tied notes"), tool)
{
}

void TiedNoteAction::mousePress(Chord* chord, Note* note, qreal distance, const QPointF& pos)
{
    Q_UNUSED( chord );
    Q_UNUSED( pos );
    
    if (!note) return;
    if (distance > 15) return; // bah, magic numbers are ugly....
    
    m_tool->addCommand(new ToggleTiedNoteCommand(m_tool->shape(), note));
}
