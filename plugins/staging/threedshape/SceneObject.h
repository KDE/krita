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

#ifndef THREEDSHAPE_H
#define THREEDSHAPE_H

// Qt
#include <QObject>
#include <QList>

// Calligra
#include <Ko3dScene.h>
#include <KoShapeContainer.h>

// Shape
#include "Object3D.h"


#define THREEDSHAPEID "ThreedShape"


class SceneObject : public Object3D //, public QObject
#if IMPLEMENT_AS_SHAPECONTAINER
    , public KoShapeContainer
#else
    , public KoShape
#endif
{
public:
    // Constructor

    // @param topLevel true if this is the top level scene
    // element. The top level element is the only one that should read
    // view parameters from the element.
    SceneObject(Object3D *parent, bool topLevel = false);
    virtual ~SceneObject();

    /// reimplemented from KoShapeContainer
    virtual void paintComponent(QPainter &painter, const KoViewConverter &converter,
                                KoShapePaintingContext &paintcontext);

    // reimplemented from KoShape
    virtual void paint(QPainter &painter, const KoViewConverter &converter,
                       KoShapePaintingContext &context);
    // reimplemented from KoShape
    virtual void saveOdf(KoShapeSavingContext &context) const;
    // reimplemented from KoShape
    virtual bool loadOdf(const KoXmlElement &element, KoShapeLoadingContext &context);

    // reimplemented from KoShape
    virtual void waitUntilReady(const KoViewConverter &converter, bool asynchronous) const;

    // Really save the object.  See the explanation in Object3D.h.
    virtual void saveObjectOdf(KoShapeSavingContext &context) const;

    // getters
    Ko3dScene *threeDParams() const;

private:
    // View attributes
    bool       m_topLevel;
    Ko3dScene *m_threeDParams;    // Camera and rendering parameters plus lightsources.

    QList<Object3D*> m_objects;
};


#endif
