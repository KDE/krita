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
#include "SetClefCommand.h"

#include "../core/Bar.h"

#include "../MusicShape.h"

#include <klocale.h>

using namespace MusicCore;

SetClefCommand::SetClefCommand(MusicShape* shape, Bar* bar, Staff* staff, Clef::ClefShape clefShape, int line, int octaveChange)
    : m_shape(shape), m_bar(bar), m_clef(new Clef(staff, 0, clefShape, line, octaveChange)), m_oldClef(NULL)
{
    setText(i18nc("(qtundo-format)", "Change clef"));
    
    for (int i = 0; i < bar->staffElementCount(staff); i++) {
        Clef* c = dynamic_cast<Clef*>(bar->staffElement(staff, i));
        if (c && c->startTime() == 0) {
            m_oldClef = c;
            break;
        }
    }
}

void SetClefCommand::redo()
{
    if (m_oldClef) m_bar->removeStaffElement(m_oldClef, false);
    m_bar->addStaffElement(m_clef);
    m_shape->engrave();
    m_shape->update();
}

void SetClefCommand::undo()
{
    m_bar->removeStaffElement(m_clef, false);
    if (m_oldClef) m_bar->addStaffElement(m_oldClef);
    m_shape->engrave();
    m_shape->update();
}
