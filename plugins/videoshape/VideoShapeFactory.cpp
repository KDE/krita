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
    : KoShapeFactory(parent, VIDEOSHAPEID, i18n("Video"))
{
    setToolTip(i18n("Video, embedded or fullscreen"));
    setIcon("x-shape-video");
    setOdfElementNames(KoXmlNS::draw, QStringList("video"));
    setLoadingPriority(1);
    m_configWidget = new VideoShapeConfigWidget();
}

KoShape* VideoShapeFactory::createDefaultShape() const
{
    VideoShape * defaultShape = new VideoShape();
    defaultShape->setShapeId(VIDEOSHAPEID);
    return defaultShape;
}

KoShape* VideoShapeFactory::createShape(const KoProperties *params) const
{
    Q_UNUSED(params);
    return createDefaultShape();
}

bool VideoShapeFactory::supports(const KoXmlElement &e) const
{
    return e.localName() == "video" && e.namespaceURI() == KoXmlNS::draw;
}

void VideoShapeFactory::populateDataCenterMap(QMap<QString, KoDataCenter*> &dataCenterMap)
{
    // only add image collection if none exist already
    if (!dataCenterMap.contains("VideoCollection"))
    {
        VideoCollection *videoCol = new VideoCollection();
        dataCenterMap["VideoCollection"] = videoCol;
    }
}

QList<KoShapeConfigWidgetBase*> VideoShapeFactory::createShapeOptionPanels()
{
    QList<KoShapeConfigWidgetBase*> panels;
    panels.append(new VideoShapeConfigWidget());
    return panels;
}
