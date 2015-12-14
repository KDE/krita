/* This file is part of the KDE project
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

#ifndef KOENHANCEDPATHHANDLE_H
#define KOENHANCEDPATHHANDLE_H

#include <QPointF>
#include <KoXmlReaderForward.h>

class EnhancedPathShape;
class EnhancedPathParameter;
class KoShapeSavingContext;
class KoShapeLoadingContext;

/**
 * An interaction handle used by the EnhancedPathShape for
 * changing the shape interactively.
 */
class EnhancedPathHandle
{
public:
    /**
     * Constructs a new empty handle;
     *
     * Note that an empty handle is not valid, as long as there are no
     * positional parameters set with setPosition.
    */
    explicit EnhancedPathHandle(EnhancedPathShape *parent);

    /// Destroys the handle
    ~EnhancedPathHandle();

    /**
     * Evaluates the position of the handle.
     * @return the actual handle position
     */
    QPointF position();

    /**
     * Attemps to changes the position of the handle.
     * Only the coordinates of the handle which reference a modifier
     * can be changed. The new position is automatically stored into
     * the modifier of the given enhanced path.
     *
     * @param position the new position the handle to set
     * @param path the enhanced path the handle is referenced from
     */
    void changePosition(const QPointF &position);

    /// Returns if the handle has valid positional parameters.S
    bool hasPosition() const;

    /**
     * Sets the positional parameters, making the handle valid.
     *
     * It replaces the actual positional parameters.
     *
     * @param positionX the x-coordinate of the handle position
     * @param positionY the y-coordinate of the handle position
     */
    void setPosition(EnhancedPathParameter *positionX, EnhancedPathParameter *positionY);

    /**
     * Sets the range of the handles x-coordinate.
     *
     * A zero pointer has the effect of no maximum/minimum value.
     *
     * @param minX the minimum x-coordinate
     * @param maxX the maximum x-coordinate
     */
    void setRangeX(EnhancedPathParameter *minX, EnhancedPathParameter *maxX);

    /**
     * Sets the range of the handles y-coordinate.
     *
     * A zero pointer has the effect of no maximum/minimum value.
     *
     * @param minY the minimum y-coordinate
     * @param maxY the maximum y-coordinate
     */
    void setRangeY(EnhancedPathParameter *minY, EnhancedPathParameter *maxY);

    /**
     * Sets the center of a polar handle.
     *
     * If both parameters are valid pointers, then the handle behaves like
     * a polar handle. This means the x-coordinate of the position represents
     * an angle in degree and the y-coordinate a radius.
     *
     * @param polarX the polar center x-coordinate
     * @param polarY the polar center y-coordinate
     */
    void setPolarCenter(EnhancedPathParameter *polarX, EnhancedPathParameter *polarY);

    /**
     * Sets the range of the radius for polar handles.
     * @param minRadius the minimum polar radius
     * @param maxRadius the maximum polar radius
     */
    void setRadiusRange(EnhancedPathParameter *minRadius, EnhancedPathParameter *maxRadius);

    /// save to the given shape saving context
    void saveOdf(KoShapeSavingContext &context) const;
    /// load handle from given element
    bool loadOdf(const KoXmlElement &element, KoShapeLoadingContext &context);
private:
    /// Returns if handle is polar
    bool isPolar() const;
    EnhancedPathShape *m_parent; ///< the enhanced path shape owning this handle
    EnhancedPathParameter *m_positionX; ///< the position x-coordinate
    EnhancedPathParameter *m_positionY; ///< the position y-coordinate
    EnhancedPathParameter *m_minimumX;  ///< the minimum x-coordinate
    EnhancedPathParameter *m_minimumY;  ///< the minmum y-coordinate
    EnhancedPathParameter *m_maximumX;  ///< the maximum x-coordinate
    EnhancedPathParameter *m_maximumY;  ///< the maximum y-coordinate
    EnhancedPathParameter *m_polarX;    ///< the polar center x-coordinate
    EnhancedPathParameter *m_polarY;    ///< the polar center y-coordinate
    EnhancedPathParameter *m_minRadius; ///< the minimum polar radius
    EnhancedPathParameter *m_maxRadius; ///< the maximum polar radius
};

#endif // KOENHANCEDPATHHANDLE_H
