/* This file is part of the KDE project
 * Copyright (C) 2011 Jan Hambrecht <jaham@gmx.net>
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

#include "SvgShapeFactory.h"
#include "SvgParser.h"
#include "KoShapeGroup.h"
#include "KoShapeGroupCommand.h"
#include "KoShapeLoadingContext.h"
#include "KoShapeRegistry.h"
#include <KoOdfLoadingContext.h>
#include <KoXmlNS.h>
#include <KoStore.h>
#include <KoStoreDevice.h>
#include <KLocale>

#define SVGSHAPEFACTORYID "SvgShapeFactory"

SvgShapeFactory::SvgShapeFactory()
    : KoShapeFactoryBase(SVGSHAPEFACTORYID, i18n("Embedded svg shape"))
{
    setLoadingPriority(10);
    setXmlElementNames(QString(KoXmlNS::draw), QStringList("image"));
    // hide from add shapes docker as the shape is not able to be dragged onto 
    // the canvas as createDefaultShape returns 0.
    setHidden(true);
}

SvgShapeFactory::~SvgShapeFactory()
{

}

void SvgShapeFactory::addToRegistry()
{
    KoShapeRegistry *registry = KoShapeRegistry::instance();
    if (!registry->contains(QString(SVGSHAPEFACTORYID))) {
        registry->addFactory(new SvgShapeFactory);
    }
}

bool SvgShapeFactory::supports(const KoXmlElement &element, KoShapeLoadingContext &context) const
{
    if (element.localName() == "image" && element.namespaceURI() == KoXmlNS::draw) {
        QString href = element.attribute("href");
        if (href.isEmpty())
            return false;

        // check the mimetype
        if (href.startsWith("./")) {
            href.remove(0,2);
        }

        QString mimetype = context.odfLoadingContext().mimeTypeForPath(href, true);
        return (mimetype == "image/svg+xml");
    }

    return false;
}

KoShape *SvgShapeFactory::createShapeFromOdf(const KoXmlElement &element, KoShapeLoadingContext &context)
{
    const KoXmlElement & imageElement(KoXml::namedItemNS(element, KoXmlNS::draw, "image"));
    if (imageElement.isNull()) {
        kError(30006) << "svg image element not found";
        return 0;
    }

    if (imageElement.tagName() == "image") {
        kDebug(30006) << "trying to create shapes form svg image";
        QString href = imageElement.attribute("href");
        if (href.isEmpty())
            return 0;

        // check the mimetype
        if (href.startsWith("./")) {
            href.remove(0,2);
        }
        QString mimetype = context.odfLoadingContext().mimeTypeForPath(href);
        kDebug(30006) << mimetype;
        if (mimetype != "image/svg+xml")
            return 0;

        if (!context.odfLoadingContext().store()->open(href))
            return 0;

        KoStoreDevice dev(context.odfLoadingContext().store());
        KoXmlDocument xmlDoc;

        int line, col;
        QString errormessage;

        const bool parsed = xmlDoc.setContent(&dev, &errormessage, &line, &col);

        context.odfLoadingContext().store()->close();

        if (! parsed) {
            kError(30006) << "Error while parsing file: "
            << "at line " << line << " column: " << col
            << " message: " << errormessage << endl;
            return 0;
        }

        SvgParser parser(context.documentResourceManager());

        QList<KoShape*> shapes = parser.parseSvg(xmlDoc.documentElement());
        if (shapes.isEmpty())
            return 0;
        if (shapes.count() == 1)
            return shapes.first();

        KoShapeGroup *svgGroup = new KoShapeGroup;
        KoShapeGroupCommand cmd(svgGroup, shapes);
        cmd.redo();

        return svgGroup;
    }

    return 0;
}
