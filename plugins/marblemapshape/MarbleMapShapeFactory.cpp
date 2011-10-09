/*  Part of Calligra Suite - Marble Map Shape
    Copyright (C) 2007 Thomas Zander <zander@kde.org>
    Copyright (C) 2007 Jan Hambrecht <jaham@gmx.net>
    Copyright (C) 2008 Simon Schmeißer <mail_to_wrt@gmx.de>
    Copyright (C) 2011  Radosław Wicik <radoslaw@wicik.pl>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/


#include "MarbleMapShapeFactory.h"
#include "MarbleMapShape.h"
#include <KoXmlNS.h>
#include <KoDocumentResourceManager.h>
#include <KoShapeLoadingContext.h>

#include <KLocale>
#include <QStringList>

MarbleMapShapeFactory::MarbleMapShapeFactory()
    :KoShapeFactoryBase(MARBLEMAPSHAPEID, i18n("Marble Map Shape"))
{
    setToolTip(i18n("A shape which displays Marble map"));
    setIcon("marble");
    setOdfElementNames(KoXmlNS::calligra, QStringList("marblemap"));
    setLoadingPriority(1);
}

MarbleMapShapeFactory::~MarbleMapShapeFactory()
{

}


bool MarbleMapShapeFactory::supports(const KoXmlElement& element, KoShapeLoadingContext& context) const
{
    Q_UNUSED(context)
    return (element.localName() == "marblemap"
            && element.namespaceURI() == KoXmlNS::calligra);
}

KoShape* MarbleMapShapeFactory::createDefaultShape(KoDocumentResourceManager* documentResources) const
{
    MarbleMapShape *shape = new MarbleMapShape();
    shape->setShapeId(MARBLEMAPSHAPEID);
    if(documentResources)
        shape->setImageCollection(documentResources->imageCollection());
    return shape;
}

