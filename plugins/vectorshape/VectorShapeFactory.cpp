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

// KOffice
#include <KoXmlNS.h>
#include "KoShapeControllerBase.h"


// KDE
#include <klocale.h>
#include <KDebug>


VectorShapeFactory::VectorShapeFactory(QObject *parent)
    : KoShapeFactoryBase(parent, VectorShape_SHAPEID, i18n("Vector"))
{
    setToolTip(i18n("A shape that shows a vector image"));
    setIcon( "vector-shape" );
    setOdfElementNames(KoXmlNS::draw, QStringList("image"));
    setLoadingPriority(1);
}

KoShape *VectorShapeFactory::createDefaultShape(KoResourceManager *documentResources) const
{
    VectorShape *shape = new VectorShape();
    shape->setShapeId(VectorShape_SHAPEID);

    return shape;
}

bool VectorShapeFactory::supports(const KoXmlElement & e) const
{
    return e.localName() == "image" && e.namespaceURI() == KoXmlNS::draw;
}

QList<KoShapeConfigWidgetBase*> VectorShapeFactory::createShapeOptionPanels()
{
    QList<KoShapeConfigWidgetBase*> result;

    return result;
}

#include <VectorShapeFactory.moc>
