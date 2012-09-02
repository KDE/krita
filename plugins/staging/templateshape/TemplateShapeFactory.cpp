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
#include "TemplateShapeFactory.h"

// Qt
#include <QList>

// KDE
#include <klocale.h>
#include <kdebug.h>

// Calligra
#include <KoXmlNS.h>
#include <KoShapeLoadingContext.h>
#include <KoOdfLoadingContext.h>
#include <KoDocumentResourceManager.h>
#include <KoProperties.h>
#include <KoIcon.h>

// This shape
#include "TemplateShape.h"
//#include "TemplateShapeConfigWidget.h"


TemplateShapeFactory::TemplateShapeFactory()
    : KoShapeFactoryBase(TEMPLATESHAPEID, i18n("Template shape")) // Template: Change to your own description
{
    setToolTip(i18n("Simple shape that is used as a template for developing other shapes."));
    setIconName(koIconNameCStr("x-shape-template"));
    setLoadingPriority(1);

    // Tell the shape loader which tag we can store
    // Template: You must change this.
    QList<QPair<QString, QStringList> > elementNamesList;
    elementNamesList.append(qMakePair(QString(KoXmlNS::calligra), QStringList("template")));
    setXmlElements(elementNamesList);
}

bool TemplateShapeFactory::supports(const KoXmlElement &e, KoShapeLoadingContext &context) const
{
    // Template: Change this to your own supported element and namespace.
    if (e.localName() == "template" && e.namespaceURI() == KoXmlNS::calligra) {
        return true;
    }

    return false;
}

KoShape *TemplateShapeFactory::createDefaultShape(KoDocumentResourceManager *documentResources) const
{
    TemplateShape *defaultShape = new TemplateShape();
    defaultShape->setShapeId(TEMPLATESHAPEID);

    // Template: Insert code to initiate the defaults of your shape here.

    return defaultShape;
}

KoShape *TemplateShapeFactory::createShape(const KoProperties *params,
                                           KoDocumentResourceManager *documentResources) const
{
    TemplateShape *shape = static_cast<TemplateShape*>(createDefaultShape(documentResources));

    return shape;
}

QList<KoShapeConfigWidgetBase*> TemplateShapeFactory::createShapeOptionPanels()
{
    QList<KoShapeConfigWidgetBase*> result;

    return result;
}
