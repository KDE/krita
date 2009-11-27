/* This file is part of the KDE project
 * Copyright (C) 2007 Boudewijn Rempt <boud@kde.org>
 * Copyright (C) 2007 Thorsten Zachmann <zachmann@kde.org>
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

#ifndef KO_CONNECTION_SHAPE_H
#define KO_CONNECTION_SHAPE_H

#include <KoParameterShape.h>

#include "flake_export.h"

#define KOCONNECTIONSHAPEID "KoConnectionShape"

/// A connection to a connection point of a shape
typedef QPair<KoShape*, int> KoConnection;

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

    explicit KoConnectionShape();
    virtual ~KoConnectionShape();

    // reimplemented
    virtual void paint(QPainter& painter, const KoViewConverter& converter);

    // reimplemented
    virtual void saveOdf(KoShapeSavingContext & context) const;

    // reimplemented
    virtual bool loadOdf(const KoXmlElement & element, KoShapeLoadingContext &context);

    // reimplemented
    inline QString pathShapeId() const {
        return KOCONNECTIONSHAPEID;
    }

    // reimplemented
    virtual bool hitTest(const QPointF &position) const;

    /**
     * Sets the first shape the connector is connected to
     *
     * Passing a null pointer as the first parameter will reset the connection.
     *
     * @param shape1 the shape to connect to or null to reset the connection
     * @param connectionPointIndex1 the index of the connection point to connect to
     * @return true if connection could be established, otherwise false
     */
    bool setConnection1(KoShape * shape1, int connectionPointIndex);

    /**
    * Sets the second shape the connector is connected to
    *
    * Passing a null pointer as the first parameter will reset the connection.
    *
    * @param shape2 the shape to connect to or null to reset the connection
    * @param connectionPointIndex2 the index of the connection point to connect to
    * @return true if connection could be established, otherwise false
    */
    bool setConnection2(KoShape * shape2, int connectionPointIndex);

    /// Returns the connection to the first shape
    KoConnection connection1() const;

    /// Returns the connection to the second shape
    KoConnection connection2() const;

    /// Updates connections to shapes
    void updateConnections();

    /// Returns connection type
    Type connectionType() const;

    /// Sets the connection type
    void setConnectionType(Type connectionType);

protected:
    /// reimplemented
    void moveHandleAction(int handleId, const QPointF & point, Qt::KeyboardModifiers modifiers = Qt::NoModifier);

    /// reimplemented
    void updatePath(const QSizeF &size);

    /// Returns if given handle is connected to a shape
    bool handleConnected(int handleId) const;

    /// Returns escape direction of given handle
    QPointF escapeDirection(int handleId) const;

    /// Checks if rays from given points into given directions intersect
    bool intersects(const QPointF &p1, const QPointF &d1, const QPointF &p2, const QPointF &d2, QPointF &isect);

    /// Returns perpendicular direction from given point p1 and direction d1 toward point p2
    QPointF perpendicularDirection(const QPointF &p1, const QPointF &d1, const QPointF &p2);

    /// reimplemented
    virtual void shapeChanged(ChangeType type, KoShape * shape);

    /// Populate the path list by a normal way
    void normalPath( const qreal MinimumEscapeLength );

private:
    qreal scalarProd(const QPointF &v1, const QPointF &v2);
    qreal crossProd(const QPointF &v1, const QPointF &v2);

    QList<QPointF> m_path; // TODO move to d-pointer
    bool m_hasMoved; // TODO move to d-pointer

    class Private;
    Private * const d;
};


#endif
