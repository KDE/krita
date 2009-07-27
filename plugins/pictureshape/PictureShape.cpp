/* This file is part of the KDE project
 * Copyright (C) 2006-2007, 2009 Thomas Zander <zander@kde.org>
 * Copyright (C) 2007 Jan Hambrecht <jaham@gmx.net>
 * Copyright (C) 2008 Thorsten Zachmann <zachmann@kde.org>
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

#include <KoViewConverter.h>
#include <KoImageCollection.h>
#include <KoImageData.h>
#include <KoShapeLoadingContext.h>
#include <KoOdfLoadingContext.h>
#include <KoShapeSavingContext.h>
#include <KoXmlWriter.h>
#include <KoXmlNS.h>
#include <KoStoreDevice.h>
#include <KoUnit.h>

#include <QPainter>
#include <kdebug.h>

PictureShape::PictureShape()
    : KoFrameShape(KoXmlNS::draw, "image")
{
    setKeepAspectRatio(true);
}

PictureShape::~PictureShape()
{
}

void PictureShape::paint(QPainter &painter, const KoViewConverter &converter)
{
    QRectF pixels = converter.documentToView(QRectF(QPointF(0,0), size()));

    KoImageData *imageData = qobject_cast<KoImageData*>(userData());
    if (imageData == 0) {
        painter.fillRect(pixels, QColor(Qt::gray));
        return;
    }

    QPixmap pixmap;
    if (! imageData->hasCachedPixmap()) {
        pixmap = imageData->pixmap(pixels.size().toSize());
    } else {
        pixmap = imageData->pixmap();
    }
    // TODO only paint the rect that is visible
    painter.drawPixmap(pixels.toRect(), pixmap, QRect(0, 0, pixmap.width(), pixmap.height()));
}

void PictureShape::saveOdf(KoShapeSavingContext &context) const
{
    // make sure we have a valid image data pointer before saving
    KoImageData *imageData = qobject_cast<KoImageData*>(userData());
    if (imageData == 0)
        return;

    KoXmlWriter &writer = context.xmlWriter();

    writer.startElement("draw:frame");
    saveOdfAttributes(context, OdfAllAttributes);
    writer.startElement("draw:image");
    // In the spec, only the xlink:href attribute is marked as mandatory, cool :)
    QString name = context.imageHref(imageData);
    writer.addAttribute("xlink:type", "simple");
    writer.addAttribute("xlink:show", "embed");
    writer.addAttribute("xlink:actuate", "onLoad");
    writer.addAttribute("xlink:href", name);
    writer.endElement(); // draw:image
    saveOdfCommonChildElements(context);
    writer.endElement(); // draw:frame

    context.addDataCenter(m_imageCollection);
}

bool PictureShape::loadOdf(const KoXmlElement &element, KoShapeLoadingContext &context)
{
    loadOdfAttributes(element, context, OdfAllAttributes);
    return loadOdfFrame(element, context);
}

bool PictureShape::loadOdfFrameElement(const KoXmlElement &element, KoShapeLoadingContext &context)
{
    if (m_imageCollection) {
        const QString href = element.attribute("href");
        // this can happen in case it is a presentation:placeholder
        if (!href.isEmpty()) {
            KoStore *store = context.odfLoadingContext().store();
            KoImageData *data = m_imageCollection->createImageData(href, store);
            setUserData(data);
        } else {
            // check if we have an office:binary data element containing the image data
            const KoXmlElement &binaryData(KoXml::namedItemNS(element, KoXmlNS::office, "binary-data"));
            if (!binaryData.isNull()) {
                QImage image;
                if (image.loadFromData(QByteArray::fromBase64(binaryData.text().toLatin1()))) {
                    KoImageData *data = m_imageCollection->createImageData(image);
                    setUserData(data);
                }
            }
        }
    }

    return true;
}

KoImageCollection *PictureShape::imageCollection() const
{
    return m_imageCollection;
}

void PictureShape::init(const QMap<QString, KoDataCenter*> &dataCenterMap)
{
    m_imageCollection = dynamic_cast<KoImageCollection *>(dataCenterMap.value("ImageCollection"));
}
