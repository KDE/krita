/* This file is part of the KDE project
 * Copyright (C) 2006 Jan Hambrecht <jaham@gmx.net>
 * Copyright (C) 2006,2007 Thorsten Zachmann <zachmann@kde.org>
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

#ifndef KOPARAMETERHANDLEMOVECOMMAND_H
#define KOPARAMETERHANDLEMOVECOMMAND_H

#include <QUndoCommand>
#include <QPointF>
#include <flake_export.h>

class KoParameterShape;

/// The undo / redo command for changing a parameter
class KoParameterHandleMoveCommand : public QUndoCommand
{
public:
    /**
     * Constructor.
     * @param shape the shape this command works on
     * @param handleId the ID under which the parameterShape knows the handle in KoParameterShape::moveHandle()
     * @param startPoint The old position
     * @param endPoint The new position
     * @parent parent the parent command if this is a compound undo command.
     */
    KoParameterHandleMoveCommand( KoParameterShape *shape, int handleId, const QPointF &startPoint, const QPointF &endPoint, QUndoCommand *parent = 0 );
    virtual ~KoParameterHandleMoveCommand();

    /// redo the command
    void redo();
    /// revert the actions done in redo
    void undo();
private:
    KoParameterShape *m_shape;
    int m_handleId;
    QPointF m_startPoint;
    QPointF m_endPoint;
};

#endif // KOPARAMETERHANDLEMOVECOMMAND_H

