/* This file is part of the KDE project
 * Copyright (C) 2008 Peter Simonsson <peter.simonsson@gmail.com>
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

#include "CollectionShapeFactory.h"

#include <KoShape.h>
#include <KoDrag.h>
#include <KoShapeOdfSaveHelper.h>
#include <KoOdf.h>
#include <KoShapeLoadingContext.h>
#include <KoShapeControllerBase.h>
#include <KoOdfLoadingContext.h>
#include <KoStore.h>
#include <KoOdfReadStore.h>
#include <KoXmlNS.h>
#include <KoShapeRegistry.h>

#include <kdebug.h>

#include <QMimeData>
#include <QBuffer>

CollectionShapeFactory::CollectionShapeFactory(const QString &id, KoShape* shape)
    : KoShapeFactoryBase(id, shape->name()), m_shape(shape)
{
}

CollectionShapeFactory::~CollectionShapeFactory()
{
    delete m_shape;
}

KoShape *CollectionShapeFactory::createDefaultShape(KoResourceManager *documentResources) const
{
    QList<KoShape*> shapes;

    shapes << m_shape;

    //kDebug() << m_shape->shapeId();

    KoDrag drag;
    KoShapeOdfSaveHelper saveHelper(shapes);
    drag.setOdf(KoOdf::mimeType(KoOdf::Graphics), saveHelper);
    QMimeData* data = drag.mimeData();

    QByteArray arr = data->data(KoOdf::mimeType(KoOdf::Graphics));
    KoShape* shape = 0;

    if ( !arr.isEmpty() ) {
        QBuffer buffer( &arr );
        KoStore * store = KoStore::createStore( &buffer, KoStore::Read );
        KoOdfReadStore odfStore( store );

        QString errorMessage;
        if ( ! odfStore.loadAndParse( errorMessage ) ) {
            kError() << "loading and parsing failed:" << errorMessage << endl;
            return 0;
        }

        KoXmlElement content = odfStore.contentDoc().documentElement();
        KoXmlElement realBody( KoXml::namedItemNS( content, KoXmlNS::office, "body" ) );

        if ( realBody.isNull() ) {
            kError() << "No body tag found!" << endl;
            return 0;
        }

        KoXmlElement body = KoXml::namedItemNS( realBody, KoXmlNS::office, KoOdf::bodyContentElement( KoOdf::Text, false ) );

        if ( body.isNull() ) {
            kError() << "No" << KoOdf::bodyContentElement(KoOdf::Text, true ) << "tag found!" << endl;
            return 0;
        }

        KoOdfLoadingContext loadingContext(odfStore.styles(), odfStore.store());
        KoShapeLoadingContext context(loadingContext, documentResources);

        KoXmlElement element;

        forEachElement(element, body)
        {
            KoShape * shape = KoShapeRegistry::instance()->createShapeFromOdf( element, context );
            if ( shape ) {
                delete data;
                return shape;
            }
        }
    }

    delete data;
    return shape;
}
