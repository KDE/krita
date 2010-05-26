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

InfoVariable::InfoVariable()
        : KoVariable(true),
        m_type(KoInlineObject::DocumentURL)
{
}

void InfoVariable::readProperties(const KoProperties *props)
{
    m_type = (Property) props->property("property").value<int>();
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
    QString nodeName;
    if (m_type == KoInlineObject::Title)
        nodeName = "text:title";
    else if (m_type == KoInlineObject::Subject)
        nodeName = "text:subject";
    else if (m_type == KoInlineObject::Keywords)
        nodeName = "text:keywords";
    if (!nodeName.isEmpty()) {
        writer->startElement(nodeName.toLatin1(), false);
        writer->addTextNode(value());
        writer->endElement();
    }
}

bool InfoVariable::loadOdf(const KoXmlElement & element, KoShapeLoadingContext & /*context*/)
{
    const QString localName(element.localName());

    if (localName == "title") {
        m_type = KoInlineObject::Title;
    } else if (localName == "subject") {
        m_type = KoInlineObject::Subject;
    } else if (localName == "keywords") {
        m_type = KoInlineObject::Keywords;
    }

    return true;
}
