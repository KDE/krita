/* This file is part of the KDE project
 *
 * Copyright (C) 2009 Inge Wallin <inge@lysator.liu.se>
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

// Own
#include "ImageShapeFactory.h"

// ImageShape
#include "ImageShape.h"
//#include "ImageShapeConfigWidget.h"

// Calligra
#include <KoXmlNS.h>
#include <KoShapeLoadingContext.h>
#include <KoOdfLoadingContext.h>
#include <KoIcon.h>

// KDE
#include <klocalizedstring.h>

ImageShapeFactory::ImageShapeFactory()
    : KoShapeFactoryBase(ImageShapeId, i18n("Image shape"))
{
    setToolTip(i18n("A shape that shows an image (PNG/JPG/TIFF)"));
    setIconName(koIconNameCStrNeededWithSubs("a generic image image icon", "x-shape-vectorimage", "application-x-wmf"));

    QList<QPair<QString, QStringList> > elementNamesList;
    elementNamesList.append(qMakePair(QString(KoXmlNS::draw), QStringList("image")));
    elementNamesList.append(qMakePair(QString(KoXmlNS::svg), QStringList("image")));
    setXmlElements(elementNamesList);
    setLoadingPriority(1);
}

KoShape *ImageShapeFactory::createDefaultShape(KoDocumentResourceManager */*documentResources*/) const
{
    ImageShape *shape = new ImageShape();
    shape->setShapeId(ImageShapeId);

    return shape;
}

bool ImageShapeFactory::supports(const KoXmlElement &e, KoShapeLoadingContext &context) const
{
    Q_UNUSED(context);
    return e.localName() == "image" &&
            (e.namespaceURI() == KoXmlNS::draw || e.namespaceURI() == KoXmlNS::svg);
}

QList<KoShapeConfigWidgetBase *> ImageShapeFactory::createShapeOptionPanels()
{
    QList<KoShapeConfigWidgetBase *> result;
    //result.append(new ImageShapeConfigWidget());
    return result;
}
