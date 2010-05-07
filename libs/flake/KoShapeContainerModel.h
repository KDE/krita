/* This file is part of the KDE project
 * Copyright (C) 2006-2007 Thomas Zander <zander@kde.org>
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
 * This class has no implementation, but only pure virtual methods. Extending this
 * class and implementing all methods allows you to implement a custom data-backend
 * for the KoShapeContainer.
 * @see KoShapeContainer
 */
class FLAKE_EXPORT KoShapeContainerModel
{
public:
    /// default constructor
    KoShapeContainerModel();

    /// destructor
    virtual ~KoShapeContainerModel();

    /**
     * Add a child to this models store.
     * @param child the child to be managed in the container.
     */
    virtual void add(KoShape *child) = 0;

    /**
     * Remove a child to be completely separated from the model.
     * @param child the child to be removed.
     */
    virtual void remove(KoShape *child) = 0;

    /**
     * Set the argument child to have its 'clipping' property set.
     * @param child the child for which the property will be changed.
     * @param clipping the property; see KoShapeContainerModel for an explenation of what
     *        this bool is for.
     */
    virtual void setClipped(const KoShape *child, bool clipping) = 0;

    /**
     * Returns if the argument child has its 'clipping' property set.
     * See KoShapeContainerModel for an explenation of what this bool is for.
     * @return if the argument child has its 'clipping' property set.
     * @param child the child for which the property will be returned.
     */
    virtual bool isClipped(const KoShape *child) const = 0;

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
     * Create and return an iterator over all child objects.
     * @return an interator over all child objects.
     */
    virtual QList<KoShape*> shapes() const = 0;

    /**
     * This method is called as a notification that one of the properties of the
     * container changed.  This can be one of size, position, rotation and skew.
     * Note that clipped children will automatically get all these changes, the model
     * does not have to do anything for that.
     * @param container the actual container that changed.
     */
    virtual void containerChanged(KoShapeContainer *container) = 0;

    /**
     * This method is called when the user tries to move a shape that is a child of the
     * container this model represents.
     * The child itself is not yet moved; it is proposed to be moved over the param move distance.
     * You can alter the value of the move to affect the actual distance moved.
     * The default implementation does nothing.
     * @param child the child of this container that the user is trying to move.
     * @param move the distance that the user proposes to move child from the current position.
     */
    virtual void proposeMove(KoShape *child, QPointF &move);

    /**
     * This method is called when one of the child shapes has been modified.
     * When a child shape is rotated, moved or scaled/skewed this method will be called
     * to inform the container of such a change.  The change has already happened at the
     * time this method is called.
     * The base implementation notifies the grand parent of the child that there was a
     * change in a child. A reimplentation if this function should call this method when
     * overwriding the function.
     *
     * @param child the child that has been changed
     * @param type this enum shows which change the child has had.
     */
    virtual void childChanged(KoShape *child, KoShape::ChangeType type);
};

#endif
