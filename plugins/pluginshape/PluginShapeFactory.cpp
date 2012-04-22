/* This file is part of the KDE project
 * Copyright (C) 2010 Nokia Corporation and/or its subsidiary(-ies).
 *
 * Contact: Vidhyapria  Arunkumar <vidhyapria.arunkumar@nokia.com>
 * Contact: Amit Aggarwal <amit.5.aggarwal@nokia.com>
 * Contact: Manikandaprasad N C <manikandaprasad.chandrasekar@nokia.com>

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

#include "PluginShapeFactory.h"

#include "PluginShape.h"

#include <KoXmlNS.h>
#include <KoShapeLoadingContext.h>
#include "KoShapeBasedDocumentBase.h"

#include <klocale.h>
#include <kdebug.h>

PluginShapeFactory::PluginShapeFactory()
    : KoShapeFactoryBase(PLUGINSHAPEID, i18n("Plugin Placeholder"))
{
    setToolTip(i18n("Plugin Placeholder, embedded or fullscreen"));
    //setIcon("video-x-generic");
    setXmlElementNames(KoXmlNS::draw, QStringList("plugin"));
    setLoadingPriority(1);
    setHidden(true);
}

KoShape *PluginShapeFactory::createDefaultShape(KoDocumentResourceManager *documentResources) const
{
    Q_UNUSED(documentResources);
    PluginShape *defaultShape = new PluginShape();
    defaultShape->setShapeId(PLUGINSHAPEID);
    return defaultShape;
}

bool PluginShapeFactory::supports(const KoXmlElement &e, KoShapeLoadingContext &context) const
{
    Q_UNUSED(context);
    return e.localName() == "plugin" && e.namespaceURI() == KoXmlNS::draw;
}


