/* This file is part of the KDE project
 *
 * Copyright (C) 2009 Jean-Nicolas Artaud <jeannicolasartaud@gmail.com>
 * Copyright (C) 2011 Jan Hambrecht <jaham@gmx.net>
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

#define ConnectionTool_ID "ConnectionTool"

#include "KoPathTool.h"

#include <KoConnectionShape.h>
#include <KoCanvasBase.h>
#include <KoSnapGuide.h>

class ConnectionTool : public KoPathTool
{
    Q_OBJECT
public:
    /**
     * @brief Constructor
     */
    explicit ConnectionTool( KoCanvasBase * canvas );
    /**
     * @brief Destructor
     */
    ~ConnectionTool();

    /// reimplemented from superclass
    virtual void paint( QPainter &painter, const KoViewConverter &converter );
    /// reimplemented from superclass
    virtual void repaintDecorations();

    /// reimplemented from superclass
    virtual void mousePressEvent( KoPointerEvent *event ) ;
    /// reimplemented from superclass
    virtual void mouseMoveEvent( KoPointerEvent *event );
    /// reimplemented from superclass
    virtual void mouseReleaseEvent( KoPointerEvent *event );
    /// reimplemented from superclass
    virtual void mouseDoubleClickEvent(KoPointerEvent *event);
    /// reimplemented from superclass
    virtual void keyPressEvent( QKeyEvent *event );
    /// reimplemented from superclass
    virtual void activate(ToolActivation toolActivation, const QSet<KoShape*> &shapes);
    /// reimplemented from superclass
    virtual void deactivate();

private:
    /**
     * @brief Return the square of the absolute distance between p1 and p2
     *
     * @param p1 The first point
     * @param p2 The second point
     * @return The float which is the square of the distance
     */
    qreal squareDistance( const QPointF &p1, const QPointF &p2 );

    /// Returns nearest connection handle or nearest connection point id of shape
    int handleAtPoint(KoShape *shape, const QPointF &mousePoint);

    /// Resets the current edit mode
    void resetEditMode();

    /// Returns the nearest connection shape within handle grab sensitiviy distance
    KoConnectionShape * nearestConnectionShape(QList<KoShape*> shapes, const QPointF &mousePos);

    /// Updates status text depending on edit mode
    void updateStatusText();

    enum EditMode {
        Idle,               ///< we are idle, nothing interesting happens
        CreateConnection,   ///< we are creating a new connection
        EditConnection,     ///< we are editing a connection
        EditConnectionPoint ///< we are editing connection points
    };

    EditMode m_editMode; ///< the current edit mode
    KoShape * m_currentShape; ///< the current shape we are working on
    int m_activeHandle;  ///< the currently active connection point/connection handle
    KoInteractionStrategy *m_currentStrategy; ///< the current editing strategy
    KoSnapGuide::Strategies m_oldSnapStrategies; ///< the previously enables snap strategies
};

#endif // KO_CONNECTION_TOOL_H
