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

#ifndef ADDCONNECTIONPOINTCOMMAND_H
#define ADDCONNECTIONPOINTCOMMAND_H

#include <kundo2command.h>
#include <QPointF>

class KoShape;

class AddConnectionPointCommand : public KUndo2Command
{
public:
    /// Creates new command to add connection point to shape
    AddConnectionPointCommand(KoShape *shape, const QPointF &connectionPoint, KUndo2Command *parent = 0);
    ~AddConnectionPointCommand() override;
    /// reimplemented from KUndo2Command
    void redo() override;
    /// reimplemented from KUndo2Command
    void undo() override;

private:
    void updateRoi();

    KoShape *m_shape;
    QPointF m_connectionPoint;
    int m_connectionPointId;
};

#endif // ADDCONNECTIONPOINTCOMMAND_H
