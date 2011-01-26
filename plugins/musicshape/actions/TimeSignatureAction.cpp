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
#include "TimeSignatureAction.h"

#include "../core/Bar.h"
#include "../core/Staff.h"
#include "../core/Part.h"
#include "../core/Sheet.h"
#include "../core/TimeSignature.h"

#include "../commands/SetTimeSignatureCommand.h"

#include "../SimpleEntryTool.h"
#include "../MusicShape.h"

using namespace MusicCore;

static QString getText(int beats, int beat)
{
    return QString("%1/%2").arg(beats).arg(beat);
}

TimeSignatureAction::TimeSignatureAction(SimpleEntryTool* tool, int beats, int beat)
    : AbstractMusicAction(getText(beats, beat), tool), m_beats(beats), m_beat(beat)
{
    setCheckable(false);
}

void TimeSignatureAction::mousePress(Staff* staff, int barIdx, const QPointF& pos)
{
    Q_UNUSED( pos );
    
    Bar* bar = staff->part()->sheet()->bar(barIdx);
    m_tool->addCommand(new SetTimeSignatureCommand(m_tool->shape(), bar, m_beats, m_beat));
}
