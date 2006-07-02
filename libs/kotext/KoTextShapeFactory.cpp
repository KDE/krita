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

#include "KoProperties.h"
#include "KoTextShape.h"
#include "KoTextShapeData.h"
#include "KoTextToolFactory.h"
#include "KoToolRegistry.h"
#include <KoTextShapeFactory.h>

#include <klocale.h>
#include <kgenericfactory.h>

K_EXPORT_COMPONENT_FACTORY(kotext2,
         KGenericFactory<KoTextShapeFactory>( "TextShape" ) )

KoTextShapeFactory::KoTextShapeFactory(QObject *parent, const QStringList& list)
: KoShapeFactory(parent, KoTextShape_SHAPEID, i18n("Text"))
{
    setToolTip(i18n("A Shape That Shows Text"));

    KoShapeTemplate t;
    t.name = "Simple text";
    t.toolTip = "Text Shape With Some Text";
    KoProperties *props = new KoProperties();
    t.properties = props;
    props->setProperty("text", "<b>Koffie</b>, koffie... Querelanten\ndrinken geen KOffice maar groene thee.");
    addTemplate(t);

    // init tool factory here, since this is the only public factory in the lib
    KoToolRegistry::instance()->add(new KoTextToolFactory(parent, list));
}

KoShape *KoTextShapeFactory::createDefaultShape() {
    KoTextShape *text = new KoTextShape();
    text->setShapeId(shapeId());
    return text;
}

KoShape *KoTextShapeFactory::createShape(const KoProperties * params) const {
    KoTextShape *shape = new KoTextShape();
    shape->setShapeId(shapeId());
    QTextDocument *doc = new QTextDocument();
    doc->setDefaultFont(QFont("Sans", 10, QFont::Normal, false));
    doc->setHtml( params->getProperty("text").toString() );
    KoTextShapeData *data = static_cast<KoTextShapeData*> (shape->userData());
    data->setDocument(doc);
    return shape;
}

#include "KoTextShapeFactory.moc"
