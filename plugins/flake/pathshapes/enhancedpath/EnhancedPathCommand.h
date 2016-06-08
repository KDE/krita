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

#ifndef KOENHANCEDPATHCOMMAND_H
#define KOENHANCEDPATHCOMMAND_H

#include <QChar>
#include <QList>
#include <QPointF>
#include <QRectF>

class EnhancedPathShape;
class EnhancedPathParameter;
class KoPathPoint;

/**
 * A EnhancedPathCommand is a command like moveto, curveto, etc.
 * that directly modifies an enhanced paths outline.
 */
class EnhancedPathCommand
{
public:
    /// Constructs a new command from the given command type
    EnhancedPathCommand(const QChar &command, EnhancedPathShape *parent);
    ~EnhancedPathCommand();
    /// Excutes the command on the specified path shape
    bool execute();
    /// Adds a new parameter to the command
    void addParameter(EnhancedPathParameter *parameter);
    /// Returns a string representation of the command
    QString toString() const;
private:
    /// Returns a list of points, created from the parameter list
    QList<QPointF> pointsFromParameters();
    /// Calculates angle from given point
    qreal angleFromPoint(const QPointF &point) const;
    /// Returns sweep angle from start to stop and given direction
    qreal radSweepAngle(qreal start, qreal stop, bool clockwise) const;
    /// Returns sweep angle from start to stop and given direction
    qreal degSweepAngle(qreal start, qreal stop, bool clockwise) const;
    /// Returns the last path point of given path
    KoPathPoint *lastPathPoint() const;
    /// Returns rectangle from given points
    QRectF rectFromPoints(const QPointF &tl, const QPointF &br) const;

    QChar m_command; ///< the actual command
    QList<EnhancedPathParameter *> m_parameters; ///< the commands parameters
    EnhancedPathShape *m_parent; ///< the enhanced path owning the command
};

#endif // KOENHANCEDPATHCOMMAND_H
