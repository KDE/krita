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

#include "PictureShapeFactory.h"

#include "PictureShape.h"
#include "PictureShapeConfigWidget.h"

#include <KoXmlNS.h>
#include "KoShapeControllerBase.h"
#include <KoShapeLoadingContext.h>
#include "KoImageCollection.h"

#include <klocale.h>
#include <kdebug.h>

PictureShapeFactory::PictureShapeFactory(QObject *parent)
    : KoShapeFactoryBase(parent, PICTURESHAPEID, i18n("Image"))
{
    setToolTip(i18n("Image shape that can display jpg, png etc."));
    setIcon("x-shape-image");
    setOdfElementNames(KoXmlNS::draw, QStringList("image"));
    setLoadingPriority(1);
}

KoShape *PictureShapeFactory::createDefaultShape(KoResourceManager *documentResources) const
{
    PictureShape * defaultShape = new PictureShape();
    defaultShape->setShapeId(PICTURESHAPEID);
    if (documentResources) {
        defaultShape->setImageCollection(documentResources->imageCollection());
    }
    return defaultShape;
}

bool PictureShapeFactory::supports(const KoXmlElement &e, KoShapeLoadingContext &context) const
{
    Q_UNUSED(e);
    return e.localName() == "image" && e.namespaceURI() == KoXmlNS::draw;
}

QList<KoShapeConfigWidgetBase*> PictureShapeFactory::createShapeOptionPanels()
{
    QList<KoShapeConfigWidgetBase*> panels;
    panels.append( new PictureShapeConfigWidget() );
    return panels;
}

void PictureShapeFactory::newDocumentResourceManager(KoResourceManager *manager)
{
    if (!manager->imageCollection())
        manager->setImageCollection(new KoImageCollection(manager));
}
