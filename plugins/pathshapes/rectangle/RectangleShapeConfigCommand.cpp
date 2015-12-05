/* This file is part of the KDE project
 * Copyright (C) 2007 Jan Hambrecht <jaham@gmx.net>
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

#include "RectangleShapeConfigCommand.h"
#include "RectangleShape.h"
#include <klocalizedstring.h>

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
