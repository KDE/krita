/* This file is part of the KDE project
 * Copyright (C) 2006-2007 Thomas Zander <zander@kde.org>
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

#include "PictureShape.h"

#include <KoImageData.h>
#include <KoViewConverter.h>
#include <KoImageCollection.h>
#include <KoImageData.h>
#include <KoShapeLoadingContext.h>
#include <KoOdfLoadingContext.h>
#include <KoShapeSavingContext.h>
#include <KoXmlWriter.h>
#include <KoStoreDevice.h>
#include <KoUnit.h>

#include <QPainter>
#include <kdebug.h>

PictureShape::PictureShape()
    : m_imageData(0)
{
    setKeepAspectRatio(true);
}

PictureShape::~PictureShape() {
}

KoShape * PictureShape::cloneShape() const
{
    // TODO implement cloning
    return 0;
}

void PictureShape::paint( QPainter& painter, const KoViewConverter& converter ) {
    QRectF target = converter.documentToView(QRectF(QPointF(0,0), size()));

    if(m_imageData != userData())
        m_imageData = dynamic_cast<KoImageData*> (userData());

    if(m_imageData == 0)
    {
	painter.fillRect(target, QColor(Qt::gray));
        return;
    }

    QPixmap pm = m_imageData->pixmap();
    painter.drawPixmap(target.toRect(), pm, QRect(0, 0, pm.width(), pm.height()));
}

void PictureShape::saveOdf( KoShapeSavingContext & context ) const
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
    QString name = data->tagForSaving();
    writer.addAttribute("xlink:type", "simple" );
    writer.addAttribute("xlink:show", "embed" );
    writer.addAttribute("xlink:actuate", "onLoad");
    writer.addAttribute("xlink:href", name);
    writer.endElement();
    if(! nestedInFrame)
        writer.endElement(); // draw-frame
}

bool PictureShape::loadOdf( const KoXmlElement & element, KoShapeLoadingContext &context )
{
    Q_UNUSED(context);

    // the frame attributes are loaded outside in the shape registry
    if( m_imageCollection)//context.imageCollection() )
    {
        const QString href = element.attribute("href");
        KoImageData * data = new KoImageData( m_imageCollection, href);
        setUserData( data );
    }

    return true;
}

bool PictureShape::loadFromUrl( KUrl &url )
{
    //This should be changed so that we store the original file somewhere for later saving
    KoImageData * data = new KoImageData( m_imageCollection);
    QImage img(url.path());
    data->setImage(img);
    setUserData( data );
    setSize(QSizeF(DM_TO_POINT(img.width() / (double) img.dotsPerMeterX() * 10.0),
DM_TO_POINT(img.height() / (double) img.dotsPerMeterY() * 10.0)  ));
    return true;
}

void PictureShape::init(QMap<QString, KoDataCenter *>  dataCenterMap)
{
    m_imageCollection = (KoImageCollection *)dataCenterMap["ImageCollection"];
}
