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

#include <KoTextShapeFactory.h>
#include "KoTextShape.h"
#include "KoProperties.h"

#include <klocale.h>
#include <kgenericfactory.h>

K_EXPORT_COMPONENT_FACTORY(kotext2,
         KGenericFactory<KoTextShapeFactory>( "TextShape" ) )

KoTextShapeFactory::KoTextShapeFactory(QObject *parent, const QStringList&)
: KoShapeFactory(parent, "TextShape", i18n("A shape that shows text"))
{
    setToolTip(i18n("A text shape"));

    KoShapeTemplate t;
    t.name = "Simple text";
    t.description = "Texty";
    t.toolTip = "Text shape with some text";
    KoProperties *props = new KoProperties();
    t.properties = props;
    props->setProperty("text", "<b>Koffie</b>, koffie... Querelanten\ndrinken geen KOffice maar groene thee.");
    addTemplate(t);
}

KoShape *KoTextShapeFactory::createDefaultShape() {
    return new KoTextShape();
}

KoShape *KoTextShapeFactory::createShape(const KoProperties * params) const {
    return new KoTextShape();
}

#include "KoTextShapeFactory.moc"
