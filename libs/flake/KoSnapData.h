/* This file is part of the KDE project
 * Copyright (C) 2008 Jan Hambrecht <jaham@gmx.net>
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

#ifndef KOSNAPDATA_H
#define KOSNAPDATA_H

#include <KoPathSegment.h>

/**
 * This class is used to provide additional data to the snap guide.
 *
 * Some shapes might have internal data it wants snapping support for,
 * i.e. the axis of a chart shape, the row of a table shape, etc.
 * As the data is internal and special to that shape, the snap guide
 * does not know about it and can therefore not provide any snapping
 * to it.
 * So the shape can put that data in form of points or segments into
 * that class which the snap guide can retrieve and use accordingly.
 */
class FLAKE_TEST_EXPORT KoSnapData
{
public:
    KoSnapData();
    ~KoSnapData();

    /// Returns list of points to snap to
    QList<QPointF> snapPoints() const;

    /// Sets list of points to snap to
    void setSnapPoints(const QList<QPointF> &snapPoints);

    /// Returns list of segments to snap to
    QList<KoPathSegment> snapSegments() const;

    /// Sets list of segments to snap to
    void setSnapSegments(const QList<KoPathSegment> &snapSegments);

private:
    QList<QPointF> m_points;
    QList<KoPathSegment> m_segments;
};

#endif //KOSNAPDATA_H
