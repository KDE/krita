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

#ifndef REMOVECONNECTIONPOINTCOMMAND_H
#define REMOVECONNECTIONPOINTCOMMAND_H

#include <KoConnectionPoint.h>
#include <kundo2command.h>
#include <QPointF>

class KoShape;

class RemoveConnectionPointCommand : public KUndo2Command
{
public:
    /// Creates new comand to remove connection point from shape
    RemoveConnectionPointCommand(KoShape *shape, int connectionPointId, KUndo2Command *parent = 0);
    virtual ~RemoveConnectionPointCommand();
    /// reimplemented from KUndo2Command
    virtual void redo();
    /// reimplemented from KUndo2Command
    virtual void undo();

private:
    void updateRoi();

    KoShape *m_shape;
    KoConnectionPoint m_connectionPoint;
    int m_connectionPointId;
};

#endif // REMOVECONNECTIONPOINTCOMMAND_H
