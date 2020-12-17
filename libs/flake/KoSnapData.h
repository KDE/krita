/* This file is part of the KDE project
 * SPDX-FileCopyrightText: 2008 Jan Hambrecht <jaham@gmx.net>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#ifndef KOSNAPDATA_H
#define KOSNAPDATA_H

#include <kritaflake_export.h>

#include "KoPathSegment.h"

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
class KRITAFLAKE_EXPORT KoSnapData
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
