/* This file is part of the KDE project
 * Copyright (C) 2006 Thomas Zander <zander@kde.org>
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

#include "RectangleShapeFactory.h"
#include "RectangleShape.h"
#include "RectangleShapeConfigWidget.h"
#include "KoLineBorder.h"
#include <KoXmlNS.h>
#include <KoXmlReader.h>
#include <KoGradientBackground.h>

#include <klocale.h>

RectangleShapeFactory::RectangleShapeFactory()
: KoShapeFactoryBase(RectangleShapeId, i18n("Rectangle"))
{
    setToolTip(i18n("A rectangle"));
    setIcon("rectangle-shape");
    setFamily("geometric");
    setOdfElementNames(KoXmlNS::draw, QStringList("rect"));
    setLoadingPriority(1);
}

KoShape *RectangleShapeFactory::createDefaultShape(KoResourceManager *) const
{
    RectangleShape *rect = new RectangleShape();

    rect->setBorder(new KoLineBorder(1.0));
    rect->setShapeId(KoPathShapeId);

    QLinearGradient *gradient = new QLinearGradient(QPointF(0,0), QPointF(1,1));
    gradient->setCoordinateMode(QGradient::ObjectBoundingMode);

    gradient->setColorAt(0.0, Qt::white);
    gradient->setColorAt(1.0, Qt::green);
    rect->setBackground(new KoGradientBackground(gradient));

    return rect;
}

bool RectangleShapeFactory::supports(const KoXmlElement & e) const
{
    return (e.localName() == "rect" && e.namespaceURI() == KoXmlNS::draw);
}

QList<KoShapeConfigWidgetBase*> RectangleShapeFactory::createShapeOptionPanels()
{
    QList<KoShapeConfigWidgetBase*> panels;
    panels.append(new RectangleShapeConfigWidget());
    return panels;
}

