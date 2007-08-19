/* This file is part of the KDE project
 * Copyright (C) 2007 Pierre Ducroquet <pinaraf@gmail.com>
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

#include "PageVariable.h"

#include <KoProperties.h>
#include <kdebug.h>
#include <KoTextShapeData.h>
#include <KoShape.h>

PageVariable::PageVariable()
    : KoVariable(true),
    m_type(PageCount)
{
}

void PageVariable::setProperties(const KoProperties *props) {
    if (props->boolProperty("count")) {
        m_type = PageCount;
    } else {
        m_type = PageNumber;
    }
}

void PageVariable::propertyChanged(Property property, const QVariant &value) {
    if ((property == KoInlineObject::PageCount) && (m_type == PageCount)) {
        setValue(value.toString());
    }
}

void PageVariable::variableMoved(const KoShape *shape, const QTextDocument *document, int posInDocument) {
    if (m_type == PageNumber) {
        if (shape) {
            KoTextShapeData *shapeData = dynamic_cast<KoTextShapeData *>(shape->userData());
            if (shapeData) {
                setValue(QString::number(shapeData->pageNumber()));
            }
        }
    }
}

void PageVariable::saveOdf (KoShapeSavingContext & context) {
    KoXmlWriter *writer = &context.xmlWriter();
    if (m_type == PageCount) {
        // <text:page-count>3</text:page-count>
        writer->startElement("text:page-count", false);
        writer->addTextNode(value());
        writer->endElement();
    } else {
        // <text:page-number text:select-page="current" >3</text:page-number>
        writer->startElement("text:page-number", false);
        writer->addAttribute("text:select-page", "current");
        writer->addTextNode(value());
        writer->endElement();
    }
}
