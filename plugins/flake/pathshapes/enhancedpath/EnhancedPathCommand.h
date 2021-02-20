/* This file is part of the KDE project
 * SPDX-FileCopyrightText: 2007 Jan Hambrecht <jaham@gmx.net>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
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
    /// Executes the command on the specified path shape
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
