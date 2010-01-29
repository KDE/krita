/* This file is part of the KDE project
 * Copyright (C) 2007 Thomas Zander <zander@kde.org>
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

#include "VideoShapeFactory.h"

#include "VideoShape.h"
#include "VideoShapeConfigWidget.h"

#include <KoXmlNS.h>
#include "KoShapeControllerBase.h"
#include "VideoCollection.h"

#include <klocale.h>
#include <kdebug.h>

VideoShapeFactory::VideoShapeFactory(QObject *parent)
    : KoShapeFactoryBase(parent, VIDEOSHAPEID, i18n("Video"))
{
    setToolTip(i18n("Video, embedded or fullscreen"));
    setIcon("x-shape-video");
    setOdfElementNames(KoXmlNS::draw, QStringList("video"));
    setLoadingPriority(1);
}

KoShape *VideoShapeFactory::createDefaultShape(KoResourceManager *documentResources) const
{
    VideoShape * defaultShape = new VideoShape();
    defaultShape->setShapeId(VIDEOSHAPEID);
    Q_ASSERT(documentResources->hasResource(VideoCollection::ResourceId));
    QVariant vc = documentResources->resource(VideoCollection::ResourceId);
    defaultShape->setVideoCollection(static_cast<VideoCollection*>(vc.value<void*>()));
    return defaultShape;
}

bool VideoShapeFactory::supports(const KoXmlElement &e) const
{
    return e.localName() == "video" && e.namespaceURI() == KoXmlNS::draw;
}

void VideoShapeFactory::newDocumentResourceManager(KoResourceManager *manager)
{
    QVariant variant;
    variant.setValue<void*>(new VideoCollection(manager));
    manager->setResource(VideoCollection::ResourceId, variant);
}

QList<KoShapeConfigWidgetBase*> VideoShapeFactory::createShapeOptionPanels()
{
    QList<KoShapeConfigWidgetBase*> panels;
    panels.append(new VideoShapeConfigWidget());
    return panels;
}
