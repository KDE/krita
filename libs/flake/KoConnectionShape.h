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

#include "kritaflake_export.h"

#define KOCONNECTIONSHAPEID "KoConnectionShape"

class KoConnectionShapePrivate;

/// API docs go here
class KRITAFLAKE_EXPORT KoConnectionShape : public KoParameterShape
{
public:
    enum Type {
        Standard, ///< escapes connected shapes with straight lines, connects with perpendicular lines
        Lines,    ///< escapes connected shapes with straight lines, connects with straight line
        Straight, ///< one straight line between connected shapes
        Curve     ///< a single curved line between connected shapes
    };

    // IDs of the connecting handles
    enum HandleId {
        StartHandle,
        EndHandle,
        ControlHandle_1,
        ControlHandle_2,
        ControlHandle_3
    };

    KoConnectionShape();
    ~KoConnectionShape() override;

    KoShape* cloneShape() const override;

    // reimplemented
    void saveOdf(KoShapeSavingContext &context) const override;

    // reimplemented
    bool loadOdf(const KoXmlElement &element, KoShapeLoadingContext &context) override;

    // reimplemented
    QString pathShapeId() const override;

    /**
     * Sets the first shape this connector is connected to
     *
     * Passing a null pointer as the first parameter will sever the connection.
     *
     * @param shape the shape to connect to or null to reset the connection
     * @param connectionPointId the id of the connection point to connect to
     * @return true if connection could be established, otherwise false
     */
    bool connectFirst(KoShape *shape, int connectionPointId);

    /**
    * Sets the second shape the connector is connected to
    *
    * Passing a null pointer as the first parameter will sever the connection.
    *
    * @param shape the shape to connect to or null to reset the connection
    * @param connectionPointId the id of the connection point to connect to
    * @return true if connection could be established, otherwise false
    */
    bool connectSecond(KoShape *shape, int connectionPointId);

    /**
     * Return the first shape this connection is attached to, or null if none.
     */
    KoShape *firstShape() const;

    /**
     * Return the connection point id of the first shape we are connected to.
     * In case we are not connected to a first shape the return value is undefined.
     * @see firstShape(), KoShape::connectionPoints()
     */
    int firstConnectionId() const;

    /**
     * Return the second shape this connection is attached to, or null if none.
     */
    KoShape *secondShape() const;

    /**
     * Return the connection point id of the second shape we are connected to.
     * In case we are not connected to a second shape the return value is undefined.
     * @see firstShape(), KoShape::connectionPoints()
     */
    int secondConnectionId() const;

    /**
     * Finishes the loading of a connection.
     */
    void finishLoadingConnection();

    /// Returns connection type
    Type type() const;

    /// Sets the connection type
    void setType(Type connectionType);

    /// Updates connections to shapes
    void updateConnections();

protected:
    KoConnectionShape(const KoConnectionShape &rhs);


    /// reimplemented
    void moveHandleAction(int handleId, const QPointF &point, Qt::KeyboardModifiers modifiers = Qt::NoModifier) override;

    /// reimplemented
    void updatePath(const QSizeF &size) override;

    /// reimplemented
    void shapeChanged(ChangeType type, KoShape *shape) override;
private:
    QPointF escapeDirection(int handleId) const;

    /// Populate the path list by a normal way
    void normalPath(const qreal MinimumEscapeLength);

private:
    class Private;
    QSharedDataPointer<Private> d;
};

#endif
