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

#include <KoDocumentResourceManager.h>
#include <KoXmlNS.h>
#include "KoShapeBasedDocumentBase.h"
#include <KoShapeLoadingContext.h>
#include "VideoCollection.h"
#include <KoIcon.h>

#include <klocale.h>
#include <kdebug.h>

VideoShapeFactory::VideoShapeFactory()
    : KoShapeFactoryBase(VIDEOSHAPEID, i18n("Video"))
{
    setToolTip(i18n("Video, embedded or fullscreen"));
    setIconName(koIconNameCStr("video-x-generic"));
    setXmlElementNames(KoXmlNS::draw, QStringList("plugin"));
    setLoadingPriority(2);
}

KoShape *VideoShapeFactory::createDefaultShape(KoDocumentResourceManager *documentResources) const
{
    VideoShape * defaultShape = new VideoShape();
    defaultShape->setShapeId(VIDEOSHAPEID);
    if (documentResources) {
          Q_ASSERT(documentResources->hasResource(VideoCollection::ResourceId));
          QVariant vc = documentResources->resource(VideoCollection::ResourceId);
          defaultShape->setVideoCollection(static_cast<VideoCollection*>(vc.value<void*>()));
    }
    return defaultShape;
}

bool VideoShapeFactory::supports(const KoXmlElement &e, KoShapeLoadingContext &context) const
{
    Q_UNUSED(context);
    if (e.localName() != "plugin" || e.namespaceURI() != KoXmlNS::draw) {
        return false;
    }
    return e.attribute("mime-type") == "application/vnd.sun.star.media";
}

void VideoShapeFactory::newDocumentResourceManager(KoDocumentResourceManager *manager) const
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
