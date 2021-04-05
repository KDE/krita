/* This file is part of the KDE project
 * SPDX-FileCopyrightText: 2009 Thomas Zander <zander@kde.org>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
*/
#ifndef KOPATHSHAPEPRIVATE_H
#define KOPATHSHAPEPRIVATE_H

#include "KoPathShape.h"
#include "KoMarker.h"

#include <QSharedData>

class KoPathShape::Private
{
public:
    explicit Private();
    explicit Private(const Private &rhs);

    QRectF handleRect(const QPointF &p, qreal radius) const;

    void map(const QTransform &matrix);

    /**
     * @brief Returns subpath at given index
     * @param subpathIndex the index of the subpath to return
     * @return subPath on success, or 0 when subpathIndex is out of bounds
     */
    KoSubpath *subPath(int subpathIndex) const;
#ifndef NDEBUG
    /// \internal
    void paintDebug(QPainter &painter);
    /**
     * @brief print debug information about a the points of the path
     */
    void debugPath() const;
#endif

    Qt::FillRule fillRule;

    KoSubpathList subpaths;

    QMap<KoFlake::MarkerPosition, QExplicitlySharedDataPointer<KoMarker>> markersNew;
    bool autoFillMarkers;
};

#endif
