/* This file is part of the KDE project
 * Copyright (C) 2010 Carlos Licea <carlos@kdab.com>
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

#include "CommentShapeFactory.h"
#include "CommentShape.h"

#include <klocalizedstring.h>
#include <KoXmlNS.h>
#include <KoDocumentResourceManager.h>
#include <KoDocumentBase.h>
#include <KoShapeLoadingContext.h>

CommentShapeFactory::CommentShapeFactory()
: KoShapeFactoryBase(COMMENTSHAPEID, i18n("Comment"))
{
    setXmlElementNames(KoXmlNS::officeooo, QStringList("annotation"));
    setHidden(true);
}

CommentShapeFactory::~CommentShapeFactory()
{
}


KoShape* CommentShapeFactory::createDefaultShape(KoDocumentResourceManager* documentResources) const
{
    return new CommentShape(documentResources);
}

bool CommentShapeFactory::supports(const KoXmlElement& element, KoShapeLoadingContext &context) const
{
    Q_UNUSED(context);
    return element.localName() == "annotation" && element.namespaceURI() == KoXmlNS::officeooo; //TODO change accordingly
}
