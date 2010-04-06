/* This file is part of the KDE project
 * Copyright (C) 2009 Thomas Zander <zander@kde.org>
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
#ifndef KOPATHSHAPEPRIVATE_H
#define KOPATHSHAPEPRIVATE_H

#include "KoShape_p.h"

class KoPathShapePrivate : public KoShapePrivate
{
public:
    KoPathShapePrivate(KoPathShape *q);

    QRectF handleRect(const QPointF &p, qreal radius) const;
    /// Applies the viewbox transformation defined in the given element
    void applyViewboxTransformation(const KoXmlElement &element);

    void map(const QMatrix &matrix);

    void updateLast(KoPathPoint **lastPoint);

    /// closes specified subpath
    void closeSubpath(KoSubpath *subpath);
    /// close-merges specified subpath
    void closeMergeSubpath(KoSubpath *subpath);

    /**
     * @brief Saves the node types
     *
     * This is inspired by inkscape and uses the same mechanism as they do.
     * The only difference is that they use sodipodi:nodeTypes as element and
     * we use koffice:nodeTyes as attribute.
     * This attribute contains of a string which has the node type of each point
     * in it. The following node types exist:
     *
     * c corner
     * s smooth
     * z symetric
     *
     * The first point of a path is always of the type c.
     * If the path is closed the type of the first point is saved in the last elemeent
     * E.g. you have a closed path with 2 points in it. The first one (start/end of path)
     * is symetric and the second one is smooth that will result in the nodeType="czs"
     * So if there is a closed sub path the nodeTypes contain one more entry then there
     * are points. That is due to the first and the last pojnt of a closed sub path get
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

    Q_DECLARE_PUBLIC(KoPathShape)
};

#endif
