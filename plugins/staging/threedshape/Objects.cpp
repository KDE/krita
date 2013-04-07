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
#include "Objects.h"

// Qt
#include <QPainter>
#include <QString>

// KDE
#include <kdebug.h>

// Calligra
#include <KoXmlReader.h>
#include <KoXmlNS.h>
#include <KoXmlWriter.h>
#include <KoGenStyle.h>
#include <KoStyleStack.h>
#include <KoShapeSavingContext.h>
#include <KoOdfLoadingContext.h>
#include <KoShapeLoadingContext.h>
#include <KoShapePaintingContext.h>
#include <KoViewConverter.h>

// Shape
#include "utils.h"


//#define OdfObjectAttributes (OdfAllAttributes & ~(OdfGeometry | OdfTransformation))
#define OdfObjectAttributes (OdfAdditionalAttributes | OdfMandatories)

// ================================================================
//                             Sphere


Sphere::Sphere(Object3D *parent)
    : Object3D(parent)
    , KoShape()
{
}

Sphere::~Sphere()
{
}


void Sphere::paint(QPainter &painter, const KoViewConverter &converter,
                   KoShapePaintingContext &context)
{
    Q_UNUSED(painter);
    Q_UNUSED(converter);
    Q_UNUSED(context);
}

bool Sphere::loadOdf(const KoXmlElement &objectElement, KoShapeLoadingContext &context)
{
    // Load style information.
    loadOdfAttributes(objectElement, context, OdfObjectAttributes);
    Object3D::loadOdf(objectElement, context);

    // These strange default values come from the default values in OOo and LO.
    QString dummy;
    dummy = objectElement.attributeNS(KoXmlNS::dr3d, "center", "(0 0 0)");
    m_center = odfToVector3D(dummy);
    dummy = objectElement.attributeNS(KoXmlNS::dr3d, "size", "(5000.0 5000.0 5000.0)");
    m_size = odfToVector3D(dummy);

    kDebug(31000) << "Sphere:" << m_center << m_size;
    return true;
}

void Sphere::saveOdf(KoShapeSavingContext &context) const
{
    Q_UNUSED(context);
}

void Sphere::saveObjectOdf(KoShapeSavingContext &context) const
{
    kDebug(31000) << "Saving Sphere:" << m_center << m_size;

    KoXmlWriter &writer = context.xmlWriter();
    writer.startElement("dr3d:sphere");

    saveOdfAttributes(context, OdfObjectAttributes);
    Object3D::saveObjectOdf(context);

    writer.addAttribute("dr3d:center", QString("(%1 %2 %3)").arg(m_center.x())
                        .arg(m_center.y()).arg(m_center.z()));
    writer.addAttribute("dr3d:size", QString("(%1 %2 %3)").arg(m_size.x())
                        .arg(m_size.y()).arg(m_size.z()));

    writer.endElement(); // dr3d:sphere
}


// ================================================================
//                             Cube


Cube::Cube(Object3D *parent)
    : Object3D(parent)
    , KoShape()
{
}

Cube::~Cube()
{
}


void Cube::paint(QPainter &painter, const KoViewConverter &converter,
                 KoShapePaintingContext &context)
{
    Q_UNUSED(painter);
    Q_UNUSED(converter);
    Q_UNUSED(context);
}

bool Cube::loadOdf(const KoXmlElement &objectElement, KoShapeLoadingContext &context)
{
    // Load style information.
    loadOdfAttributes(objectElement, context, OdfObjectAttributes);
    Object3D::loadOdf(objectElement, context);

    // These strange default values come from the default values in OOo and LO.
    QString dummy;
    dummy = objectElement.attributeNS(KoXmlNS::dr3d, "min-edge", "(-2500.0 -2500.0 -2500.0)");
    m_minEdge = odfToVector3D(dummy);
    dummy = objectElement.attributeNS(KoXmlNS::dr3d, "max-edge", "(2500.0 2500.0 2500.0)");
    m_maxEdge = odfToVector3D(dummy);

    kDebug(31000) << "Cube:" << m_minEdge << m_maxEdge;
    return true;
}

void Cube::saveOdf(KoShapeSavingContext &context) const
{
    Q_UNUSED(context);
}

void Cube::saveObjectOdf(KoShapeSavingContext &context) const
{
    kDebug(31000) << "Saving Cube:" << m_minEdge << m_maxEdge;

    KoXmlWriter &writer = context.xmlWriter();
    writer.startElement("dr3d:cube");

    saveOdfAttributes(context, OdfObjectAttributes);
    Object3D::saveObjectOdf(context);

    writer.addAttribute("dr3d:min-edge", QString("(%1 %2 %3)").arg(m_minEdge.x())
                        .arg(m_minEdge.y()).arg(m_minEdge.z()));
    writer.addAttribute("dr3d:max-edge", QString("(%1 %2 %3)").arg(m_maxEdge.x())
                        .arg(m_maxEdge.y()).arg(m_maxEdge.z()));

    writer.endElement(); // dr3d:cube
}


// ================================================================
//                             Extrude


Extrude::Extrude(Object3D *parent)
    : Object3D(parent)
    , KoShape()
    , m_path()
    , m_depth(1.0)
    , m_closeFront(true)
    , m_closeBack(true)
    , m_backScale(1.0)
{
}

Extrude::~Extrude()
{
}


void Extrude::paint(QPainter &painter, const KoViewConverter &converter,
                    KoShapePaintingContext &context)
{
    Q_UNUSED(painter);
    Q_UNUSED(converter);
    Q_UNUSED(context);
}

bool Extrude::loadOdf(const KoXmlElement &objectElement, KoShapeLoadingContext &context)
{
    // Load style information.
    loadOdfAttributes(objectElement, context, OdfObjectAttributes);
    Object3D::loadOdf(objectElement, context);

    QString dummy;
    m_path = objectElement.attributeNS(KoXmlNS::svg, "d", "");
    m_viewBox = objectElement.attributeNS(KoXmlNS::svg, "viewBox", "");

    kDebug(31000) << "Extrude:" << m_path;
    return true;
}

void Extrude::saveOdf(KoShapeSavingContext &context) const
{
    Q_UNUSED(context);
}

void Extrude::saveObjectOdf(KoShapeSavingContext &context) const
{
    kDebug(31000) << "Saving Extrude:" << m_path;

    KoXmlWriter &writer = context.xmlWriter();
    writer.startElement("dr3d:extrude");

    saveOdfAttributes(context, OdfObjectAttributes);
    Object3D::saveObjectOdf(context);

    writer.addAttribute("svg:d", m_path);
    writer.addAttribute("svg:viewBox", m_viewBox);

    writer.endElement(); // dr3d:extrude
}

void Extrude::loadStyle(const KoXmlElement& element, KoShapeLoadingContext& context)
{
    // Load the common parts of the style.
    KoShape::loadStyle(element, context);

    KoStyleStack &styleStack = context.odfLoadingContext().styleStack();
    styleStack.setTypeProperties("graphic");

    QString dummy;

    if (styleStack.hasProperty(KoXmlNS::dr3d, "depth")) {
        dummy = styleStack.property(KoXmlNS::dr3d, "depth");
        bool ok;
        qreal depth = dummy.toDouble(&ok);
        if (ok) {
            m_depth = depth;
        }
    }

    if (styleStack.hasProperty(KoXmlNS::dr3d, "close-front")) {
        dummy = styleStack.property(KoXmlNS::dr3d, "close-front");
        m_closeFront = (dummy == "true");
    }
    if (styleStack.hasProperty(KoXmlNS::dr3d, "close-back")) {
        dummy = styleStack.property(KoXmlNS::dr3d, "close-back");
        m_closeBack = (dummy == "true");
    }

    if (styleStack.hasProperty(KoXmlNS::dr3d, "back-scale")) {
        dummy = styleStack.property(KoXmlNS::dr3d, "back-scale");
        bool ok;
        qreal bs = dummy.toDouble(&ok);
        if (ok) {
            m_backScale = bs;
        }
    }
}

QString Extrude::saveStyle(KoGenStyle& style, KoShapeSavingContext& context) const
{
    style.addProperty("dr3d:depth", QString("%1").arg(m_depth));

    style.addProperty("dr3d:close-front", m_closeFront);
    style.addProperty("dr3d:close-back",  m_closeBack);

    if (m_backScale != 1.0) {
        style.addProperty("dr3d:back-scale", QString("%1").arg(m_backScale));
    }

    return KoShape::saveStyle(style, context);
}


// ================================================================
//                             Rotate


Rotate::Rotate(Object3D *parent)
    : Object3D(parent)
    , KoShape()
    , m_path()
    , m_horizontalSegments(-1)
    , m_verticalSegments(-1)
    , m_endAngle(360.0)
    , m_closeFront(true)
    , m_closeBack(true)
    , m_backScale(1.0)
{
}

Rotate::~Rotate()
{
}


void Rotate::paint(QPainter &painter, const KoViewConverter &converter,
                   KoShapePaintingContext &context)
{
    Q_UNUSED(painter);
    Q_UNUSED(converter);
    Q_UNUSED(context);
}

bool Rotate::loadOdf(const KoXmlElement &objectElement, KoShapeLoadingContext &context)
{
    // Load style information.
    loadOdfAttributes(objectElement, context, OdfObjectAttributes);
    Object3D::loadOdf(objectElement, context);

    QString dummy;
    m_path = objectElement.attributeNS(KoXmlNS::svg, "d", "");
    m_viewBox = objectElement.attributeNS(KoXmlNS::svg, "viewBox", "");

    kDebug(31000) << "Rotate:" << m_path;
    return true;
}

void Rotate::saveOdf(KoShapeSavingContext &context) const
{
    Q_UNUSED(context);
}

void Rotate::saveObjectOdf(KoShapeSavingContext &context) const
{
    kDebug(31000) << "Saving Rotate:" << m_path;

    KoXmlWriter &writer = context.xmlWriter();
    writer.startElement("dr3d:rotate");

    saveOdfAttributes(context, OdfObjectAttributes);
    Object3D::saveObjectOdf(context);

    writer.addAttribute("svg:d", m_path);
    writer.addAttribute("svg:viewBox", m_viewBox);

    writer.endElement(); // dr3d:rotate
}

void Rotate::loadStyle(const KoXmlElement& element, KoShapeLoadingContext& context)
{
    // Load the common parts of the style.
    KoShape::loadStyle(element, context);

    KoStyleStack &styleStack = context.odfLoadingContext().styleStack();
    styleStack.setTypeProperties("graphic");

    QString dummy;

    if (styleStack.hasProperty(KoXmlNS::dr3d, "horizontal-segments")) {
        dummy = styleStack.property(KoXmlNS::dr3d, "horizontal-segments");
        bool ok;
        int hs = dummy.toInt(&ok);
        if (ok) {
            m_horizontalSegments = hs;
        }
    }
    if (styleStack.hasProperty(KoXmlNS::dr3d, "vertical-segments")) {
        dummy = styleStack.property(KoXmlNS::dr3d, "vertical-segments");
        bool ok;
        int vs = dummy.toInt(&ok);
        if (ok) {
            m_verticalSegments = vs;
        }
    }

    if (styleStack.hasProperty(KoXmlNS::dr3d, "end-angle")) {
        dummy = styleStack.property(KoXmlNS::dr3d, "end-angle");
        bool ok;
        qreal ea = dummy.toDouble(&ok);
        if (ok) {
            m_endAngle = ea;
        }
    }

    if (styleStack.hasProperty(KoXmlNS::dr3d, "close-front")) {
        dummy = styleStack.property(KoXmlNS::dr3d, "close-front");
        m_closeFront = (dummy == "true");
    }
    if (styleStack.hasProperty(KoXmlNS::dr3d, "close-back")) {
        dummy = styleStack.property(KoXmlNS::dr3d, "close-back");
        m_closeBack = (dummy == "true");
    }

    // FIXME: Note that this can be a percentage (our test file has "80%").
    if (styleStack.hasProperty(KoXmlNS::dr3d, "back-scale")) {
        dummy = styleStack.property(KoXmlNS::dr3d, "back-scale");
        bool ok;
        qreal bs = dummy.toDouble(&ok);
        if (ok) {
            m_backScale = bs;
        }
    }
}

QString Rotate::saveStyle(KoGenStyle& style, KoShapeSavingContext& context) const
{
    if (m_horizontalSegments != -1) {
        style.addProperty("dr3d:horizontal-segments", QString("%1").arg(m_horizontalSegments));
    }
    if (m_verticalSegments != -1) {
        style.addProperty("dr3d:vertical-segments", QString("%1").arg(m_verticalSegments));
    }

    if (m_endAngle != 360.0) {
        style.addProperty("dr3d:end-angle", QString("%1").arg(m_endAngle));
    }

    style.addProperty("dr3d:close-front", m_closeFront);
    style.addProperty("dr3d:close-back",  m_closeBack);

    if (m_backScale != 1.0) {
        style.addProperty("dr3d:back-scale", QString("%1").arg(m_backScale));
    }

    return KoShape::saveStyle(style, context);
}
