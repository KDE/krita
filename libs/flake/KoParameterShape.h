/* This file is part of the KDE project
   Copyright (C) 2006 Thorsten Zachmann <zachmann@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
*/

#ifndef KOPARAMETERSHAPE_H
#define KOPARAMETERSHAPE_H

#include "KoPathShape.h"

class KoParameterShape : public KoPathShape
{
public:
    KoParameterShape();
    ~KoParameterShape();

    /**
     * @brief Move handle to point
     *
     * This method calls moveHandleAction. Overload moveHandleAction to get the behaviour you want.
     * After that updatePath and a repaint is called.
     *
     * @param handleId of the handle
     * @param point to move the handle to
     * @param modifiers used during move to point
     */
    virtual void moveHandle( int handleId, const QPointF & point, Qt::KeyboardModifiers modifiers = Qt::NoModifier );

    /**
     * @brief Get the handleId in the rect
     *
     * @param rect in shape coordinates
     * @return id of the found handle or -1 if one was found
     */
    virtual int handleIdAt( const QRectF & rect ) const;

    /**
     * @brief Get the handle position 
     * 
     * @param handleId for which ti get the position in shape coordinates
     */
    virtual QPointF handlePosition( int handleId );

    /**
     * @brief Paint the handles
     *
     * @param painter
     * @param converter
     */
    virtual void paintHandles( QPainter & painter, const KoViewConverter & converter );

    /**
     * @brief Paint the given handles
     *
     * @param painter
     * @param converter
     * @param handleId of the handle which should be repainted
     */
    virtual void paintHandle( QPainter & painter, const KoViewConverter & converter, int handleId );

    virtual void resize( const QSizeF &newSize );

    /**
     * @brief Check if object is a parametric shape 
     *
     * It is no longer a parametric shape when the path was manipulated 
     *
     * @return true if it is a parametic shape, false otherwise
     */
    bool isParametricShape() const { return !m_modified; }

    /**
     * @brief Set the modified status.
     * 
     * After the state is set to modified it is no longer possible to work 
     * with parameters on this shape.
     *
     * @param modified the modification state
     */
    void setModified( bool modified ) { m_modified = modified; }

protected:
    /**
     * @brief Updates the internal state of a KoParameterShape.
     *
     * This method is called from moveHandle.
     *
     * @param handleId of the handle
     * @param point to move the handle to
     * @param modifiers used during move to point
     */
    virtual void moveHandleAction( int handleId, const QPointF & point, Qt::KeyboardModifiers modifiers = Qt::NoModifier ) = 0;

    /**
     * @brief Update the path of the parameter shape
     *
     * @param size of the shape
     */
    virtual void updatePath( const QSizeF &size ) = 0;

    QList<QPointF> m_handles;
    bool m_modified;
};

#endif /* KOPARAMETERSHAPE_H */
