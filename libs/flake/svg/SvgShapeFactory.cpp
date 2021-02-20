/* This file is part of the KDE project
 * SPDX-FileCopyrightText: 2011 Jan Hambrecht <jaham@gmx.net>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include "SvgShapeFactory.h"
#include "SvgParser.h"
#include "KoShapeGroup.h"
#include "KoShapeGroupCommand.h"
#include "KoShapeLoadingContext.h"
#include "KoShapeRegistry.h"
#include "FlakeDebug.h"
#include <KoXmlNS.h>
#include <KoStore.h>
#include <KoStoreDevice.h>
#include <klocalizedstring.h>

#define SVGSHAPEFACTORYID "SvgShapeFactory"

SvgShapeFactory::SvgShapeFactory()
    : KoShapeFactoryBase(SVGSHAPEFACTORYID, i18n("Embedded svg shape"))
{
    setLoadingPriority(4);
    setXmlElementNames(QString(KoXmlNS::draw), QStringList("image"));
    // hide from add shapes docker as the shape is not able to be dragged onto 
    // the canvas as createDefaultShape returns 0.
    setHidden(true);
}

SvgShapeFactory::~SvgShapeFactory()
{

}

bool SvgShapeFactory::supports(const KoXmlElement &element, KoShapeLoadingContext &context) const
{
    if (element.localName() == "image" && element.namespaceURI() == KoXmlNS::draw) {
        QString href = element.attribute("href");
        if (href.isEmpty())
            return false;

        // check the mimetype
        if (href.startsWith(QLatin1String("./"))) {
            href.remove(0,2);
        }

        QString mimetype = context.mimeTypeForPath(href, true);
        return (mimetype == "image/svg+xml");
    }

    return false;
}

KoShape *SvgShapeFactory::createShapeFromXML(const KoXmlElement &element, KoShapeLoadingContext &context)
{
    const KoXmlElement & imageElement(KoXml::namedItemNS(element, KoXmlNS::draw, "image"));
    if (imageElement.isNull()) {
        errorFlake << "svg image element not found";
        return 0;
    }

    if (imageElement.tagName() == "image") {
        debugFlake << "trying to create shapes form svg image";
        QString href = imageElement.attribute("href");
        if (href.isEmpty())
            return 0;

        // check the mimetype
        if (href.startsWith(QLatin1String("./"))) {
            href.remove(0,2);
        }
        QString mimetype = context.mimeTypeForPath(href);
        debugFlake << mimetype;
        if (mimetype != "image/svg+xml")
            return 0;

        if (!context.store()->open(href))
            return 0;

        KoStoreDevice dev(context.store());
        KoXmlDocument xmlDoc;

        int line, col;
        QString errormessage;

        const bool parsed = xmlDoc.setContent(&dev, &errormessage, &line, &col);

        context.store()->close();

        if (! parsed) {
            errorFlake << "Error while parsing file: "
            << "at line " << line << " column: " << col
            << " message: " << errormessage << endl;
            return 0;
        }

        const int zIndex = calculateZIndex(element, context);


        /**
         * In Krita 3.x we used hardcoded values for shape resolution and font resolution.
         * Override them here explicitly, because ODF-based files can be created only in
         * Krita 3.x.
         *
         * NOTE: don't ask me why they differ...
         */
        const qreal hardcodedImageResolution = 90.0;
        const qreal hardcodedFontResolution = 96.0;

        return createShapeFromSvgDirect(xmlDoc.documentElement(), QRect(0,0,300,300),
                                        hardcodedImageResolution,
                                        hardcodedFontResolution, zIndex, context);
    }

    return 0;
}

int SvgShapeFactory::calculateZIndex(const KoXmlElement &element, KoShapeLoadingContext &context)
{
    int zIndex = 0;

    if (element.hasAttributeNS(KoXmlNS::draw, "z-index")) {
        zIndex = element.attributeNS(KoXmlNS::draw, "z-index").toInt();
    } else {
        zIndex = context.zIndex();
    }

    return zIndex;
}

KoShape *SvgShapeFactory::createShapeFromSvgDirect(const KoXmlElement &root,
                                                   const QRectF &boundsInPixels,
                                                   const qreal pixelsPerInch,
                                                   const qreal forcedFontSizeResolution,
                                                   int zIndex,
                                                   KoShapeLoadingContext &context,
                                                   QSizeF *fragmentSize)
{
    SvgParser parser(context.documentResourceManager());
    parser.setResolution(boundsInPixels, pixelsPerInch);
    parser.setForcedFontSizeResolution(forcedFontSizeResolution);

    QList<KoShape*> shapes = parser.parseSvg(root, fragmentSize);
    if (shapes.isEmpty())
        return 0;

    if (shapes.count() == 1) {
        KoShape *shape = shapes.first();
        shape->setZIndex(zIndex);
        return shape;
    }

    KoShapeGroup *svgGroup = new KoShapeGroup;
    KoShapeGroupCommand cmd(svgGroup, shapes);
    cmd.redo();
    svgGroup->setZIndex(zIndex);

    return svgGroup;
}
