/* This file is part of the KDE project
 *
 * SPDX-FileCopyrightText: 2009 Inge Wallin <inge@lysator.liu.se>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

// Own
#include "ImageShapeFactory.h"

// ImageShape
#include "ImageShape.h"
//#include "ImageShapeConfigWidget.h"

// Calligra
#include <KoXmlNS.h>
#include <KoShapeLoadingContext.h>
#include <KoIcon.h>
#include <KoProperties.h>
#include <kis_assert.h>

// KDE
#include <klocalizedstring.h>

ImageShapeFactory::ImageShapeFactory()
    : KoShapeFactoryBase(ImageShapeId, i18n("Image shape"))
{
    setToolTip(i18n("A shape that shows an image (PNG/JPG/TIFF)"));
    setIconName(koIconNameCStrNeededWithSubs("a generic image icon", "x-shape-vectorimage", "application-x-wmf"));

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


KoShape *ImageShapeFactory::createShape(const KoProperties *params, KoDocumentResourceManager */*documentResources*/) const
{
    ImageShape *shape = new ImageShape();
    shape->setShapeId(ImageShapeId);

    QVariant var = params->value("image");

    KIS_SAFE_ASSERT_RECOVER(var.type() == QVariant::Image);
    shape->setImage(var.value<QImage>());

    var = params->value("viewboxTransform");
    if (var.type() == QVariant::Transform) {
        shape->setViewBoxTransform(var.value<QTransform>());
    }
    return shape;
}

bool ImageShapeFactory::supports(const QDomElement &e, KoShapeLoadingContext &context) const
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
