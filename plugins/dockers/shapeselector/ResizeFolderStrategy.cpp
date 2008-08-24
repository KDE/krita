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
#include "ResizeFolderStrategy.h"
#include "FolderShape.h"
#include "Canvas.h"

#include <KoShapeManager.h>
#include <KoSelection.h>
#include <KoPointerEvent.h>

ResizeFolderStrategy::ResizeFolderStrategy(Canvas *canvas, FolderShape *clickedFolder, KoPointerEvent &event)
    : m_canvas(canvas), m_folder(clickedFolder)
{
    Q_ASSERT(m_canvas);
    Q_ASSERT(m_folder);
    m_startPosition = event.point;
    m_startSize = clickedFolder->size();
    m_folderStartPosition = clickedFolder->position();

    QPointF offsetInFolder = m_startPosition - m_folderStartPosition;
    if (offsetInFolder.x() < 10) // on the left
        m_scaleRole = MoveLeftBorder;
    else if (offsetInFolder.x() < m_startSize.width() - 10) // not on right side.
        m_scaleRole = MoveBottomBorder;
    else if (offsetInFolder.y() < m_startSize.height() - 10) // not on the bottom
        m_scaleRole = MoveRightBorder;
    else
        m_scaleRole = ScaleShape;
}

void ResizeFolderStrategy::handleMouseMove(const QPointF &mouseLocation, Qt::KeyboardModifiers modifiers)
{
    Q_UNUSED(modifiers);
    m_folder->update();
    QPointF distanceTravelled = mouseLocation - m_startPosition;
    QSizeF newSize = m_startSize;
    QPointF newPosition = m_folderStartPosition;
    switch(m_scaleRole) {
    case MoveLeftBorder:
        newPosition.setX(newPosition.x() + distanceTravelled.x());
        newSize.setWidth(newSize.width() - distanceTravelled.x());
        break;
    case MoveRightBorder:
        newSize.setWidth( qMax(qreal(40), m_startSize.width() + distanceTravelled.x()) );
        break;
    case MoveBottomBorder:
        newSize.setHeight( qMax(qreal(40), m_startSize.height() + distanceTravelled.y()) );
        break;
    case ScaleShape:
        newSize = QSizeF( qMax(qreal(40), m_startSize.width() + distanceTravelled.x()), qMax(qreal(40), m_startSize.height() + distanceTravelled.y()) );
        break;
    }
    m_folder->setSize(newSize);
    m_folder->setPosition(newPosition);
    m_folder->update();
}

