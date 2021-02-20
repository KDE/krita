/* This file is part of the KDE project
 * SPDX-FileCopyrightText: 2007 Rob Buis <buis@kde.org>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
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
