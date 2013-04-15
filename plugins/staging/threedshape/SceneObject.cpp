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

// Own
#include "SceneObject.h"

// Qt
#include <QPainter>
#include <QTimer>
#include <QPixmapCache>
#include <QBuffer>

// KDE
#include <kdebug.h>

// Calligra
#include <KoViewConverter.h>
#include <KoImageCollection.h>
#include <KoImageData.h>
#include <KoShapeLoadingContext.h>
#include <KoShapePaintingContext.h>
#include <KoOdfLoadingContext.h>
#include <KoShapeSavingContext.h>
#include <KoXmlWriter.h>
#include <KoXmlNS.h>
#include <KoStoreDevice.h>
#include <KoUnit.h>
#include <KoGenStyle.h>
#include <KoFilterEffectStack.h>

// 3D shape
#include "Objects.h"


SceneObject::SceneObject(Object3D *parent, bool topLevel)
    : Object3D(parent)
#if IMPLEMENT_AS_SHAPECONTAINER
    , KoShapeContainer()
#endif
    , m_topLevel(topLevel)
    , m_threeDParams(0)
{
}

SceneObject::~SceneObject()
{
    delete m_threeDParams;
    qDeleteAll(m_objects);
}

/// reimplemented from KoShapeContainer
void SceneObject::paintComponent(QPainter &painter, const KoViewConverter &converter,
                                 KoShapePaintingContext &paintcontext)
{
    Q_UNUSED(painter);
    Q_UNUSED(converter);
    Q_UNUSED(paintcontext);
}


void SceneObject::paint(QPainter &painter, const KoViewConverter &converter,
                        KoShapePaintingContext &context)
{
    Q_UNUSED(context);

    //painter.setPen(QPen(QColor(172, 196, 206)));
    painter.setPen(QPen(QColor(0, 0, 0)));

#if 1
    painter.drawRect(converter.documentToView(QRectF(QPoint(0, 0), size())));
#else
    QRectF rect = converter.documentToView(boundingRect());
    QRectF rect = converter.documentToView(QRectF(QPoint(0, 0), size()));
    painter.drawRect(rect);
#endif
    kDebug(31000) << "boundingRect: " << boundingRect();
    kDebug(31000) << "outlineRect: " << outlineRect();

#if 0 // Taken from the vector shape
    QRectF  rect(QPointF(0,0), m_size);
    painter.save();

    // Draw a simple cross in a rectangle just to indicate that there is something here.
    painter.setPen(QPen(QColor(172, 196, 206)));
    painter.drawRect(rect);
    painter.drawLine(rect.topLeft(), rect.bottomRight());
    painter.drawLine(rect.bottomLeft(), rect.topRight());

    painter.restore();
#endif
}


void SceneObject::saveOdf(KoShapeSavingContext &context) const
{
    if (m_topLevel) {
        saveObjectOdf(context);
    }
}

void SceneObject::saveObjectOdf(KoShapeSavingContext &context) const
{
    KoXmlWriter &writer = context.xmlWriter();

    writer.startElement("dr3d:scene");
    if (m_topLevel) {
        saveOdfAttributes(context, (OdfAdditionalAttributes | OdfMandatories | OdfGeometry));
    } else {
        saveOdfAttributes(context, (OdfAdditionalAttributes | OdfMandatories));
    }

    if (m_topLevel && m_threeDParams)
        m_threeDParams->saveOdfAttributes(writer);

    // 2.1 Light sources
    if (m_topLevel && m_threeDParams)
        m_threeDParams->saveOdfChildren(writer);

    // 2.2 Objects in the scene
    foreach (const Object3D *object, m_objects) {
        object->saveObjectOdf(context);
    }

    writer.endElement(); // dr3d:scene
}

bool SceneObject::loadOdf(const KoXmlElement &sceneElement, KoShapeLoadingContext &context)
{
    // Load style information.
    if (m_topLevel) {
        loadOdfAttributes(sceneElement, context,
                          (OdfAdditionalAttributes | OdfMandatories | OdfGeometry));
    } else {
        loadOdfAttributes(sceneElement, context,
                          (OdfAdditionalAttributes | OdfMandatories));
    }
    Object3D::loadOdf(sceneElement, context);

    // Load the view parameters.
    if (m_topLevel) {
        m_threeDParams = load3dScene(sceneElement);
    }

    // Load the child elements, i.e the scene itself.

    // From the ODF 1.1 spec section 9.4.1:
    //
    // The elements that may be contained in the <dr3d:scene> element are:
    //  * Title (short accessible name) – see section 9.2.20.
    //  * Long description (in support of accessibility) – see section 9.2.20.
    //  * Light – see section 9.4.2. (handled by Ko3DScene)
    //
    //  * Scene – see section 9.4.1.     [All of these can be 0 or more.]
    //  * Cube – see section 9.4.3.
    //  * Sphere – see section 9.4.4.
    //  * Extrude – see section 9.4.5.
    //  * Rotate – see section 9.4.6.
    //
    // The lights are skipped here, they are taken care of by the call
    // to load3dScene() above.
    KoXmlElement  elem;
    forEachElement(elem, sceneElement) {

        if (elem.localName() == "scene" && elem.namespaceURI() == KoXmlNS::dr3d) {
            SceneObject  *scene = new SceneObject(this, false);
            scene->loadOdf(elem, context);
            m_objects.append(scene);
#if IMPLEMENT_AS_SHAPECONTAINER
            addShape(scene);
#endif

        }
        else if (elem.localName() == "sphere" && elem.namespaceURI() == KoXmlNS::dr3d) {
            Sphere  *sphere = new Sphere(this);
            sphere->loadOdf(elem, context);
            m_objects.append(sphere);
#if IMPLEMENT_AS_SHAPECONTAINER
            addShape(sphere);
#endif
        }
        else if (elem.localName() == "cube" && elem.namespaceURI() == KoXmlNS::dr3d) {
            Cube  *cube = new Cube(this);
            cube->loadOdf(elem, context);
            m_objects.append(cube);
#if IMPLEMENT_AS_SHAPECONTAINER
            addShape(cube);
#endif
        }
        else if (elem.localName() == "extrude" && elem.namespaceURI() == KoXmlNS::dr3d) {
            Extrude  *extrude = new Extrude(this);
            extrude->loadOdf(elem, context);
            m_objects.append(extrude);
#if IMPLEMENT_AS_SHAPECONTAINER
            addShape(extrude);
#endif
        }
        else if (elem.localName() == "rotate" && elem.namespaceURI() == KoXmlNS::dr3d) {
            Rotate  *rotate = new Rotate(this);
            rotate->loadOdf(elem, context);
            m_objects.append(rotate);
#if IMPLEMENT_AS_SHAPECONTAINER
            addShape(rotate);
#endif
        }
    }

    kDebug(31000) << "Objects:" << m_objects.size();
#if IMPLEMENT_AS_SHAPECONTAINER
    kDebug(31000) << "Objects in shape container:" << shapeCount();
#endif

    return true;
}


void SceneObject::waitUntilReady(const KoViewConverter &converter, bool asynchronous) const
{
    Q_UNUSED(converter);
    Q_UNUSED(asynchronous);
}


#include <SceneObject.moc>
