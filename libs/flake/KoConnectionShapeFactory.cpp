/* This file is part of the KDE project
 *
 * Copyright (C) 2007 Boudewijn Rempt <boud@kde.org>
 * Copyright (C) 2007 Thorsten Zachmann <zachmann@kde.org>
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

#include "KoConnectionShapeFactory.h"

#include "KoConnectionShape.h"
#include "KoConnectionShapeConfigWidget.h"

#include <KoXmlNS.h>
#include <KoIcon.h>
#include <klocale.h>
#include <KoShapeStroke.h>
#include <KoShapeLoadingContext.h>

KoConnectionShapeFactory::KoConnectionShapeFactory()
        : KoShapeFactoryBase(KOCONNECTIONSHAPEID, i18n("Tie"))
{
    setToolTip(i18n("A connection between two other shapes"));
    setIconName(koIconNameCStr("x-shape-connection"));
    setXmlElementNames(KoXmlNS::draw, QStringList("connector"));
    setLoadingPriority(1);
    setHidden(true); // Don't show this shape in collections. Only ConnectionTool should create
}

KoShape* KoConnectionShapeFactory::createDefaultShape(KoDocumentResourceManager *) const
{
    KoConnectionShape * shape = new KoConnectionShape();
    shape->setStroke(new KoShapeStroke());
    shape->setShapeId(KoPathShapeId);
    return shape;
}

bool KoConnectionShapeFactory::supports(const KoXmlElement & e, KoShapeLoadingContext &context) const
{
    Q_UNUSED(context);
    return (e.localName() == "connector" && e.namespaceURI() == KoXmlNS::draw);
}

QList<KoShapeConfigWidgetBase*> KoConnectionShapeFactory::createShapeOptionPanels()
{
    QList<KoShapeConfigWidgetBase*> panels;
    panels.append(new KoConnectionShapeConfigWidget());
    return panels;
}
