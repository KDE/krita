/* This file is part of the KDE project
 * Copyright (C) 2007 Rob Buis <buis@kde.org>
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

#include "SpiralShapeConfigCommand.h"
#include <klocalizedstring.h>

SpiralShapeConfigCommand::SpiralShapeConfigCommand(SpiralShape *spiral, SpiralShape::SpiralType type, bool clockWise, qreal fade, KUndo2Command *parent)
    : KUndo2Command(parent)
    , m_spiral(spiral)
    , m_newType(type)
    , m_newClockWise(clockWise)
    , m_newFade(fade)
{
    Q_ASSERT(m_spiral);

    setText(kundo2_i18n("Change spiral"));

    m_oldType = m_spiral->type();
    m_oldClockWise = m_spiral->clockWise();
    m_oldFade = m_spiral->fade();
}

void SpiralShapeConfigCommand::redo()
{
    KUndo2Command::redo();

    m_spiral->update();

    if (m_oldType != m_newType) {
        m_spiral->setType(m_newType);
    }
    if (m_oldClockWise != m_newClockWise) {
        m_spiral->setClockWise(m_newClockWise);
    }
    if (m_oldFade != m_newFade) {
        m_spiral->setFade(m_newFade);
    }

    m_spiral->update();
}

void SpiralShapeConfigCommand::undo()
{
    KUndo2Command::undo();

    m_spiral->update();

    if (m_oldType != m_newType) {
        m_spiral->setType(m_oldType);
    }
    if (m_oldClockWise != m_newClockWise) {
        m_spiral->setClockWise(m_oldClockWise);
    }
    if (m_oldFade != m_newFade) {
        m_spiral->setFade(m_oldFade);
    }

    m_spiral->update();
}
