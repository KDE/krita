/* This file is part of the KDE project
   Copyright (C) 2006 Jan Hambrecht <jaham@gmx.net>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
*/

#include "KoShapeLayer.h"
#include "SimpleShapeContainerModel.h"
#include "KoShapeSavingContext.h"
#include "KoXmlWriter.h"

KoShapeLayer::KoShapeLayer()
: KoShapeContainer(new SimpleShapeContainerModel())
{
    setSelectable(false);
}

bool KoShapeLayer::hitTest( const QPointF &position ) const
{
    Q_UNUSED(position);
    return false;
}

QRectF KoShapeLayer::boundingRect() const
{
    QRectF bb;

    foreach( KoShape* shape, iterator() )
    {
        if(bb.isEmpty())
             bb = shape->boundingRect();
        else
            bb = bb.unite( shape->boundingRect() );
    }

    return bb;
}

void KoShapeLayer::saveOdf( KoShapeSavingContext * context ) {
    // save according to parag 9.1.3
    context->xmlWriter().startElement( "draw:layer" );
    context->xmlWriter().startElement( "svg:title" );
    context->xmlWriter().addTextNode(name());
    context->xmlWriter().endElement();
    context->xmlWriter().endElement();

    foreach(KoShape* shape, iterator())
        shape->saveOdf(context);
}
