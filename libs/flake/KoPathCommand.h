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

/// the base command for commands altering a path shape
class KoPointBaseCommand : public KCommand {
public:
    /// intialize the base command with the shape
    KoPointBaseCommand( KoPathShape *shape );
protected:
    /**
     * Call this to repaint the shape after altering.
     * @param oldControlPointRect the control point rect of the shape before altering
     */
    void repaint( const QRectF &oldControlPointRect );
    KoPathShape *m_shape; ///< the shape the command operates on
};

/// The undo / redo command for path point moving.
class KoPointMoveCommand : public KoPointBaseCommand {
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
    QList<KoPathPoint*> m_points;
    QPointF m_offset;
    KoPathPoint::KoPointType m_pointType;
};

/// The undo / redo command for changing the path point type.
class KoPointPropertyCommand : public KoPointBaseCommand {
public:
    /**
     * Command to change the type of the point
     * @param shape the path shape containing the point
     * @param point the path point to move
     * @param property ??
     */
    KoPointPropertyCommand( KoPathShape *shape, KoPathPoint *point, KoPathPoint::KoPointProperties property );
    /// execute the command
    void execute();
    /// revert the actions done in execute
    void unexecute();
    /// return the name of this command
    QString name() const;
private:
    KoPathPoint* m_point;
    KoPathPoint::KoPointProperties m_newProperties;
    KoPathPoint::KoPointProperties m_oldProperties;
    QPointF m_controlPoint1;
    QPointF m_controlPoint2;
};

/// The undo / redo command for removing path points.
class KoPointRemoveCommand : public KoPointBaseCommand {
public:
    /**
     * Command to remove a single point from a path shape
     * @param shape the path shape containing the point
     * @param point the path point to remove
     */
    KoPointRemoveCommand( KoPathShape *shape, KoPathPoint *point );
    /**
     * Command to remove multiple points from a path shape
     * @param shape the path shape containing the points
     * @param points the path points to remove
     */
    KoPointRemoveCommand( KoPathShape *shape, const QList<KoPathPoint*> &points );
    /// execute the command
    void execute();
    /// revert the actions done in execute
    void unexecute();
    /// return the name of this command
    QString name() const;
private:
    struct KoPointRemoveData
    {
        KoPointRemoveData( KoPathPoint * point )
        : m_point( point )
        , m_subpath( 0 )
        , m_position( 0 )
        {}
        KoPathPoint * m_point;
        KoSubpath * m_subpath;///< the position in the path 
        int m_position;
    };
    QList<KoPointRemoveData> m_data;
};

#endif
