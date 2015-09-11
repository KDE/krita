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
#include "AddBarsCommand.h"
#include "../core/Sheet.h"
#include "../MusicShape.h"

#include <klocalizedstring.h>

using namespace MusicCore;

AddBarsCommand::AddBarsCommand(MusicShape* shape, int bars)
    : m_sheet(shape->sheet()), m_bars(bars), m_shape(shape)
{
    setText(kundo2_i18n("Add measures"));
}

void AddBarsCommand::redo()
{
    m_sheet->addBars(m_bars);
    m_shape->engrave();
    m_shape->update();
}

void AddBarsCommand::undo()
{
    m_sheet->removeBars(m_sheet->barCount() - m_bars, m_bars);
    m_shape->engrave();
    m_shape->update();
}
