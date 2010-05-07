/* This file is part of the KDE project
 * Copyright (C) 2006-2010 Thomas Zander <zander@kde.org>
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

#ifndef KOSHAPECONTAINER_H
#define KOSHAPECONTAINER_H

#include "KoShape.h"
#include "KoViewConverter.h"

#include <QList>

#include "flake_export.h"

class QPainter;
class KoShapeContainerModel;
class KoShapeContainerPrivate;

/**
 * This is the base class that all Flake group-shapes are based on.
 * Extending from this class allows you to have child-shapes.
 * Like the KoShape class, this shape is a visible class with
 * a position and a size. It can paint itself as well if you implement
 * the paintComponent() method.
 *
 * <p>The most important feature of this class is that you can make
 * other KoShape classes to be children of this container.
 *
 * <p>The effect of grouping those shapes is that their position
 * is relative to the position of the container. Move the container and
 * all children move with it.
 *
 * <p>Each child can optionally be said to be 'clipped' by the container.
 * This feature will give the effect that if the child has a size and
 * position outside the container, parts outside the container will not be shown.
 * This is especially useful
 * for showing cutouts of content, like images, without changing the actual content.
 *
 * <p>For so called clipped children any modification made to the container is
 * propagated to the child. This includes rotation as well as scaling
 * and shearing.
 *
 * <p>Maintaining the list of children can be done using the supplied methods
 * addChild() and removeChild(). However, they only forward their requests to the
 * data model KoShapeContainerModel and if you provide a custom implementation
 * of that model any means can be used to maintain a list of children, as long as
 * you will take care to register them with the appropriate shape manager.
 *
 * <p>An example usage where a custom model might be useful is when you have a
 * container for text areas which are split into columns.  If you resize the container
 * and the width of the individual columns gets too small, the model can choose to
 * remove a child or add one when the width allows another column.
 */
class FLAKE_EXPORT KoShapeContainer : public KoShape
{
public:

    /**
     * Default constructor; this constructs a container a default model that does what you expect.
     */
    KoShapeContainer();

    /**
     * Constructor with custom model to be used for maintaining the list of children.
     * For all the normal cases you don't need a custom model. Only when you want to respond
     * to moves of the container to do something special, or disable one of the features the
     * container normally has (like clipping).  Use the default constructor in those cases.
     * @param model the custom model to be used for maintaining the list of children.
     */
    explicit KoShapeContainer(KoShapeContainerModel *model);

    /**
     * Destructor for the shape container.
     * All children will be orphaned by calling a KoShape::setParent(0)
     */
    virtual ~KoShapeContainer();

    /**
     * Add a child to this container.
     * @param shape the child to be managed in the container.
     */
    void addShape(KoShape *shape);

    /**
     * Remove a child to be completely separated from the container.
     * @param shape the child to be removed.
     */
    void removeShape(KoShape *shape);

    /**
     * Return the current number of children registered.
     * @return the current number of children registered.
     */
    int shapeCount() const;

    /**
     * Set the argument child to have its 'clipping' property set.
     * @param child the child for which the property will be changed.
     * @param clipping the property
     */
    void setClipped(const KoShape *child, bool clipping);

    /**
     * Returns if the argument child has its 'clipping' property set.
     * @return if the argument child has its 'clipping' property set.
     * @param child the child for which the property will be returned.
     */
    bool isClipped(const KoShape *child) const;

    /**
     * Return wheather the child has the effective state of being locked for user modifications.
     * This method is deferred to the model, which should call the KoShape::isGeometryProtected() on the child.
     * @param child the shape that the user wants to move.
     */
    bool isChildLocked(const KoShape *child) const;

    /// reimplemented
    virtual void paint(QPainter &painter, const KoViewConverter &converter);

    /**
     * @brief Paint the component
     * Implement this method to allow the shape to paint itself, just like the KoShape::paint()
     * method does.
     *
     * @param painter used for painting the shape
     * @param converter to convert between internal and view coordinates.
     * @see applyConversion()
     */
    virtual void paintComponent(QPainter &painter, const KoViewConverter &converter) = 0;

    using KoShape::update;
    /// reimplemented
    virtual void update() const;

    /**
     * Create and return an iterator over all child shapes.
     * @return an interator over all child shapes.
     */
    QList<KoShape*> shapes() const;

    /**
     * return the model for this container
     */
    KoShapeContainerModel *model() const;

protected:
    /**
     * This hook is for inheriting classes that need to do something on adding/removing
     * of children.
     * This method will be called just after the child has been added/removed.
     * The default implementation is empty.
     */
    virtual void shapeCountChanged() { }

    /// constructor
    KoShapeContainer(KoShapeContainerPrivate &);

private:
    void shapeChanged(ChangeType type, KoShape *shape = 0);

    Q_DECLARE_PRIVATE(KoShapeContainer)
};

#endif
