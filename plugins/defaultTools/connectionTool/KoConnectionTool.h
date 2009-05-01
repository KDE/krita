/* This file is part of the KDE project
 *
 * Copyright (C) 2009 Jean-Nicolas Artaud <jeannicolasartaud@gmail.com>
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

#ifndef KO_CONNECTION_TOOL_H
#define KO_CONNECTION_TOOL_H

#define KoConnectionTool_ID "KoConnectionTool"

#include "KoPathTool.h"

#include <KoConnectionShape.h>
#include <KoCanvasBase.h>
class KoConnectionTool : public KoPathTool
{
    Q_OBJECT
public:
    /**
     * @brief Constructor
     */
    explicit KoConnectionTool( KoCanvasBase * canvas );
    /**
     * @brief Destructor
     */
    ~KoConnectionTool();
    
    /// reimplemented from superclass
    virtual void paint( QPainter &painter, const KoViewConverter &converter );

    /// reimplemented from superclass
    virtual void mousePressEvent( KoPointerEvent *event ) ;
    /// reimplemented from superclass
    virtual void mouseMoveEvent( KoPointerEvent *event );
    /// reimplemented from superclass
    virtual void mouseReleaseEvent( KoPointerEvent *event );
    /// reimplemented from superclass
    virtual void activate( bool temporary );
    /// reimplemented from superclass
    virtual void deactivate();
    
    /**
     * @brief Connect on the shape1 which is the nearest point with the second shape 
     *
     * @param shape1 The shape to connect
     * @param shape2 The shape to be nearby
     */
    KoConnection getConnection( KoShape * shape1, KoShape * shape2 );
    /**
     * @brief Connect on the shape1 which is the point 
     *
     * @param shape1 The shape to connect
     * @param point The point to connect
     */
    KoConnection getConnectionPointShape( KoShape * shape1, QPointF * point );
    /**
     * @brief Return the square of the absolute distance between p1 and p2 
     *
     * @param p1 The first point
     * @param p2 The second point
     */
    float distanceSquare( QPointF p1, QPointF p2 );
    /**
     * @brief Return true if the two float are approximativly equal, false else
     *
     * @param x The first float
     * @param y The second float
     */
    bool approx( float x, float y );
    /**
     * @brief Permit to put in m_pointSelected the point on the shape, under the mouse
     *
     * @param shape A KoShape on which we want to select the point
     */
    void getPointSelected(KoShape * shape);
    /**
     * @brief Permit to activate the connection with a comand
     */
    void command();
    
private:
    KoShape * m_firstShape;
    KoShape * m_shapeOn;
    QPointF m_mouse;
    QPointF * m_pointSelected;
    QPointF * m_beginPoint;
    KoConnectionShape * m_connectionShape;
    
};

#endif
