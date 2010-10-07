/* This file is part of the KDE project
 * Copyright (C) 2006 Thomas Zander <zander@kde.org>
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
#include "KoLineBorder.h"
#include "KoImageCollection.h"
#include "KoResourceManager.h"

#include <klocale.h>

#include <KoXmlReader.h>
#include <KoXmlNS.h>

KoPathShapeFactory::KoPathShapeFactory(const QStringList&)
        : KoShapeFactoryBase(KoPathShapeId, i18n("Simple path shape"))
{
    setToolTip(i18n("A simple path shape"));
    setIcon("pathshape");
    QStringList elementNames;
    elementNames << "path" << "line" << "polyline" << "polygon";
    setOdfElementNames(KoXmlNS::draw, elementNames);
    setLoadingPriority(0);
}

KoShape *KoPathShapeFactory::createDefaultShape(KoResourceManager *) const
{
    KoPathShape* path = new KoPathShape();
    path->moveTo(QPointF(0, 50));
    path->curveTo(QPointF(0, 120), QPointF(50, 120), QPointF(50, 50));
    path->curveTo(QPointF(50, -20), QPointF(100, -20), QPointF(100, 50));
    path->normalize();
    path->setBorder(new KoLineBorder(1.0));
    return path;
}

bool KoPathShapeFactory::supports(const KoXmlElement & e) const
{
    if (e.localName() == "path" && e.namespaceURI() == KoXmlNS::draw)
        return true;
    if (e.localName() == "line" && e.namespaceURI() == KoXmlNS::draw)
        return true;
    if (e.localName() == "polyline" && e.namespaceURI() == KoXmlNS::draw)
        return true;
    if (e.localName() == "polygon" && e.namespaceURI() == KoXmlNS::draw)
        return true;

    return false;
}

void KoPathShapeFactory::newDocumentResourceManager(KoResourceManager *manager)
{
    // as we need an image collection for the pattern background
    // we want to make sure that there is always an image collection
    // added to the data center map, in case the picture shape plugin
    // is not loaded
    if (manager->imageCollection() == 0) {
        KoImageCollection *imgCol = new KoImageCollection(manager);
        manager->setImageCollection(imgCol);
    }
}
