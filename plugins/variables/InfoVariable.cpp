/* This file is part of the KDE project
 * Copyright (C) 2007 Pierre Ducroquet <pinaraf@gmail.com>
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

#include "InfoVariable.h"

#include <KoProperties.h>
#include <kdebug.h>
#include <KoShapeSavingContext.h>
#include <KoXmlReader.h>
#include <KoXmlWriter.h>

#include <KGlobal>

static const struct {
    KoInlineObject::Property property;
    const char * tag;
    const char * saveTag;
} propertyData[] = {
    { KoInlineObject::AuthorName, "creator", "text:creator" },
    { KoInlineObject::DocumentURL, "file-name", "text:file-name" },
    { KoInlineObject::Title, "title", "text:title" },
    { KoInlineObject::Subject, "subject", "text:subject" },
    { KoInlineObject::Keywords, "keywords", "text:keywords" },
    //{ KoInlineObject::Description, "description", "text:description" }
    { KoInlineObject::Comments, "comments", "text:comments" }
};

static const unsigned int numPropertyData = sizeof(propertyData) / sizeof(*propertyData);

QStringList InfoVariable::tags()
{
    QStringList tagList;
    for (unsigned int i = 0; i < numPropertyData; ++i) {
        tagList << propertyData[i].tag;
    }
    return tagList;
}

InfoVariable::InfoVariable()
        : KoVariable(true),
        m_type(KoInlineObject::AuthorName)
{
}

void InfoVariable::readProperties(const KoProperties *props)
{
    m_type = (Property) props->property("vartype").value<int>();
}

void InfoVariable::propertyChanged(Property property, const QVariant &value)
{
    if (property == m_type) {
        setValue(value.toString());
    }
}

void InfoVariable::saveOdf(KoShapeSavingContext & context)
{
    KoXmlWriter *writer = &context.xmlWriter();

    typedef QMap<KoInlineObject::Property, const char*> SaveMap;
    K_GLOBAL_STATIC(SaveMap, s_saveInfo)

    if (!s_saveInfo.exists() ) {
        for (unsigned int i = 0; i < numPropertyData; ++i) {
            s_saveInfo->insert(propertyData[i].property, propertyData[i].saveTag);
        }
    }
    const char *nodeName = s_saveInfo->value(m_type, 0);
    if (nodeName) {
        writer->startElement(nodeName, false);
        writer->addTextNode(value());
        writer->endElement();
    }
}

bool InfoVariable::loadOdf(const KoXmlElement & element, KoShapeLoadingContext & /*context*/)
{
    typedef QMap<QString, KoInlineObject::Property> LoadMap;
    K_GLOBAL_STATIC(LoadMap, s_loadInfo)

    if (!s_loadInfo.exists() ) {
        for (unsigned int i = 0; i < numPropertyData; ++i) {
            s_loadInfo->insert(propertyData[i].tag, propertyData[i].property);
        }
    }

    const QString localName(element.localName());
    m_type = s_loadInfo->value(localName);

    for(KoXmlNode node = element.firstChild(); !node.isNull(); node = node.nextSibling() ) {
        if (node.isText()) {
            setValue(node.toText().data());
            break;
        }
    }

    return true;
}
