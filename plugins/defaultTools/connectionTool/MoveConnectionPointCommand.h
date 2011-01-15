/* This file is part of the KDE project
 * 
 * Copyright (C) 2011 Jan Hambrecht <jaham@gmx.net>
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

#ifndef MOVECONNECTIONPOINTCOMMAND_H
#define MOVECONNECTIONPOINTCOMMAND_H

#include <QtGui/QUndoCommand>
#include <QtCore/QPointF>

class KoShape;

class MoveConnectionPointCommand : public QUndoCommand
{
public:
    /// Creates new comand to move connection point of shape
    MoveConnectionPointCommand(KoShape *shape, int connectionPointId, const QPointF &oldPosition, const QPointF &newPosition, QUndoCommand *parent = 0);
    virtual ~MoveConnectionPointCommand();
    /// reimplemented from QUndoCommand
    virtual void redo();
    /// reimplemented from QUndoCommand
    virtual void undo();

private:
    void updateRoi(const QPointF &position);

    KoShape * m_shape;
    int m_connectionPointId;
    QPointF m_oldPosition;
    QPointF m_newPosition;
};

#endif // MOVECONNECTIONPOINTCOMMAND_H