/* This file is part of the KDE project
 * Copyright (C) 2006-2007,2009 Thomas Zander <zander@kde.org>
 * Copyright (C) 2007 Jan Hambrecht <jaham@gmx.net>
 * Copyright (C) 2008 Thorsten Zachmann <zachmann@kde.org>
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
#include "TextShapeFactory.h"
#include "TextShape.h"

#include <KoProperties.h>
#include <KoShape.h>
#include <KoTextShapeData.h>
#include <KoXmlNS.h>
#include <KoStyleManager.h>
#include <KoInlineTextObjectManager.h>
#include <changetracker/KoChangeTracker.h>

#include <klocale.h>
#include <QTextCursor>

TextShapeFactory::TextShapeFactory(QObject *parent)
        : KoShapeFactory(parent, TextShape_SHAPEID, i18n("Text")),
        m_inlineTextObjectManager(0)
{
    setToolTip(i18n("A shape that shows text"));
    setOdfElementNames(KoXmlNS::draw, QStringList("text-box"));
    setLoadingPriority(1);

    KoShapeTemplate t;
    t.name = i18n("Text");
    t.icon = "x-shape-text";
    t.toolTip = i18n("Text Shape");
    KoProperties *props = new KoProperties();
    t.properties = props;
    props->setProperty("demo", true);
    addTemplate(t);
}

KoShape *TextShapeFactory::createDefaultShape() const
{
    TextShape *text = new TextShape(m_inlineTextObjectManager);
    return text;
}

KoShape *TextShapeFactory::createShape(const KoProperties *params) const
{
    TextShape *shape = new TextShape(m_inlineTextObjectManager);
    shape->setSize(QSizeF(300, 200));
    shape->setDemoText(params->boolProperty("demo"));
    QString text("text");
    if (params->contains(text)) {
        KoTextShapeData *shapeData = qobject_cast<KoTextShapeData*>(shape->userData());
        QTextCursor cursor(shapeData->document());
        cursor.insertText(params->stringProperty(text));
    }
    return shape;
}

bool TextShapeFactory::supports(const KoXmlElement & e) const
{
    return (e.localName() == "text-box" && e.namespaceURI() == KoXmlNS::draw);
}

void TextShapeFactory::populateDataCenterMap(QMap<QString, KoDataCenter *>  & dataCenterMap)
{
    dataCenterMap["StyleManager"] = new KoStyleManager();
    m_inlineTextObjectManager = new KoInlineTextObjectManager(this);
    dataCenterMap["InlineTextObjectManager"] = m_inlineTextObjectManager;
    dataCenterMap["ChangeTracker"] = new KoChangeTracker();
}

#include <TextShapeFactory.moc>
