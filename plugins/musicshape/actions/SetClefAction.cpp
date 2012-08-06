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
#include "SetClefAction.h"

#include "../core/Staff.h"
#include "../core/Part.h"
#include "../core/Bar.h"
#include "../core/Sheet.h"

#include "../commands/SetClefCommand.h"

#include "../SimpleEntryTool.h"
#include "../MusicShape.h"

#include <KoIcon.h>

#include <kdebug.h>
#include <klocale.h>

using namespace MusicCore;

static KIcon getIcon(Clef::ClefShape shape)
{
    const char *const id =
        (shape == Clef::GClef) ? koIconNameCStr("music-clef-trebble") :
        (shape == Clef::FClef) ? koIconNameCStr("music-clef-bass") :
        (shape == Clef::CClef) ? koIconNameCStr("music-clef-alto") :
        /* else */               koIconNameCStr("music-clef");

    return KIcon(QLatin1String(id));
}

static QString getText(Clef::ClefShape shape, int line)
{
    switch (shape) {
        case Clef::GClef: return i18nc("Treble clef", "Treble");
        case Clef::FClef: return i18nc("Bass clef", "Bass");
        case Clef::CClef:
            switch (line) {
                case 1: return i18nc("Soprano clef", "Soprano");
                case 3: return i18nc("Alto clef", "Alto");
                case 4: return i18nc("Tenor clef", "Tenor");
                default: return i18n("C clef on line %1", line);
            }
    }
    return i18n("Unknown clef");
}

SetClefAction::SetClefAction(Clef::ClefShape shape, int line, int octaveChange, SimpleEntryTool* tool)
    : AbstractMusicAction(getIcon(shape), getText(shape, line), tool), m_shape(shape), m_line(line), m_octaveChange(octaveChange)
{
    setCheckable(false);
}

void SetClefAction::mousePress(Staff* staff, int barIdx, const QPointF& pos)
{
    Q_UNUSED( pos );
    
    Bar* bar = staff->part()->sheet()->bar(barIdx);
    m_tool->addCommand(new SetClefCommand(m_tool->shape(), bar, staff, m_shape, m_line, m_octaveChange));
}
