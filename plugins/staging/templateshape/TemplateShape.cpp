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
#include "TemplateShape.h"

// Qt
#include <QPainter>

// KDE
#include <KDebug>

// Calligra
#include <KoViewConverter.h>
#include <KoShapeLoadingContext.h>
#include <KoOdfLoadingContext.h>
#include <KoShapeSavingContext.h>
#include <KoXmlWriter.h>
#include <KoXmlNS.h>
#include <KoStoreDevice.h>
#include <KoGenStyle.h>

// This shape
//#include "Foo.h"


TemplateShape::TemplateShape()
    : QObject()
    , KoShape()
      // , m_member()  // initiate members here.
{
}

TemplateShape::~TemplateShape()
{
}

void TemplateShape::paint(QPainter &painter, const KoViewConverter &converter,
                          KoShapePaintingContext &context)
{
    painter.setPen(QPen(QColor(0, 0, 0)));

    // Example painting code: Draw a rectangle around the shape
    painter.drawRect(converter.documentToView(QRectF(QPoint(0, 0), size())));
}


void TemplateShape::saveOdf(KoShapeSavingContext &context) const
{
    KoXmlWriter &writer = context.xmlWriter();

    // Example code: Save with calligra:template as the top XML element.
    writer.startElement("calligra:template");

    // Save shape attributes that were loaded using loadOdfAttributes.
    saveOdfAttributes(context, OdfAllAttributes);

    writer.endElement(); // calligra:template
}

bool TemplateShape::loadOdf(const KoXmlElement &templateElement, KoShapeLoadingContext &context)
{
    kDebug(31000) << "========================== Starting Template shape";
    kDebug(31000) <<"Loading ODF element: " << templateElement.tagName();

    // Load all standard odf attributes and store into the KoShape
    loadOdfAttributes(templateElement, context, OdfAllAttributes);

    // Template: Load the actual content of the shape here.

    return true;
}


void TemplateShape::waitUntilReady(const KoViewConverter &converter, bool asynchronous) const
{
}


#include <TemplateShape.moc>
