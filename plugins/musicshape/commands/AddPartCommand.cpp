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
#include "AddPartCommand.h"

#include "../core/Sheet.h"
#include "../core/Part.h"
#include "../core/Clef.h"
#include "../core/Bar.h"
#include "../core/Staff.h"
#include "../core/TimeSignature.h"

#include "../MusicShape.h"

#include <klocale.h>

using namespace MusicCore;

AddPartCommand::AddPartCommand(MusicShape* shape)
    : m_sheet(shape->sheet()),
    m_shape(shape)
{
    setText(i18nc("(qtundo-format)", "Add part"));
    m_part = new Part(m_sheet, QString("Part %1").arg(m_sheet->partCount() + 1));
    Staff* s = m_part->addStaff();
    m_part->sheet()->bar(0)->addStaffElement(new Clef(s, 0, Clef::GClef, 2));
    // figure out time signature
    if (m_sheet->partCount() == 0) {
        m_part->sheet()->bar(0)->addStaffElement(new TimeSignature(s, 0, 4, 4));
    } else {
        Staff* curStaff = m_sheet->part(0)->staff(0);
        TimeSignature* ts = curStaff->lastTimeSignatureChange(0);
        if (!ts) {
            m_part->sheet()->bar(0)->addStaffElement(new TimeSignature(s, 0, 4, 4));
        } else {
            m_part->sheet()->bar(0)->addStaffElement(new TimeSignature(s, 0, ts->beats(), ts->beat(), ts->type()));
        }
    }
}

void AddPartCommand::redo()
{
    m_sheet->addPart(m_part);
    m_sheet->setStaffSystemCount(0);
    m_shape->engrave();
    m_shape->update();
}

void AddPartCommand::undo()
{
    m_sheet->removePart(m_part, false);
    m_sheet->setStaffSystemCount(0);
    m_shape->engrave();
    m_shape->update();
}
