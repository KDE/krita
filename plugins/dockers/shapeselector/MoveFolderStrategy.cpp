/*
 * Copyright (C) 2008 Thomas Zander <zander@kde.org>
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
#include "MoveFolderStrategy.h"
#include "FolderShape.h"
#include "Canvas.h"

#include <KoShapeManager.h>
#include <KoSelection.h>
#include <KoPointerEvent.h>

MoveFolderStrategy::MoveFolderStrategy(Canvas *canvas, FolderShape *clickedFolder, KoPointerEvent &event)
    : m_canvas(canvas), m_folder(clickedFolder)
{
    Q_ASSERT(m_canvas);
    Q_ASSERT(m_folder);
    m_startPosition = clickedFolder->position();
    m_offsetInFolder = event.point - m_startPosition;
}

void MoveFolderStrategy::handleMouseMove(const QPointF &mouseLocation, Qt::KeyboardModifiers modifiers)
{
    Q_UNUSED(modifiers); // TODO use modifiers to move a folder to be *inside* another folder.  I think shift would do.
    m_folder->update();
    m_folder->setPosition(mouseLocation - m_offsetInFolder);
    m_folder->update();
}

void MoveFolderStrategy::cancelInteraction()
{
    m_folder->update();
    m_folder->setPosition(m_startPosition);
    m_folder->update();
    m_canvas->shapeManager()->selection()->deselect(m_folder);
}

