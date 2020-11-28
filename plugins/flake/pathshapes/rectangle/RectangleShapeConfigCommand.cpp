/* This file is part of the KDE project
 * SPDX-FileCopyrightText: 2007 Jan Hambrecht <jaham@gmx.net>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include "RectangleShapeConfigCommand.h"
#include "RectangleShape.h"
#include <klocalizedstring.h>
#include "kis_command_ids.h"


RectangleShapeConfigCommand::RectangleShapeConfigCommand(RectangleShape *rectangle, qreal cornerRadiusX, qreal cornerRadiusY, KUndo2Command *parent)
    : KUndo2Command(parent)
    , m_rectangle(rectangle)
    , m_newCornerRadiusX(cornerRadiusX)
    , m_newCornerRadiusY(cornerRadiusY)
{
    Q_ASSERT(m_rectangle);

    setText(kundo2_i18n("Change rectangle"));

    m_oldCornerRadiusX = m_rectangle->cornerRadiusX();
    m_oldCornerRadiusY = m_rectangle->cornerRadiusY();
}

void RectangleShapeConfigCommand::redo()
{
    KUndo2Command::redo();

    m_rectangle->update();

    if (m_oldCornerRadiusX != m_newCornerRadiusX) {
        m_rectangle->setCornerRadiusX(m_newCornerRadiusX);
    }
    if (m_oldCornerRadiusY != m_newCornerRadiusY) {
        m_rectangle->setCornerRadiusY(m_newCornerRadiusY);
    }

    m_rectangle->update();
}

void RectangleShapeConfigCommand::undo()
{
    KUndo2Command::undo();

    m_rectangle->update();

    if (m_oldCornerRadiusX != m_newCornerRadiusX) {
        m_rectangle->setCornerRadiusX(m_oldCornerRadiusX);
    }
    if (m_oldCornerRadiusY != m_newCornerRadiusY) {
        m_rectangle->setCornerRadiusY(m_oldCornerRadiusY);
    }

    m_rectangle->update();
}

int RectangleShapeConfigCommand::id() const
{
    return KisCommandUtils::ChangeRectangleShapeId;
}

bool RectangleShapeConfigCommand::mergeWith(const KUndo2Command *command)
{
    const RectangleShapeConfigCommand *other = dynamic_cast<const RectangleShapeConfigCommand*>(command);

    if (!other || other->m_rectangle != m_rectangle) {
        return false;
    }

    m_newCornerRadiusX = other->m_newCornerRadiusX;
    m_newCornerRadiusY = other->m_newCornerRadiusY;

    return true;
}
