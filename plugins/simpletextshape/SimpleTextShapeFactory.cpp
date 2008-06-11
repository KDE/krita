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

#include "SimpleTextShapeFactory.h"
#include "SimpleTextShape.h"
#include "SimpleTextShapeConfigWidget.h"

#include <KoXmlNS.h>
#include <KoColorBackground.h>

#include <klocale.h>

SimpleTextShapeFactory::SimpleTextShapeFactory(QObject *parent)
    : KoShapeFactory(parent, SimpleTextShapeID, i18n("SimpleTextShape"))
{
    setToolTip(i18n("A shape which shows a single text line"));
    setIcon( "text" );
    setLoadingPriority( 5 );
    setOdfElementNames( KoXmlNS::draw, QStringList( "custom-shape" ) );
}

KoShape *SimpleTextShapeFactory::createDefaultShape() const
{
    SimpleTextShape * text = new SimpleTextShape();
    text->setBackground( new KoColorBackground( QColor( Qt::black) ) );
    return text;
}

KoShape *SimpleTextShapeFactory::createShape(const KoProperties *) const
{
    return createDefaultShape();
}

QList<KoShapeConfigWidgetBase*> SimpleTextShapeFactory::createShapeOptionPanels()
{
    QList<KoShapeConfigWidgetBase*> answer;
    answer.append( new SimpleTextShapeConfigWidget() );
    return answer;
}

bool SimpleTextShapeFactory::supports(const KoXmlElement & e) const
{
    return ( e.localName() == "custom-shape" && e.namespaceURI() == KoXmlNS::draw );
}

#include "SimpleTextShapeFactory.moc"
