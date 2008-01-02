/* This file is part of the KDE project
 * Copyright (C) 2008 Boudewijn Rempt <boud@valdyas.org>
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

#include "TableShapeFactory.h"

#include "TableShape.h"
#include <KoXmlNS.h>
#include <klocale.h>

TableShapeFactory::TableShapeFactory( QObject* parent)
    : KoShapeFactory( parent, TABLESHAPEID, i18n( "Table Shape" ) )
{
    setToolTip( i18n( "A shape which displays a table" ) );
    ///@todo setIcon( "tableshape" );
    setIcon( "image" );
    setOdfElementNames( KoXmlNS::draw, QStringList( "image" ) );
    setLoadingPriority( 1 );
}

KoShape* TableShapeFactory::createDefaultShape() const
{
    return new TableShape();
}

KoShape* TableShapeFactory::createShape( const KoProperties* params ) const
{
    Q_UNUSED(params);
    return createDefaultShape();
}

bool TableShapeFactory::supports(const KoXmlElement & e) const
{
    return ( e.localName() == "image" && e.namespaceURI() == KoXmlNS::draw );
}

