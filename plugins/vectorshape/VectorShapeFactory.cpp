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
#include "VectorShapeFactory.h"

// VectorShape
#include "VectorShape.h"

// Calligra
#include <KoXmlNS.h>
#include <KoShapeLoadingContext.h>
#include <KoOdfLoadingContext.h>


// KDE
#include <klocale.h>
#include <KDebug>


VectorShapeFactory::VectorShapeFactory()
    : KoShapeFactoryBase(VectorShape_SHAPEID, i18n("Vector image"))
{
    setToolTip(i18n("A shape that shows a vector image"));
    setIcon( "vector-shape" );
    setXmlElementNames(KoXmlNS::draw, QStringList("image"));
    setLoadingPriority(2);
}

KoShape *VectorShapeFactory::createDefaultShape(KoDocumentResourceManager */*documentResources*/) const
{
    VectorShape *shape = new VectorShape();
    shape->setShapeId(VectorShape_SHAPEID);

    return shape;
}

bool VectorShapeFactory::supports(const KoXmlElement & e, KoShapeLoadingContext &context) const
{
    if (e.localName() == "image" && e.namespaceURI() == KoXmlNS::draw) {
        QString href = e.attribute("href");
        if (!href.isEmpty()) {
            // check the mimetype
            if (href.startsWith("./")) {
                href.remove(0,2);
            }
            QString mimetype = context.odfLoadingContext().mimeTypeForPath(href);
            // don't try to load image types
            return !mimetype.startsWith("image");
        }
        return true;
    }

    return false;
}

QList<KoShapeConfigWidgetBase*> VectorShapeFactory::createShapeOptionPanels()
{
    QList<KoShapeConfigWidgetBase*> result;

    return result;
}
