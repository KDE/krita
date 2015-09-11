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
#include "RemoveBarCommand.h"
#include <klocalizedstring.h>
#include "../core/Bar.h"
#include "../core/Sheet.h"

#include "../MusicShape.h"

using namespace MusicCore;

RemoveBarCommand::RemoveBarCommand(MusicShape* shape, int barIdx)
    : m_shape(shape), m_bar(m_shape->sheet()->bar(barIdx)), m_index(barIdx)
{
    setText(kundo2_i18n("Remove bar"));
}

void RemoveBarCommand::redo()
{
    m_bar->sheet()->removeBar(m_index, false);
    m_shape->engrave();
    m_shape->update();
}

void RemoveBarCommand::undo()
{
    m_bar->sheet()->insertBar(m_index, m_bar);
    m_shape->engrave();
    m_shape->update();
}
