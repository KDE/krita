/* This file is part of the KDE project
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

#include "ArtisticTextShapeFactory.h"
#include "ArtisticTextShape.h"
#include "ArtisticTextShapeConfigWidget.h"

#include <KoXmlNS.h>
#include <KoColorBackground.h>

#include <klocale.h>

ArtisticTextShapeFactory::ArtisticTextShapeFactory(QObject *parent)
    : KoShapeFactoryBase(parent, ArtisticTextShapeID, i18n("ArtisticTextShape"))
{
    setToolTip(i18n("A shape which shows a single text line"));
    setIcon( "text" );
    setLoadingPriority( 5 );
    setOdfElementNames( KoXmlNS::draw, QStringList( "custom-shape" ) );
}

KoShape *ArtisticTextShapeFactory::createDefaultShape(KoResourceManager *) const
{
    ArtisticTextShape * text = new ArtisticTextShape();
    text->setBackground( new KoColorBackground( QColor( Qt::black) ) );
    return text;
}

bool ArtisticTextShapeFactory::supports(const KoXmlElement & e) const
{
    if (!(e.localName() == "custom-shape" && e.namespaceURI() == KoXmlNS::draw)) {
        return false;
    }

    QString drawEngine = e.attributeNS( KoXmlNS::draw, "engine", "" );
    if ( drawEngine.isEmpty() || drawEngine != "svg:text" ) {
        return false;
    }

    return true;
}

#include <ArtisticTextShapeFactory.moc>
