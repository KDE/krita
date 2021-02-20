/* This file is part of the KDE project
 * SPDX-FileCopyrightText: 2006-2010 Thomas Zander <zander@kde.org>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#ifndef KOSHAPECONTAINER_H
#define KOSHAPECONTAINER_H

#include "KoShape.h"

#include <QList>

#include "kritaflake_export.h"

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
class KRITAFLAKE_EXPORT KoShapeContainer : public KoShape
{
public:

    /**
     * Constructor with custom model to be used for maintaining the list of children.
     * For all the normal cases you don't need a custom model. Only when you want to respond
     * to moves of the container to do something special, or disable one of the features the
     * container normally has (like clipping).  Use the default constructor in those cases.
     * @param model the custom model to be used for maintaining the list of children.
     */
    explicit KoShapeContainer(KoShapeContainerModel *model = 0);

    /**
     * Destructor for the shape container.
     * All children will be orphaned by calling a KoShape::setParent(0)
     */
    ~KoShapeContainer() override;

    /**
     * Add a child to this container.
     *
     * This container will NOT take over ownership of the shape. The caller or those creating
     * the shape is responsible to delete it if not needed any longer.
     *
     * @param shape the child to be managed in the container.
     */
    void addShape(KoShape *shape);

    /**
     * Remove a child to be completely separated from the container.
     *
     * The shape will only be removed from this container but not be deleted.
     *
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
     *
     * A shape that is clipped by the container will have its visible portion
     * limited to the area where it intersects with the container.
     * If a shape is positioned or sized such that it would be painted outside
     * of the KoShape::outline() of its parent container, setting this property
     * to true will clip the shape painting to the container outline.
     *
     * @param child the child for which the property will be changed.
     * @param clipping the property
     */
    void setClipped(const KoShape *child, bool clipping);

    /**
     * Returns if the argument child has its 'clipping' property set.
     *
     * A shape that is clipped by the container will have its visible portion
     * limited to the area where it intersects with the container.
     * If a shape is positioned or sized such that it would be painted outside
     * of the KoShape::outline() of its parent container, setting this property
     * to true will clip the shape painting to the container outline.
     *
     * @return if the argument child has its 'clipping' property set.
     * @param child the child for which the property will be returned.
     */
    bool isClipped(const KoShape *child) const;

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
    void setInheritsTransform(const KoShape *shape, bool inherit);

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
    bool inheritsTransform(const KoShape *shape) const;


    /// reimplemented
    void paint(QPainter &painter, KoShapePaintingContext &paintcontext) const override;

    /**
     * @brief Paint the component
     * Implement this method to allow the shape to paint itself, just like the KoShape::paint()
     * method does.
     *
     * @param painter used for painting the shape
     * @param paintcontext the painting context
     * @see applyConversion()
     */
    virtual void paintComponent(QPainter &painter, KoShapePaintingContext &paintcontext) const = 0;

    using KoShape::update;
    /// reimplemented
    void update() const override;

    /**
     * Return the list of all child shapes.
     * @return the list of all child shapes
     */
    QList<KoShape*> shapes() const;

    /**
     * return the model for this container
     */
    KoShapeContainerModel *model() const;

protected:

    /**
     * set the model for this container
     */
    void setModel(KoShapeContainerModel *model);
    /**
     * set the model, and take control of all its children
     */
    void setModelInit(KoShapeContainerModel *model);

public:

    /**
     * A special interface for KoShape to use during setParent call. Don't use
     * these method directly for managing shapes hierarchy! Use shape->setParent()
     * instead.
     */
    struct ShapeInterface {
        ShapeInterface(KoShapeContainer *_q);

        /**
         * Add a child to this container.
         *
         * This container will NOT take over ownership of the shape. The caller or those creating
         * the shape is responsible to delete it if not needed any longer.
         *
         * @param shape the child to be managed in the container.
         */
        void addShape(KoShape *shape);

        /**
         * Remove a child to be completely separated from the container.
         *
         * The shape will only be removed from this container but not be deleted.
         *
         * @param shape the child to be removed.
         */
        void removeShape(KoShape *shape);

    protected:
        KoShapeContainer *q;
    };

    ShapeInterface* shapeInterface();

protected:
    KoShapeContainer(const KoShapeContainer &rhs);

    /**
     * This hook is for inheriting classes that need to do something on adding/removing
     * of children.
     * This method will be called just after the child has been added/removed.
     * The default implementation is empty.
     */
    virtual void shapeCountChanged() { }

    void shapeChanged(ChangeType type, KoShape *shape = 0) override;

private:
    class Private;
    QScopedPointer<Private> d;
};

#endif
