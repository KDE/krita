/* This file is part of the KDE project
 * Copyright (C) 2006 Thomas Zander <zander@kde.org>
 * Copyright (C) 2011 Thorsten Zachmann <zachmann@kde.org>
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
#include "KoPathShapeFactory.h"
#include "KoPathShape.h"
#include "KoShapeStroke.h"
#include "KoImageCollection.h"
#include "KoMarkerCollection.h"
#include "KoDocumentResourceManager.h"
#include "KoShapeLoadingContext.h"
#include <KoIcon.h>

#include <klocale.h>

#include <KoXmlReader.h>
#include <KoXmlNS.h>

KoPathShapeFactory::KoPathShapeFactory(const QStringList&)
        : KoShapeFactoryBase(KoPathShapeId, i18n("Simple path shape"))
{
    setToolTip(i18n("A simple path shape"));
    setIconName(koIconNameCStr("pathshape"));
    QStringList elementNames;
    elementNames << "path" << "line" << "polyline" << "polygon";
    setXmlElementNames(KoXmlNS::draw, elementNames);
    setLoadingPriority(0);
}

KoShape *KoPathShapeFactory::createDefaultShape(KoDocumentResourceManager *) const
{
    KoPathShape* path = new KoPathShape();
    path->moveTo(QPointF(0, 50));
    path->curveTo(QPointF(0, 120), QPointF(50, 120), QPointF(50, 50));
    path->curveTo(QPointF(50, -20), QPointF(100, -20), QPointF(100, 50));
    path->normalize();
    path->setStroke(new KoShapeStroke(1.0));
    return path;
}

bool KoPathShapeFactory::supports(const KoXmlElement & e, KoShapeLoadingContext &context) const
{
    Q_UNUSED(context);
    if (e.namespaceURI() == KoXmlNS::draw) {
        if (e.localName() == "path")
            return true;
        if (e.localName() == "line")
            return true;
        if (e.localName() == "polyline")
            return true;
        if (e.localName() == "polygon")
            return true;
    }

    return false;
}

void KoPathShapeFactory::newDocumentResourceManager(KoDocumentResourceManager *manager) const
{
    // as we need an image collection for the pattern background
    // we want to make sure that there is always an image collection
    // added to the data center map, in case the picture shape plugin
    // is not loaded
    if (manager->imageCollection() == 0) {
        KoImageCollection *imgCol = new KoImageCollection(manager);
        manager->setImageCollection(imgCol);
    }
    // we also need a MarkerCollection so add if it is not there yet
    if (!manager->hasResource(KoDocumentResourceManager::MarkerCollection)) {
        KoMarkerCollection *markerCollection = new KoMarkerCollection(manager);
        manager->setResource(KoDocumentResourceManager::MarkerCollection, qVariantFromValue(markerCollection));
    }
}
