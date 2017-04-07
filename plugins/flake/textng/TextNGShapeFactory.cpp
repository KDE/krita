/* This file is part of the KDE project
 *
 * Copyright 2017 Boudewijn Rempt <boud@valdyas.org>
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
#include "TextNGShapeFactory.h"

// TextNGShape
#include "TextNGShape.h"
#include "TextNGShapeConfigWidget.h"

// Calligra
#include <KoXmlNS.h>
#include <KoShapeLoadingContext.h>
#include <KoOdfLoadingContext.h>
#include <KoIcon.h>

// KDE
#include <klocalizedstring.h>

TextNGShapeFactory::TextNGShapeFactory()
    : KoShapeFactoryBase(TextNGShape_SHAPEID, i18n("TextNG image"))
{
    setToolTip(i18n("A shape that shows a textng image (EMF/WMF/SVM)"));
    setIconName(koIconNameCStrNeededWithSubs("a generic textng image icon", "x-shape-textngimage", "application-x-wmf"));
    setXmlElementNames(KoXmlNS::draw, QStringList("image"));
    setLoadingPriority(5);
}

KoShape *TextNGShapeFactory::createDefaultShape(KoDocumentResourceManager */*documentResources*/) const
{
    TextNGShape *shape = new TextNGShape();
    shape->setShapeId(TextNGShape_SHAPEID);

    return shape;
}

bool TextNGShapeFactory::supports(const KoXmlElement &e, KoShapeLoadingContext &Context) const
{
//    if (e.localName() == "image" && e.namespaceURI() == KoXmlNS::draw) {
//        QString href = e.attribute("href");
//        if (!href.isEmpty()) {
//            // check the mimetype
//            if (href.startsWith(QLatin1String("./"))) {
//                href.remove(0, 2);
//            }
//            // LO 3.5 does not write a mimetype for embedded wmf files, so guess also from content
//            const QString mimetype = Context.odfLoadingContext().mimeTypeForPath(href, true);

//            return
//                mimetype == QLatin1String("image/x-svm") ||
//                mimetype == QLatin1String("image/x-emf") ||
//                mimetype == QLatin1String("image/x-wmf") ||
//                // Note: the TextNG Shape supports SVG, but _NOT_ in this method, otherwise it will stomp all over loading the artistic textng shape's svg
//                //mimetype == QLatin1String("image/svg+xml") ||
//                // next three for backward compatibility with Calligra
//                mimetype == QLatin1String("application/x-svm") ||
//                mimetype == QLatin1String("application/x-emf") ||
//                mimetype == QLatin1String("application/x-wmf") ||
//                // seems like MSO does not always write a mimetype
//                // see jeffcoweb.jeffco.k12.co.us%2Fhigh%2Fchatfield%2Fdepartments%2Fbusiness%2Fbanking_finance%2Funit_Plan_Budget.odp
//                mimetype.isEmpty() ||
//                // next for compatibility with OO/LO and our filters
//                // see drwho.virtadpt.net%2Ffiles%2FNOVALUG-Tor.odp
//                mimetype.startsWith("application/x-openoffice");
//        }
//        return true;
//    }

    return false;
}

QList<KoShapeConfigWidgetBase *> TextNGShapeFactory::createShapeOptionPanels()
{
    QList<KoShapeConfigWidgetBase *> result;
    result.append(new TextNGShapeConfigWidget());
    return result;
}
