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
#include "ThreedShapeFactory.h"

// Qt
#include <QByteArray>
#include <QBuffer>
#include <QImage>

// KDE
#include <klocale.h>
#include <kdebug.h>

// Calligra
#include <KoXmlNS.h>
#include <KoIcon.h>
//#include "KoShapeBasedDocumentBase.h"
#include <KoShapeLoadingContext.h>
#include <KoOdfLoadingContext.h>
#include <KoDocumentResourceManager.h>
#include <KoProperties.h>

// 3D Shape
#include "SceneObject.h"
//#include "ThreedShapeConfigWidget.h"


ThreedShapeFactory::ThreedShapeFactory()
    : KoShapeFactoryBase(THREEDSHAPEID, i18n("3D Scene"))
{
    setToolTip(i18n("Shape that displays a simple 3D scene."));
    //KIconLoader::global()->addAppDir("kchart");
    setIconName(koIconNameCStr("x-shape-3d"));
    setLoadingPriority(1);

    // Tell the shape loader which tag we can store
    QList<QPair<QString, QStringList> > elementNamesList;
    elementNamesList.append(qMakePair(QString(KoXmlNS::dr3d), QStringList("scene")));
    setXmlElements(elementNamesList);
}

bool ThreedShapeFactory::supports(const KoXmlElement &e, KoShapeLoadingContext &context) const
{
    Q_UNUSED(context);

    if (e.localName() == "scene" && e.namespaceURI() == KoXmlNS::dr3d) {
        return true;
    }

    return false;
}

KoShape *ThreedShapeFactory::createDefaultShape(KoDocumentResourceManager *documentResources) const
{
    Q_UNUSED(documentResources);

    kDebug(31000) << "Creating a 3d shape";

    SceneObject *defaultShape = new SceneObject(0, true);
    defaultShape->setShapeId(THREEDSHAPEID);

    return defaultShape;
}

KoShape *ThreedShapeFactory::createShape(const KoProperties *params,
                                         KoDocumentResourceManager *documentResources) const
{
    Q_UNUSED(params);

    SceneObject *shape = static_cast<SceneObject*>(createDefaultShape(documentResources));

    return shape;
}

QList<KoShapeConfigWidgetBase*> ThreedShapeFactory::createShapeOptionPanels()
{
    QList<KoShapeConfigWidgetBase*> result;

    return result;
}
