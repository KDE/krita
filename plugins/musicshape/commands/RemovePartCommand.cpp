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
#include "RemovePartCommand.h"
#include "../core/Sheet.h"
#include "../core/Part.h"
#include "../MusicShape.h"

#include "klocalizedstring.h"

using namespace MusicCore;

RemovePartCommand::RemovePartCommand(MusicShape* shape, Part* part)
    : m_sheet(part->sheet()),
    m_part(part),
    m_shape(shape),
    m_partIndex(m_sheet->partIndex(part))
{
    setText(kundo2_i18n("Remove part"));
}

void RemovePartCommand::redo()
{
    m_sheet->removePart(m_part, false);
    m_sheet->setStaffSystemCount(0);
    m_shape->engrave();
    m_shape->update();
}

void RemovePartCommand::undo()
{
    m_sheet->insertPart(m_partIndex, m_part);
    m_sheet->setStaffSystemCount(0);
    m_shape->engrave();
    m_shape->update();
}
