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
#include "EmfShapeFactory.h"

// EmfShape
#include "EmfShape.h"

// KOffice
#include <KoXmlNS.h>
#include "KoShapeControllerBase.h"


// KDE
#include <klocale.h>
#include <KDebug>


// The default EMF.  Contains "static char defaultEMF[]"
#include "DefaultEmf.h"


EmfShapeFactory::EmfShapeFactory(QObject *parent)
    : KoShapeFactory(parent, EmfShape_SHAPEID, i18n("Emf"))
{
    setToolTip(i18n("A Shape That Shows an Embedded Metafile"));
    setIcon( "emf-shape" );
    setOdfElementNames(KoXmlNS::draw, QStringList("image"));
    setLoadingPriority(1);
}

KoShape *EmfShapeFactory::createDefaultShape() const
{
    EmfShape *emf = new EmfShape();
    emf->setShapeId(EmfShape_SHAPEID);

    emf->setEmfBytes(defaultEMF, sizeof(defaultEMF), false);

    return emf;
}

KoShape *EmfShapeFactory::createShape(const KoProperties * /*params*/) const
{
    return createDefaultShape();
}

    /// Reimplemented
bool EmfShapeFactory::supports(const KoXmlElement & e) const
{
    kDebug(33001) << "EmfShapeFactory::supports(): Tag = " << e.tagName();
    return e.localName() == "image" && e.namespaceURI() == KoXmlNS::draw;
}

QList<KoShapeConfigWidgetBase*> EmfShapeFactory::createShapeOptionPanels()
{
    QList<KoShapeConfigWidgetBase*> result;

    return result;
}

#include "EmfShapeFactory.moc"
