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
#include "RemoveStaffElementCommand.h"
#include <klocalizedstring.h>
#include "../core/StaffElement.h"
#include "../core/Clef.h"
#include "../core/Bar.h"
#include "../core/KeySignature.h"
#include "../core/Staff.h"

#include "../MusicShape.h"

using namespace MusicCore;

RemoveStaffElementCommand::RemoveStaffElementCommand(MusicShape* shape, StaffElement* se, Bar* bar)
    : m_shape(shape), m_element(se), m_bar(bar), m_index(m_bar->indexOfStaffElement(se))
{
    if (dynamic_cast<Clef*>(se)) {
        setText(kundo2_i18n("Remove clef"));
    } else {
        setText(kundo2_i18n("Remove staff element"));
    }
}

void RemoveStaffElementCommand::redo()
{
    m_bar->removeStaffElement(m_element, false);
    if (dynamic_cast<KeySignature*>(m_element)) {
        m_element->staff()->updateAccidentals(m_bar);
    }
    m_shape->engrave();
    m_shape->update();
}

void RemoveStaffElementCommand::undo()
{
    m_bar->addStaffElement(m_element, m_index);
    if (dynamic_cast<KeySignature*>(m_element)) {
        m_element->staff()->updateAccidentals(m_bar);
    }
    m_shape->engrave();
    m_shape->update();
}
