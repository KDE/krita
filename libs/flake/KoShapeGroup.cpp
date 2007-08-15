/* This file is part of the KDE project
 * Copyright (C) 2006 Thomas Zander <zander@kde.org>
 * Copyright (C) 2007 Jan Hambrecht <jaham@gmx.net>
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

#include "KoShapeGroup.h"
#include "KoShapeContainerModel.h"
#include "SimpleShapeContainerModel.h"
#include "KoShapeSavingContext.h"
#include "KoShapeLoadingContext.h"
#include "KoXmlWriter.h"
#include "KoXmlReader.h"
#include "KoShapeRegistry.h"

#include <QPainter>

KoShapeGroup::KoShapeGroup()
: KoShapeContainer(new SimpleShapeContainerModel())
{
}

void KoShapeGroup::paintComponent(QPainter &painter, const KoViewConverter &converter) {
    Q_UNUSED(painter);
    Q_UNUSED(converter);
}

bool KoShapeGroup::hitTest( const QPointF &position ) const {
    Q_UNUSED(position);
    return false;
}

void KoShapeGroup::childCountChanged() {
    QRectF br = boundingRect();
    setAbsolutePosition( br.topLeft(), KoFlake::TopLeftCorner );
    setSize( br.size() );
}

void KoShapeGroup::saveOdf( KoShapeSavingContext & context ) const {
    context.xmlWriter().startElement( "draw:g" );
    saveOdfAttributes(context, OdfMandatories);
    context.xmlWriter().addAttributePt( "svg:y", position().y() );

    foreach(KoShape* shape, iterator()) // store children.
        shape->saveOdf(context);

    saveOdfConnections(context);
    context.xmlWriter().endElement();
}

bool KoShapeGroup::loadOdf( const KoXmlElement & element, KoShapeLoadingContext &context ) {
    KoXmlElement child;
    forEachElement( child, element )
    {
        KoShape * shape = KoShapeRegistry::instance()->createShapeFromOdf( child, context );
        if( shape )
        {
            addChild( shape );
            // add shape to context for later adding to the document
            context.addShapeToDocument( shape );
        }
    }

    loadOdfAttributes( element, context, OdfMandatories );

    QRectF bound;
    bool boundInitialized = false;
    foreach( KoShape * shape, iterator() )
    {
        if( ! boundInitialized )
            bound = shape->boundingRect();
        else
            bound = bound.united( shape->boundingRect() );
    }

    setSize( bound.size() );
    setPosition( bound.topLeft() );

    foreach( KoShape * shape, iterator() )
        shape->setAbsolutePosition( shape->absolutePosition() - bound.topLeft() );

    return true;
}
