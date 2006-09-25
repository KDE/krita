/* This file is part of the KDE project
 * Copyright (C) 2006 Thomas Zander <zander@kde.org>
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

#ifndef KOGRAPHICBASECONTAINER_H
#define KOGRAPHICBASECONTAINER_H

#include "KoShape.h"
#include "KoViewConverter.h"

#include <QList>

#include <koffice_export.h>

class QPainter;
class QPointF;

/**
 * The interface for the container model.
 * This class has no implementation, but only pure virtual methods. Extending this
 * class and implementing all methods allows you to implement a custom data-backend
 * for the KoShapeContainer.
 * @see KoShapeContainer
 */
class FLAKE_EXPORT KoGraphicsContainerModel {
public:
    /// default constructor
    KoGraphicsContainerModel() {} ;

    /// destructor
    virtual ~KoGraphicsContainerModel() {} ;

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
     * @param clipping the property; see KoGraphicsContainerModel for an explenation of what
     *        this bool is for.
     */
    virtual void setClipping(const KoShape *child, bool clipping) = 0;

    /**
     * Returns if the argument child has its 'clipping' property set.
     * See KoGraphicsContainerModel for an explenation of what this bool is for.
     * @return if the argument child has its 'clipping' property set.
     * @param child the child for which the property will be returned.
     */
    virtual bool childClipped(const KoShape *child) const = 0;

    /**
     * Return the current number of children registered.
     * @return the current number of children registered.
     */
    virtual int count() const = 0;

    /**
     * Create and return an iterator over all child objects.
     * @return an interator over all child objects.
     */
    virtual QList<KoShape*> iterator() const = 0;

    /**
     * This method is called as a notification that one of the properties of the
     * container changed.  This can be one of size, position, rotation and skew.
     * Note that clipped children will automatically get all these changes, the model
     * does not have to do anything for that.
     * @param container the actual container that changed.
     */
    virtual void containerChanged(KoShapeContainer *container) = 0;
};

/**
 * This is the base class that all Flake group-objects are based on.
 * Extending from this class allows you to have child-objects.
 * Like the KoShape class, this object is a visible class with
 * a position and a size. It can paint itself as well if you implement
 * the paintComponent() method.
 * <p>The most important feature of this class is that you can make
 * other KoShape classes to be children of this container.
 * <p>The effect of grouping those objects is that their position
 * is relative to the position of the container. Move the container and
 * all children move with it.
 * <p>Each child can optionally be said to be 'clipped' by the container.
 * This feature will give the effect that even if the child has a size and
 * position outside the container, it will not be shown.  This is especially useful
 * for showing cutouts of content, like images, without changing the actual content.
 * <p>For so called clipped children any modification made to the container is
 * propagated to the child. This includes rotation as well as scaling and shearing.
 * <p>Maintaining the list of children can be done using the supplied methods
 * addChild() and removeChild(). However, they only forward their requests to the
 * data model KoGraphicsContainerModel and if you provide a custom implementation
 * of that model any means can be used to maintain a list of children, as long as
 * you will take care to register them with the appropriate object manager.
 * <p>An example usage where a custom model might be useful is when you have a
 * container for text areas which are split into columns.  If you resize the container
 * and the width of the individual columns gets too small, the model can choose to
 * remove a child or add one when the width allows another column.
 */
class FLAKE_EXPORT KoShapeContainer : public KoShape {
public:
    /**
     * Default constructor; uses simple internal model.
     */
    KoShapeContainer();
    /**
     * Constructor with custom model to be used for maintaining the list of children.
     * @param model the custom model to be used for maintaining the list of children.
     */
    KoShapeContainer(KoGraphicsContainerModel *model);

    /// destructor
    virtual ~KoShapeContainer();

    /**
     * Add a child to this container.
     * @param object the child to be managed in the container.
     */
    void addChild(KoShape *object);

    /**
     * Remove a child to be completely separated from the container.
     * @param object the child to be removed.
     */
    void removeChild(KoShape *object);

    /**
     * Return the current number of children registered.
     * @return the current number of children registered.
     */
    int childCount() const;

    /**
     * Set the argument child to have its 'clipping' property set.
     * @param child the child for which the property will be changed.
     * @param clipping the property
     */
    void setClipping(const KoShape *child, bool clipping);

    /**
     * Returns if the argument child has its 'clipping' property set.
     * @return if the argument child has its 'clipping' property set.
     * @param child the child for which the property will be returned.
     */
    bool childClipped(const KoShape *child) const;

    void paint(QPainter &painter, const KoViewConverter &converter);

    /**
     * @brief Paint the component
     * Implement this method to allow the object to paint itself, just like the KoShape::paint()
     * method does.
     *
     * @param painter used for painting the object
     * @param converter to convert between internal and view coordinates.
     * @see applyConversion()
     */
    virtual void paintComponent(QPainter &painter, const KoViewConverter &converter) = 0;

    void repaint() const;

    /**
     * Create and return an iterator over all child objects.
     * @return an interator over all child objects.
     */
    QList<KoShape*> iterator() const;

protected:
    /**
     * This hook is for inheriting classes that need to do something on adding/removing
     * of children.
     * This method will be called just after the child has been added/removed.
     * The default implementation is empty.
     */
    virtual void childCountChanged() { }

private:
    /**
     */
    class ChildrenData : public KoGraphicsContainerModel {
        public:
            ChildrenData();
            ~ChildrenData();
            void add(KoShape *child);
            void setClipping(const KoShape *child, bool clipping);
            bool childClipped(const KoShape *child) const;
            void remove(KoShape *child);
            int count() const;
            QList<KoShape*> iterator() const;
            void containerChanged(KoShapeContainer *container);

        private:
            /**
             * This class is a simple data-storage class for Relation objects.
             */
            class Relation {
                public:
                    Relation(KoShape *child);
                    KoShape* child() { return m_child; }
                    bool m_inside; ///< if true, the child will be clipped by the parent.
                private:
                    KoShape *m_child;
            };

            Relation* findRelation(const KoShape *child) const;

        private: // members
            QList <Relation *> m_relations;
    };
    void shapeChanged(ChangeType type);

private: // members
    KoGraphicsContainerModel *m_children;
};

#endif
