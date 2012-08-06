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

#include <KoXmlNS.h>
#include <KoColorBackground.h>

#include <KoIcon.h>
#include <klocale.h>

ArtisticTextShapeFactory::ArtisticTextShapeFactory()
    : KoShapeFactoryBase(ArtisticTextShapeID, i18n("ArtisticTextShape"))
{
    setToolTip(i18n("A shape which shows a single text line"));
    setIconName(koIconNameCStrNeeded("currently falls back to x-shape-text","x-shape-text-artistic"));
    setLoadingPriority( 5 );
    setXmlElementNames(KoXmlNS::svg, QStringList("text"));
}

KoShape *ArtisticTextShapeFactory::createDefaultShape(KoDocumentResourceManager *) const
{
    ArtisticTextShape * text = new ArtisticTextShape();
    text->setBackground( new KoColorBackground( QColor( Qt::black) ) );
    text->setPlainText(i18n("Artistic Text"));
    return text;
}

bool ArtisticTextShapeFactory::supports(const KoXmlElement &/*element*/, KoShapeLoadingContext &/*context*/) const
{
    // the artistic text shape is embedded as svg into an odf file
    // so we tell the caller we do not support any element
    return false;
}
