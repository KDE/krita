/* This file is part of the KDE project
 * Copyright (C) 2006-2007, 2010 Thomas Zander <zander@kde.org>
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

#ifndef KOSHAPECONTAINERMODEL_H
#define KOSHAPECONTAINERMODEL_H

#include "flake_export.h"

#include <KoShape.h>

#include <QList>
#include <QPointF>

class KoShapeContainer;

/**
 * The interface for the container model.
 * This class has no implementation, but only pure virtual methods. You can find a
 * fully implemented model using KoShapeContainerDefaultModel.  Extending this
 * class and implementing all methods allows you to implement a custom data-backend
 * for the KoShapeContainer.
 * @see KoShapeContainer, KoShapeContainerDefaultModel
 */
class FLAKE_EXPORT KoShapeContainerModel
{
public:
    /// default constructor
    KoShapeContainerModel();

    /// destructor
    virtual ~KoShapeContainerModel();

    /**
     * Add a shape to this models store.
     * @param shape the shape to be managed in the container.
     */
    virtual void add(KoShape *shape) = 0;

    /**
     * Remove a shape to be completely separated from the model.
     * @param shape the shape to be removed.
     */
    virtual void remove(KoShape *shape) = 0;

    /**
     * Set the argument shape to have its 'clipping' property set.
     *
     * A shape that is clipped by the container will have its visible portion
     * limited to the area where it intersects with the container.
     * If a shape is positioned or sized such that it would be painted outside
     * of the KoShape::outline() of its parent container, setting this property
     * to true will clip the shape painting to the container outline.
     *
     * @param shape the shape for which the property will be changed.
     * @param clipping the new value
     */
    virtual void setClipped(const KoShape *shape, bool clipping) = 0;

    /**
     * Returns if the argument shape has its 'clipping' property set.
     *
     * A shape that is clipped by the container will have its visible portion
     * limited to the area where it intersects with the container.
     * If a shape is positioned or sized such that it would be painted outside
     * of the KoShape::outline() of its parent container, setting this property
     * to true will clip the shape painting to the container outline.
     *
     * @return if the argument shape has its 'clipping' property set.
     * @param shape the shape for which the property will be returned.
     */
    virtual bool isClipped(const KoShape *shape) const = 0;

    /**
     * Set the shape to inherit the container transform.
     *
     * A shape that inherits the transform of the parent container will have its
     * share / rotation / skew etc be calculated as being the product of both its
     * own local transformation and also that of its parent container.
     * If you set this to true and rotate the container, the shape will get that
     * rotation as well automatically.
     *
     * @param shape the shape for which the property will be changed.
     * @param inherit the new value
     */
    virtual void setInheritsTransform(const KoShape *shape, bool inherit) = 0;

    /**
     * Returns if the shape inherits the container transform.
     *
     * A shape that inherits the transform of the parent container will have its
     * share / rotation / skew etc be calculated as being the product of both its
     * own local transformation and also that of its parent container.
     * If you set this to true and rotate the container, the shape will get that
     * rotation as well automatically.
     *
     * @return if the argument shape has its 'inherits transform' property set.
     * @param shape the shape for which the property will be returned.
     */
    virtual bool inheritsTransform(const KoShape *shape) const = 0;

    /**
     * Return wheather the child has the effective state of being locked for user modifications.
     * The model has to call KoShape::isGeometryProtected() and base its return value upon that, it can
     *  additionally find rules on wheather the child is locked based on the container state.
     * @param child the shape that the user wants to move.
     */
    virtual bool isChildLocked(const KoShape *child) const = 0;

    /**
     * Return the current number of children registered.
     * @return the current number of children registered.
     */
    virtual int count() const = 0;

    /**
     * Create and return an iterator over all shapes added to this model
     * @return an interator over all shapes
     */
    virtual QList<KoShape*> shapes() const = 0;

    /**
     * This method is called as a notification that one of the properties of the
     * container changed.  This can be one of size, position, rotation and skew.
     * Note that clipped children will automatically get all these changes, the model
     * does not have to do anything for that.
     * @param container the actual container that changed.
     * @param type this enum shows which change the container has had.
     */
    virtual void containerChanged(KoShapeContainer *container, KoShape::ChangeType type) = 0;

    /**
     * This method is called when the user tries to move a shape that is a shape of the
     * container this model represents.
     * The shape itself is not yet moved; it is proposed to be moved over the param move distance.
     * You can alter the value of the move to affect the actual distance moved.
     * The default implementation does nothing.
     * @param shape the shape of this container that the user is trying to move.
     * @param move the distance that the user proposes to move shape from the current position.
     */
    virtual void proposeMove(KoShape *shape, QPointF &move);

    /**
     * This method is called when one of the shape shapes has been modified.
     * When a shape shape is rotated, moved or scaled/skewed this method will be called
     * to inform the container of such a change.  The change has already happened at the
     * time this method is called.
     * The base implementation notifies the grand parent of the shape that there was a
     * change in a shape. A reimplentation if this function should call this method when
     * overwriding the function.
     *
     * @param shape the shape that has been changed
     * @param type this enum shows which change the shape has had.
     */
    virtual void childChanged(KoShape *shape, KoShape::ChangeType type);
};

#endif
