/* This file is part of the KDE project
 *
 * Copyright (C) 2012 Inge Wallin <inge@lysator.liu.se>
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

#ifndef OBJECT3D_H
#define OBJECT3D_H

// Qt
#include <QString>

// Calligra
#include <KoShape.h>


class KoXmlElement;


// This is the base class for the 3D objects that are defined in ODF:
// dr3d:scene, dr3d:cube, dr3d:sphere, dr3d:extrude and
// dr3d:rotate. The top level object is always a dr3d:scene.
//
// We implement these objects as KoShapes because they can all have
// style information, and if we didn't we would have to reimplement
// loading all attributes and styling that KoShape already implements.
//
// However, there is a problem.
//
// All the 3D objects support the draw:layer attribute. Layers are
// implemented as KoShapeContainers and for applications that support
// layers, they are the top level containers in a file. But it is
// nowhere defined that all the objects of a scene need to belong to
// the same layer; they could potentially be put into different
// layers.
//
// Calligra enforces a strict shape hierarchy with a shape only
// belonging to one parent at a time. So if we want to implement the
// dr3d:scene as a KoShapeContainer, we would have to put all the
// objects in the same layer, namely the one that the scene itself
// belongs to. This means that if we want to allow the objects being
// in different layers we cannot implement the scene as a
// KoShapeContainer.  But not doing that brings other problems that
// need workarounds.  Basically we have two choices.
//
// 1. Implement a scene as a KoShapeContainer
//    Advantages:
//     * Saving is handled correctly (see below)
//    Disadvantages:
//     * All objects have to be in the same layer, including the scene.
//
// 2. Implement the scene as a KoShape only and keep track of the
//    objects inside it some other way (in our case a QList).
//    Advantages:
//     * Any shape can belong to any layer
//    Disadvantages:
//     * The shape manager will call saveOdf() for all the shapes, and
//       they will be saved inline in the document contents.  But ODF
//       only allows 3D objects inside a dr3d:scene, so this is
//       invalid XML.  Thus, we have to create an empty saveOdf() and
//       instead introduce a new saving function (saveObjectOdf()) that is
//       only called from the saveOdf() of the top level dr3d:scene
//       object, thus reimplementing some of the features of
//       KoShapeContainer.
//
// During different times in the implementation process we have
// implemented both options. At this time, option 1 is activated. See
// the #define just below.
//

#define IMPLEMENT_AS_SHAPECONTAINER 1

class Object3D // : public KoShape
{
public:
    explicit Object3D(Object3D *parent);
    virtual ~Object3D();

    // getters
    Object3D *parent();
    QString transform();

#if 0
    // reimplemented
    virtual void paint(QPainter &painter, const KoViewConverter &converter,
                       KoShapePaintingContext &context);
#endif
    virtual bool loadOdf(const KoXmlElement &objectElement, KoShapeLoadingContext &context);
    virtual void saveOdf(KoShapeSavingContext &context) const;

    virtual void saveObjectOdf(KoShapeSavingContext &context) const;


protected:
    Object3D *m_parent;
    QString   m_transform3D;    // Must be a real matrix when we implement drawing.
};


#endif
