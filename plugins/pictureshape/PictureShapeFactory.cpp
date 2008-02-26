/* This file is part of the KDE project
 * Copyright (C) 2007 Thomas Zander <zander@kde.org>
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

#include "PictureShapeFactory.h"

#include "PictureShape.h"
#include <KoXmlNS.h>
#include "KoShapeControllerBase.h"
#include "KoImageCollection.h"

#include <klocale.h>
#include <kdebug.h>

PictureShapeFactory::PictureShapeFactory( QObject* parent)
    : KoShapeFactory( parent, PICTURESHAPEID, i18n( "Picture Shape" ) )
{
    setToolTip( i18n( "A shape which displays a picture" ) );
    ///@todo setIcon( "pictureshape" );
    setIcon( "image" );
    setOdfElementNames( KoXmlNS::draw, QStringList( "image" ) );
    setLoadingPriority( 1 );
}

KoShape* PictureShapeFactory::createDefaultShape() const
{
    return new PictureShape();
}

KoShape* PictureShapeFactory::createShape( const KoProperties* params ) const
{
    Q_UNUSED(params);
    return createDefaultShape();
}

bool PictureShapeFactory::supports(const KoXmlElement & e) const
{
    return ( e.localName() == "image" && e.namespaceURI() == KoXmlNS::draw );
}

void PictureShapeFactory::populateDataCenterMap(QMap<QString, KoDataCenter *>   & dataCenterMap) 
{
    KoImageCollection *imgCol = new KoImageCollection();
    dataCenterMap["ImageCollection"] = imgCol;
}
