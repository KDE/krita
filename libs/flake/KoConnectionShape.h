/* This file is part of the KDE project
 * Copyright (C) 2007 Boudewijn Rempt <boud@kde.org>
 * Copyright (C) 2007 Thorsten Zachmann <zachmann@kde.org>
 * Copyright (C) 2007 Jan Hambrecht <jaham@gmx.net>
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

#ifndef KO_CONNECTION_SHAPE_H
#define KO_CONNECTION_SHAPE_H

#include "KoParameterShape.h"

#include "flake_export.h"

#define KOCONNECTIONSHAPEID "KoConnectionShape"

class KoConnectionShapePrivate;

/// API docs go here
class FLAKE_EXPORT KoConnectionShape : public KoParameterShape
{
public:
    enum Type {
        Standard, ///< escapes connected shapes with straight lines, connects with perpendicular lines
        Lines,    ///< escapes connected shapes with straight lines, connects with straight line
        Straight, ///< one straight line between connected shapes
        Curve     ///< a single curved line between connected shapes
    };

    KoConnectionShape();
    virtual ~KoConnectionShape();

    // reimplemented
    virtual void saveOdf(KoShapeSavingContext &context) const;

    // reimplemented
    virtual bool loadOdf(const KoXmlElement &element, KoShapeLoadingContext &context);

    // reimplemented
    virtual QString pathShapeId() const;

    /// reimplemented
    virtual void shapeChanged(ChangeType type, KoShape *shape);

    /**
     * Sets the first shape this connector is connected to
     *
     * Passing a null pointer as the first parameter will sever the connection.
     *
     * @param shape the shape to connect to or null to reset the connection
     * @param connectionPointIndex the index of the connection point to connect to
     * @return true if connection could be established, otherwise false
     */
    bool connectFirst(KoShape *shape, int connectionPointIndex);

    /**
    * Sets the second shape the connector is connected to
    *
    * Passing a null pointer as the first parameter will sever the connection.
    *
    * @param shape the shape to connect to or null to reset the connection
    * @param connectionPointIndex the index of the connection point to connect to
    * @return true if connection could be established, otherwise false
    */
    bool connectSecond(KoShape *shape, int connectionPointIndex);

    /**
     * Return the first shape this connection is attached to, or null if none.
     */
    KoShape *firstShape() const;

    /**
     * Return the connection-index in the first shape we are connected to.
     * In case we are not connected to a first shape the return value is undefined.
     * @see firstShape(), KoShape::connectionPoints()
     */
    int firstConnectionIndex() const;

    /**
     * Return the second shape this connection is attached to, or null if none.
     */
    KoShape *secondShape() const;

    /**
     * Return the connection-index in the second shape we are connected to.
     * In case we are not connected to a second shape the return value is undefined.
     * @see firstShape(), KoShape::connectionPoints()
     */
    int secondConnectionIndex() const;

    /// Updates connections to shapes
    void updateConnections();

    /// Returns connection type
    Type type() const;

    /// Sets the connection type
    void setType(Type connectionType);

protected:
    /// reimplemented
    void moveHandleAction(int handleId, const QPointF &point, Qt::KeyboardModifiers modifiers = Qt::NoModifier);

    /// reimplemented
    void updatePath(const QSizeF &size);

private:
    Q_DECLARE_PRIVATE(KoConnectionShape)
};


#endif
