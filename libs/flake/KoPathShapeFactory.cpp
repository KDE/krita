/* This file is part of the KDE project
 * SPDX-FileCopyrightText: 2006 Thomas Zander <zander@kde.org>
 * SPDX-FileCopyrightText: 2011 Thorsten Zachmann <zachmann@kde.org>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */
#include "KoPathShapeFactory.h"
#include "KoPathShape.h"
#include "KoShapeStroke.h"
#include "KoImageCollection.h"
#include "KoMarkerCollection.h"
#include "KoDocumentResourceManager.h"
#include "KoShapeLoadingContext.h"
#include <KoIcon.h>
#include "KoInsets.h"

#include <klocalizedstring.h>

#include <KoXmlReader.h>
#include <KoXmlNS.h>

#include "kis_pointer_utils.h"

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
    path->setStroke(toQShared(new KoShapeStroke(1.0)));
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
        manager->setResource(KoDocumentResourceManager::MarkerCollection, QVariant::fromValue(markerCollection));
    }
}
