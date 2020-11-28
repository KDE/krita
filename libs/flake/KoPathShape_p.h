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
     * @brief Saves the node types
     *
     * This is inspired by inkscape and uses the same mechanism as they do.
     * The only difference is that they use sodipodi:nodeTypes as element and
     * we use calligra:nodeTyes as attribute.
     * This attribute contains of a string which has the node type of each point
     * in it. The following node types exist:
     *
     * c corner
     * s smooth
     * z symmetric
     *
     * The first point of a path is always of the type c.
     * If the path is closed the type of the first point is saved in the last element
     * E.g. you have a closed path with 2 points in it. The first one (start/end of path)
     * is symmetric and the second one is smooth that will result in the nodeType="czs"
     * So if there is a closed sub path the nodeTypes contain one more entry then there
     * are points. That is due to the first and the last point of a closed sub path get
     * merged into one when they are on the same position.
     *
     * @return The node types as string
     */
    QString nodeTypes() const;

    /**
     * @brief Loads node types
     */
    void loadNodeTypes(const KoXmlElement &element);

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
