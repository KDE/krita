/* This file is part of the KDE project
 * Copyright (C) 2006 Jan Hambrecht <jaham@gmx.net>
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

#ifndef KOPATHCOMMAND_H
#define KOPATHCOMMAND_H

#include <kcommand.h>
#include <QList>
#include <QPointF>
#include <KoPathShape.h>

/// The undo / redo command for path point moving.
class KoPointMoveCommand : public KCommand {
public:
    /**
     * Command to move single path point.
     * @param shape the path shape containing the point
     * @param point the path point to move
     * @param offset the offset by which the point is moved
     * @param pointType the type of the point to move
     */
    KoPointMoveCommand( KoPathShape *shape, KoPathPoint *point, const QPointF &offset, KoPathPoint::KoPointType pointType );
    /**
     * Command to move multiple points at once.
     * @param shape the path shape containing the points
     * @param points the path points to move
     * @param offset the offset by which the points are moved
     */
    KoPointMoveCommand( KoPathShape *shape, const QList<KoPathPoint*> &points, const QPointF &offset );
    /// execute the command
    void execute();
    /// revert the actions done in execute
    void unexecute();
    /// return the name of this command
    QString name() const;
private:
    KoPathShape *m_shape;
    QList<KoPathPoint*> m_points;
    QPointF m_offset;
    KoPathPoint::KoPointType m_pointType;
};

#endif
