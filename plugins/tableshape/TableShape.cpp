/* This file is part of the KDE project
 * Copyright (C) 2008 Boudewijn Rempt <boud@valdyas.org>
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

#include "TableShape.h"

#include <KoImageData.h>
#include <KoViewConverter.h>
#include <KoImageCollection.h>
#include <KoImageData.h>
#include <KoShapeLoadingContext.h>
#include <KoOasisLoadingContext.h>
#include <KoShapeSavingContext.h>
#include <KoXmlWriter.h>
#include <KoStoreDevice.h>
#include <KoUnit.h>

#include <QPainter>
#include <kdebug.h>

TableShape::TableShape()
    : m_imageData(0)
{
    setKeepAspectRatio(true);
}

TableShape::~TableShape() {
}

void TableShape::paint( QPainter& painter, const KoViewConverter& converter ) {
    if(m_imageData != userData())
        m_imageData = dynamic_cast<KoImageData*> (userData());
    if(m_imageData == 0)
        return;

    QPixmap pm = m_imageData->pixmap();
    QRectF target = converter.documentToView(QRectF(QPointF(0,0), size()));
    painter.drawPixmap(target.toRect(), pm, QRect(0, 0, pm.width(), pm.height()));
}

void TableShape::saveOdf( KoShapeSavingContext & context ) const
{
    // make sure we have a valid image data pointer before saving
    KoImageData * data = m_imageData;
    if( data != userData() )
        data = dynamic_cast<KoImageData*> (userData());
    if(data == 0)
        return;

    KoXmlWriter &writer = context.xmlWriter();

    const bool nestedInFrame = context.isSet(KoShapeSavingContext::FrameOpened);
    if( ! nestedInFrame ) {
        writer.startElement( "draw:frame" );
        saveOdfFrameAttributes(context);
    }
    saveOdfAttributes(context, 0); // required to clear the 'frameOpened' attribute on KoShape

    writer.startElement("draw:image");
    // In the spec, only the xlink:href attribute is marked as mandatory, cool :)
    QString name = context.addImageForSaving( data->pixmap() );
    writer.addAttribute("xlink:href", name);
    writer.endElement();
    if(! nestedInFrame)
        writer.endElement(); // draw-frame
}

bool TableShape::loadOdf( const KoXmlElement & element, KoShapeLoadingContext &context )
{
    Q_UNUSED(context);

    // the frame attributes are loaded outside in the shape registry
    if( context.imageCollection() )
    {
        const QString href = element.attribute("href");

        KoImageData * data = new KoImageData( context.imageCollection() );
        data->setStoreHref( href );
        setUserData( data );
    }

    return true;
}
