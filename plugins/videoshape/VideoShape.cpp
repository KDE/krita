/* This file is part of the KDE project
 * Copyright (C) 2006-2007, 2009 Thomas Zander <zander@kde.org>
 * Copyright (C) 2007 Jan Hambrecht <jaham@gmx.net>
 * Copyright (C) 2008 Thorsten Zachmann <zachmann@kde.org>
 * Copyright (C) 2009 C. Boemann <cbo@boemann.dk>
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
#include <KoIcon.h>

#include <kdebug.h>
#include <kurl.h>

#include <QPainter>

VideoShape::VideoShape()
    : KoFrameShape(KoXmlNS::draw, "plugin")
    , m_videoEventAction(new VideoEventAction(this))
    , m_icon(koIconName("video-x-generic"))
{
    setKeepAspectRatio(true);
    addEventAction(m_videoEventAction);
}

VideoShape::~VideoShape()
{
}

void VideoShape::paint(QPainter &painter, const KoViewConverter &converter, KoShapePaintingContext &)
{
    QRectF pixelsF = converter.documentToView(QRectF(QPointF(0,0), size()));

    painter.fillRect(pixelsF, QColor(Qt::gray));
    painter.setPen(QPen());
    painter.drawRect(pixelsF);

    m_icon.paint(&painter, pixelsF.toRect());
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
    writer.startElement("draw:plugin");
    // In the spec, only the xlink:href attribute is marked as mandatory, cool :)
    QUrl storeUrl;//= context.odfSavingContext().store()->urlOfStore();
    QString name = videoData->tagForSaving(storeUrl, m_videoCollection->saveCounter);
    writer.addAttribute("xlink:type", "simple");
    writer.addAttribute("xlink:show", "embed");
    writer.addAttribute("xlink:actuate", "onLoad");
    writer.addAttribute("xlink:href", name);
    writer.addAttribute("draw:mime-type", "application/vnd.sun.star.media");
    writer.endElement(); // draw:plugin
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
    /* the loading of the attributes might set the event actions which removes the m_videoEventAction
     * when there are other eventactions for the shape. Therefore we need to add it again. It is no
     * problem to add it again as internally a set is used and so it is not problematic when it is 
     * already set. */
    addEventAction(m_videoEventAction);

    if (m_videoCollection) {
        const QString href = element.attribute("href");
        // this can happen in case it is a presentation:placeholder
        if (!href.isEmpty()) {
            QUrl url(href);
            VideoData *data=0;

            if(href.startsWith("../")) {
                // file is outside store
                KUrl storePath = context.odfLoadingContext().store()->urlOfStore();
                KUrl extName(storePath, href.mid(3));
                data = m_videoCollection->createExternalVideoData(extName.url());
            } else if(!url.isRelative()) {
                // file is outside store and absolute
                data = m_videoCollection->createExternalVideoData(href);
            } else {
                // file is inside store
                KoStore *store = context.odfLoadingContext().store();
                data = m_videoCollection->createVideoData(href, store);
            }
            setUserData(data);
        }
    }

    return true;
}

VideoCollection *VideoShape::videoCollection() const
{
    return m_videoCollection;
}

void VideoShape::setVideoCollection(VideoCollection *collection)
{
    m_videoCollection = collection;
}
