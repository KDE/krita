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
#include "SetTimeSignatureCommand.h"

#include "../core/Bar.h"
#include "../core/Sheet.h"
#include "../core/Part.h"

#include "../MusicShape.h"

#include <klocalizedstring.h>

using namespace MusicCore;

SetTimeSignatureCommand::SetTimeSignatureCommand(MusicShape* shape, Bar* bar, int beats, int beat)
    : m_shape(shape), m_bar(bar)
{
    setText(kundo2_i18n("Change time signature"));
    
    Sheet* sheet = bar->sheet();
    for (int p = 0; p < sheet->partCount(); p++) {
        Part* part = sheet->part(p);
        for (int s = 0; s < part->staffCount(); s++) {
            Staff* staff = part->staff(s);
            m_newSigs.append(new TimeSignature(staff, 0, beats, beat));
            for (int i = 0; i < bar->staffElementCount(staff); i++) {
                TimeSignature* ts = dynamic_cast<TimeSignature*>(bar->staffElement(staff, i));
                if (ts) {
                    m_oldSigs.append(ts);
                    break;
                }
            }
        }
    }
}

void SetTimeSignatureCommand::redo()
{
    foreach (TimeSignature* ts, m_oldSigs) {
        m_bar->removeStaffElement(ts, false);
    }
    foreach (TimeSignature* ts, m_newSigs) {
        m_bar->addStaffElement(ts);
    }
    m_shape->engrave();
    m_shape->update();
}

void SetTimeSignatureCommand::undo()
{
    foreach (TimeSignature* ts, m_newSigs) {
        m_bar->removeStaffElement(ts, false);
    }
    foreach (TimeSignature* ts, m_oldSigs) {
        m_bar->addStaffElement(ts);
    }
    m_shape->engrave();
    m_shape->update();
}
