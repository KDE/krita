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

#ifndef CHANGECONNECTIONPOINTCOMMAND_H
#define CHANGECONNECTIONPOINTCOMMAND_H

#include <KoConnectionPoint.h>
#include <kundo2command.h>
#include <QPointF>

class KoShape;

class ChangeConnectionPointCommand : public KUndo2Command
{
public:
    /// Creates new comand to change connection point of shape
    ChangeConnectionPointCommand(KoShape *shape, int connectionPointId, const KoConnectionPoint &oldPoint, const KoConnectionPoint &newPoint, KUndo2Command *parent = 0);
    virtual ~ChangeConnectionPointCommand();
    /// reimplemented from KUndo2Command
    virtual void redo();
    /// reimplemented from KUndo2Command
    virtual void undo();

private:
    void updateRoi(const QPointF &position);

    KoShape *m_shape;
    int m_connectionPointId;
    KoConnectionPoint m_oldPoint;
    KoConnectionPoint m_newPoint;
};

#endif // CHANGECONNECTIONPOINTCOMMAND_H
