/* This file is part of the KDE project
 * Copyright (C) 2007 Thomas Zander <zander@kde.org>
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
#ifndef KOSHAPECONNECTION_H
#define KOSHAPECONNECTION_H

#include "flake_export.h"

#include <QPointF>
#include <QRectF>

class KoShape;
class QPainter;
class KoViewConverter;

/**
 * The shapeConnection class represents a connection between two shapes.
 * In order to create a visible connection between any two shapes of any kind you can
 * create a new KoShapeConnection passing the two shapes that it connects.
 * Each KoShape instance can have a number of connection points, also called glue points, each
 * of which can be used to start or end a connection.  Consider an shape in the form of a man.
 * You would call KoShape::addConnectionPoint() with a point where his hand is.  If you have a
 * pet with a similarly added connection point adding a connection is a simple case of
 * @code
   new KoShapeConnection(man, 0, dog, 0);
   @endcode
 */
class FLAKE_EXPORT KoShapeConnection {
public:

/* TODO
    enum ConnectionType {
        TautConnection,
        MultiLineConnection,
        CurvedConnection
    };
*/

    /**
     * Constructor for the connection between two shapes.
     * The connection will be added to each of the shapes.  Note that we refer to the gluePoints by index
     * instead of directly accessing the point.  This is done because resizing the shape may alter the actual
     * point, but not the index.
     * @param from is the originating shape
     * @param gluePointIndex1 The point to connect to is found via the index in the list of connectors on the originating shape.
     * @param to is the shape for the endpoint.
     * @param gluePointIndex2 The point to connect to is found via the index in the list of connectors on the end shape.
     */
    explicit KoShapeConnection(KoShape *from, int gluePointIndex1, KoShape *to,  int gluePointIndex2);
    // TODO add a constructor for a shape and a QPointF for a connection between a shape a static-point
    ~KoShapeConnection();

    /**
     * @brief Paint the connection.
     * The connection is painted from start to finish in absolute coordinates. Meaning that the top left
     * of the document is coordinate 0,0.  This in contradiction to shapes.
     * @param painter used for painting the shape
     * @param converter to convert between internal and view coordinates.
     */
    void paint(QPainter &painter, const KoViewConverter &converter);

    /**
     * Return the first shape.
     */
    KoShape *shape1() const;
    /**
     * Return the second shape.
     * Note that this can be 0.
     */
    KoShape *shape2() const;

    /**
     * The z index in which the connection will be drawn.  If the index is higher it will be drawn on top.
     */
    int zIndex() const;
    /**
     * Set the z index in which the connection will be drawn.  If the index is higher it will be drawn on top.
     * @param index the z-index.
     */
    void setZIndex(int index);

    /**
     * Return the gluePointIndex for the originating shape.
     * Note that we refer to the gluePoints by index instead of directly accessing the point.
     * This is done because resizing the shape may alter the actual point, but not the index.
     */
    int gluePointIndex1() const;
    /**
     * Return the gluePointIndex for the end shape.
     * Note that we refer to the gluePoints by index instead of directly accessing the point.
     * This is done because resizing the shape may alter the actual point, but not the index.
     */
    int gluePointIndex2() const;

    /**
     * Calculate and return the absolute point where this connection starts.
     */
    QPointF startPoint() const;
    /**
     * Calculate and return the absolute point where this connection ends.
     */
    QPointF endPoint() const;

    /**
     * This is a method used to sort a list using the STL sorting methods.
     * @param c1 the first connection
     * @param c2 the second connection
     */
    static bool compareConnectionZIndex(KoShapeConnection*c1, KoShapeConnection *c2);

    /**
     * Return a bounding rectangle in which this connection is completely present.
     */
    QRectF boundingRect() const;

private:
    class Private;
    Private * const d;
};

/*
     TODO
    Add a strategy for InteractionTool that when it selects a connection it can be used to change
        the properties or delete it.

    Should we have a way to segment the repaint-rects of the connection?  Now if we have a long connection
      the repaint rect will be huge due to us using the plain bounding rect.  It might be useful to return
      a list of QRectFs which each will be redrawn. Allowing for a substantially smaller repaint area.

    Should I remove a shapeConnection from the ShapeManager on destruction?
*/

#endif
