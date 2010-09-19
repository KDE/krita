/* This file is part of the KDE project
 * Copyright (C) 2007 Jan Hambrecht <jaham@gmx.net>
 * Copyright (C) 2010 Thomas Zander <zander@kde.org>
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

#include "EnhancedPathCommand.h"
#include "EnhancedPathParameter.h"
#include "EnhancedPathShape.h"
#include <KoPathPoint.h>
#include <math.h>

#include <KDebug>

// radian to degree factor
const qreal rad2deg = 180.0/M_PI;

EnhancedPathCommand::EnhancedPathCommand(const QChar &command, EnhancedPathShape *parent)
    : m_command(command), m_parent(parent)
{
    Q_ASSERT(m_parent);
}

EnhancedPathCommand::~EnhancedPathCommand()
{
}

bool EnhancedPathCommand::execute()
{
    /*
     * The parameters of the commands are in viewbox coordinates, which have
     * to be converted to the shapes coordinate system by calling viewboxToShape
     * on the enhanced path the command works on.
     * Parameters which resemble angles are angles corresponding to the viewbox
     * coordinate system. Those have to be transformed into angles corresponding
     * to the normal mathematically coordinate system to be used for the arcTo
     * drawing routine. This is done by computing (2*M_PI - angle).
     */
    QList<QPointF> points = pointsFromParameters();
    const int pointsCount = points.size();

    switch (m_command.unicode()) {
    // starts new subpath at given position (x y) +
    case 'M':
        if (!pointsCount)
            return false;
        m_parent->moveTo(points[0]);
        if (pointsCount > 1)
            for (int i = 1; i < pointsCount; i++)
                m_parent->lineTo(points[i]);
        break;
    // line from current point (x y) +
    case 'L':
        foreach(const QPointF &point, points)
            m_parent->lineTo(point);
        break;
    // cubic bezier curve from current point (x1 y1 x2 y2 x y) +
    case 'C':
        for (int i = 0; i < pointsCount; i+=3)
            m_parent->curveTo(points[i], points[i+1], points[i+2]);
        break;
    // closes the current subpath
    case 'Z':
        m_parent->close();
        break;
    // ends the current set of subpaths
    case 'N':
        // N just ends the complete path
        break;
    // no fill for current set of subpaths
    case 'F':
        // TODO implement me
        break;
    // no stroke for current set of subpaths
    case 'S':
        // TODO implement me
        break;
    // segment of an ellipse (x y w h t0 t1) +
    case 'T':
    // same like T but with implied movement to starting point (x y w h t0 t1) +
    case 'U': {
        bool lineTo = m_command.unicode() == 'T';

        for (int i = 0; i < pointsCount; i+=3) {
            const QPointF &radii = points[i+1];
            const QPointF &angles = points[i+2] / rad2deg;
            // compute the ellipses starting point
            QPointF start(radii.x() * cos(angles.x()), radii.y() * sin(angles.x()));
            qreal sweepAngle = degSweepAngle(points[i+2].x(), points[i+2].y(), false);

            if (lineTo)
                m_parent->lineTo(points[i] + start);
            else
                m_parent->moveTo(points[i] + start);

            m_parent->arcTo(radii.x(), radii.y(), points[i+2].x(), sweepAngle);
        }
        break;
    }
    // counter-clockwise arc (x1 y1 x2 y2 x3 y3 x y) +
    case 'A':
    // the same as A, with implied moveto to the starting point (x1 y1 x2 y2 x3 y3 x y) +
    case 'B': {
        bool lineTo = m_command.unicode() == 'A';

        for (int i = 0; i < pointsCount; i+=4) {
            QRectF bbox = rectFromPoints(points[i], points[i+1]);
            QPointF center = bbox.center();
            qreal rx = 0.5 * bbox.width();
            qreal ry = 0.5 * bbox.height();
            qreal startAngle = angleFromPoint(points[i+2] - center);
            qreal stopAngle = angleFromPoint(points[i+3] - center);
            // we are moving counter-clockwise to the end angle
            qreal sweepAngle = radSweepAngle(startAngle, stopAngle, false);
            // compute the starting point to draw the line to
            QPointF startPoint(rx * cos(startAngle), ry * sin(2*M_PI - startAngle));

            if (lineTo)
                m_parent->lineTo(center + startPoint);
            else
                m_parent->moveTo(center + startPoint);

            m_parent->arcTo(rx, ry, startAngle * rad2deg, sweepAngle * rad2deg);
        }
        break;
    }
    // clockwise arc (x1 y1 x2 y2 x3 y3 x y) +
    case 'W':
    // the same as W, but implied moveto (x1 y1 x2 y2 x3 y3 x y) +
    case 'V': {
        bool lineTo = m_command.unicode() == 'W';

        for (int i = 0; i < pointsCount; i+=4) {
            QRectF bbox = rectFromPoints(points[i], points[i+1]);
            QPointF center = bbox.center();
            qreal rx = 0.5 * bbox.width();
            qreal ry = 0.5 * bbox.height();
            qreal startAngle = angleFromPoint(points[i+2] - center);
            qreal stopAngle = angleFromPoint(points[i+3] - center);
            // we are moving clockwise to the end angle
            qreal sweepAngle = radSweepAngle(startAngle, stopAngle, true);

            if (lineTo)
                m_parent->lineTo(points[i+2]);
            else
                m_parent->moveTo(points[i+2]);

            m_parent->arcTo(rx, ry, startAngle * rad2deg, sweepAngle * rad2deg);
        }
        break;
    }
    // elliptical quadrant (initial segment tangential to x-axis) (x y) +
    case 'X': {
        KoPathPoint * lastPoint = lastPathPoint();
        bool xDir = true;
        foreach (const QPointF &point, points) {
            qreal rx = point.x() - lastPoint->point().x();
            qreal ry = point.y() - lastPoint->point().y();
            qreal startAngle = xDir ? (ry > 0.0 ? 90.0 : 270.0) : (rx < 0.0 ? 0.0 : 180.0);
            qreal sweepAngle = xDir ? (rx*ry < 0.0 ? 90.0 : -90.0) : (rx*ry > 0.0 ? 90.0 : -90.0);
            lastPoint = m_parent->arcTo(fabs(rx), fabs(ry), startAngle, sweepAngle);
            xDir = !xDir;
        }
        break;
    }
    // elliptical quadrant (initial segment tangential to y-axis) (x y) +
    case 'Y': {
        KoPathPoint * lastPoint = lastPathPoint();
        bool xDir = false;
        foreach (const QPointF &point, points) {
            qreal rx = point.x() - lastPoint->point().x();
            qreal ry = point.y() - lastPoint->point().y();
            qreal startAngle = xDir ? (ry > 0.0 ? 90.0 : 270.0) : (rx < 0.0 ? 0.0 : 180.0);
            qreal sweepAngle = xDir ? (rx*ry < 0.0 ? 90.0 : -90.0) : (rx*ry > 0.0 ? 90.0 : -90.0);
            lastPoint = m_parent->arcTo(fabs(rx), fabs(ry), startAngle, sweepAngle);
            xDir = !xDir;
        }
        break;
    }
    // quadratic bezier curve (x1 y1 x y)+
    case 'Q':
        for (int i = 0; i < pointsCount; i+=2)
            m_parent->curveTo(points[i], points[i+1]);
        break;
    default:
        break;
    }
    return true;
}

QList<QPointF> EnhancedPathCommand::pointsFromParameters()
{
    QList<QPointF> points;
    QPointF p;

    int paramCount = m_parameters.count();
    for (int i = 0; i < paramCount - 1; i += 2) {
        p.setX(m_parameters[i]->evaluate());
        p.setY(m_parameters[i+1]->evaluate());
        points.append(p);
    }

    int mod = 1;
    if (m_command.unicode() == 'C' || m_command.unicode() == 'U'
            || m_command.unicode() == 'T') {
        mod = 3;
    } else if (m_command.unicode() == 'A' || m_command.unicode() == 'B'
            || m_command.unicode() == 'W' || m_command.unicode() == 'V') {
        mod = 4;
    } else if (m_command.unicode() == 'Q') {
        mod = 2;
    }
    if ((points.count() % mod) != 0) { // invalid command
        kWarning(31000) << "Invalid point count for command" << m_command << "ignoring";
        return QList<QPointF>();
    }

    return points;
}

void EnhancedPathCommand::addParameter(EnhancedPathParameter *parameter)
{
    if (parameter)
        m_parameters.append(parameter);
}

qreal EnhancedPathCommand::angleFromPoint(const QPointF &point) const
{
    qreal angle = atan2(point.y(), point.x());
    if (angle < 0.0)
        angle += 2*M_PI;

    return 2*M_PI - angle;
}

qreal EnhancedPathCommand::radSweepAngle(qreal start, qreal stop, bool clockwise) const
{
    qreal sweepAngle = stop - start;
    if (clockwise) {
        // we are moving clockwise to the end angle
        if (stop > start)
            sweepAngle = (stop - start) - 2*M_PI;
    } else {
        // we are moving counter-clockwise to the stop angle
        if (start > stop)
            sweepAngle = 2*M_PI - (start-stop);
    }

   return sweepAngle;
}

qreal EnhancedPathCommand::degSweepAngle(qreal start, qreal stop, bool clockwise) const
{
    qreal sweepAngle = stop - start;
    if (clockwise) {
        // we are moving clockwise to the end angle
        if (stop > start)
            sweepAngle = (stop - start) - 360.0;
    } else {
        // we are moving counter-clockwise to the stop angle
        if (start > stop)
            sweepAngle = 360.0 - (start-stop);
    }

   return sweepAngle;
}

KoPathPoint * EnhancedPathCommand::lastPathPoint() const
{
    KoPathPoint *lastPoint = 0;
    int subpathCount = m_parent->subpathCount();
    if (subpathCount) {
        int subpathPointCount = m_parent->subpathPointCount(subpathCount-1);
        lastPoint = m_parent->pointByIndex(KoPathPointIndex(subpathCount-1, subpathPointCount-1));
    }
    return lastPoint;
}

QRectF EnhancedPathCommand::rectFromPoints(const QPointF &tl, const QPointF &br) const
{
    return QRectF(tl, QSizeF(br.x()-tl.x(), br.y()-tl.y())).normalized();
}

QString EnhancedPathCommand::toString() const
{
    QString cmd = m_command;

    foreach(EnhancedPathParameter *p, m_parameters)
        cmd += p->toString() + ' ';

    return cmd.trimmed();
}
