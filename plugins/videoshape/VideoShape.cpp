/* This file is part of the KDE project
 * Copyright (C) 2006-2007, 2009 Thomas Zander <zander@kde.org>
 * Copyright (C) 2007 Jan Hambrecht <jaham@gmx.net>
 * Copyright (C) 2008 Thorsten Zachmann <zachmann@kde.org>
 * Copyright (C) 2009 Casper Boemann <cbo@boemann.dk>
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

#include "VideoShape.h"

#include <KoViewConverter.h>
#include <VideoEventAction.h>
#include <VideoCollection.h>
#include <VideoData.h>
#include <KoShapeLoadingContext.h>
#include <KoOdfLoadingContext.h>
#include <KoShapeSavingContext.h>
#include <KoXmlWriter.h>
#include <KoXmlNS.h>
#include <KoStoreDevice.h>
#include <KoUnit.h>

#include <QPainter>
#include <kdebug.h>

VideoShape::VideoShape()
    : KoFrameShape(KoXmlNS::draw, "object-ole")
{
    setKeepAspectRatio(true);
    addEventAction(new VideoEventAction(this));
}

VideoShape::~VideoShape()
{
}

void VideoShape::paint(QPainter &painter, const KoViewConverter &converter)
{
    QRectF pixelsF = converter.documentToView(QRectF(QPointF(0,0), size()));
    VideoData *videoData = qobject_cast<VideoData*>(userData());
    if (videoData == 0) {
        painter.fillRect(pixelsF, QColor(Qt::gray));
        return;
    }
}

void VideoShape::saveOdf(KoShapeSavingContext &context) const
{
    // make sure we have a valid image data pointer before saving
    VideoData *videoData = qobject_cast<VideoData*>(userData());
    if (videoData == 0)
        return;

    KoXmlWriter &writer = context.xmlWriter();

    writer.startElement("draw:frame");
    saveOdfAttributes(context, OdfAllAttributes);
    writer.startElement("draw:object-ole");
    // In the spec, only the xlink:href attribute is marked as mandatory, cool :)
    QString name = videoData->tagForSaving(m_videoCollection->saveCounter);
    writer.addAttribute("xlink:type", "simple");
    writer.addAttribute("xlink:show", "embed");
    writer.addAttribute("xlink:actuate", "onRequest");
    writer.addAttribute("xlink:href", name);
    writer.endElement(); // draw:object-ole
    saveOdfCommonChildElements(context);
    writer.endElement(); // draw:frame

    context.addDataCenter(m_videoCollection);
}

bool VideoShape::loadOdf(const KoXmlElement &element, KoShapeLoadingContext &context)
{
    loadOdfAttributes(element, context, OdfAllAttributes);
    return loadOdfFrame(element, context);
}

bool VideoShape::loadOdfFrameElement(const KoXmlElement &element, KoShapeLoadingContext &context)
{
    if (m_videoCollection) {
        const QString href = element.attribute("href");
        // this can happen in case it is a presentation:placeholder
        if (!href.isEmpty()) {
            KoStore *store = context.odfLoadingContext().store();
            VideoData *data = m_videoCollection->createVideoData(href, store);
            setUserData(data);
        }
    }

    return true;
}

VideoCollection *VideoShape::videoCollection() const
{
    return m_videoCollection;
}
