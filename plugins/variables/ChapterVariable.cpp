/* This file is part of the KDE project
 * Copyright (C) 2011 Sebastian Sauer <mail@dipe.org>
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

#include "ChapterVariable.h"

#include <KoXmlReader.h>
#include <KoXmlWriter.h>
#include <KoProperties.h>
#include <kdebug.h>
#include <KoShape.h>
#include <KoShapeSavingContext.h>
#include <KoShapeLoadingContext.h>
#include <KoXmlNS.h>
#include <KoTextLayoutRootArea.h>
#include <KoTextDocumentLayout.h>
#include <KoParagraphStyle.h>
#include <KoTextBlockData.h>

#include <QFontMetricsF>
#include <QTextDocument>
#include <QAbstractTextDocumentLayout>
#include <QTextInlineObject>
#include <QDebug>

ChapterVariable::ChapterVariable()
        : KoVariable(true)
        , m_type(KoInlineObject::ChapterName)
        , m_level(0)
{
}

void ChapterVariable::readProperties(const KoProperties *props)
{
    m_type = (KoInlineObject::Property) props->intProperty("vartype");
    m_level = props->intProperty("level");
}

void ChapterVariable::propertyChanged(Property property, const QVariant &value)
{
    Q_UNUSED(property);
    //setValue(value.toString());
}

void ChapterVariable::resize(const QTextDocument *document, QTextInlineObject object, int posInDocument, const QTextCharFormat &format, QPaintDevice *pd)
{
    QTextBlock block = document->findBlock(posInDocument);//cursor.block();
    for(; block.isValid(); block = block.previous()) {
        if (block.blockFormat().hasProperty(KoParagraphStyle::OutlineLevel)) {
            int level = block.blockFormat().intProperty(KoParagraphStyle::OutlineLevel);
            if (m_level < 1 || level == m_level) {
                KoTextBlockData *data = dynamic_cast<KoTextBlockData*>(block.userData());
                switch(m_type) {
                case KoInlineObject::ChapterName:
                    setValue(block.text());
                    break;
                case KoInlineObject::ChapterNumber:
                    setValue(data ? data->counterText() : QString());
                    break;
                case KoInlineObject::ChapterNumberName:
                    setValue(data ? QString("%1 %2").arg(data->counterText()).arg(block.text()) : block.text());
                    break;
                case KoInlineObject::ChapterPlainNumber:
                    setValue(data ? QString::number(data->counterIndex()) : QString());
                    break;
                case KoInlineObject::ChapterPlainNumberName:
                    setValue(data ? QString("%1 %2").arg(data->counterIndex()).arg(block.text()) : QString());
                    break;
                default:
                    break;
                }
                break;
            }
        }
    }

    KoVariable::resize(document, object, posInDocument, format, pd);
}

void ChapterVariable::saveOdf(KoShapeSavingContext &context)
{
    KoXmlWriter *writer = &context.xmlWriter();
    writer->startElement("text:chapter ", false);
    switch(m_type) {
    case KoInlineObject::ChapterName:
        writer->addAttribute("text:display", "name");
        break;
    case KoInlineObject::ChapterNumber:
        writer->addAttribute("text:display", "number");
        break;
    case KoInlineObject::ChapterNumberName:
        writer->addAttribute("text:display", "number-and-name");
        break;
    case KoInlineObject::ChapterPlainNumber:
        writer->addAttribute("text:display", "plain-number");
        break;
    case KoInlineObject::ChapterPlainNumberName:
        writer->addAttribute("text:display", "plain-number-and-name");
        break;
    default:
        break;
    }
    writer->addAttribute("text:outline-level", m_level);
    writer->addTextNode(value());
    writer->endElement(); // text:chapter
}

bool ChapterVariable::loadOdf(const KoXmlElement & element, KoShapeLoadingContext & context)
{
    Q_UNUSED(context);

    const QString display = element.attributeNS(KoXmlNS::text, "display", QString());
    if (display == "name") {
        m_type = KoInlineObject::ChapterName;
    } else if (display == "number") {
        m_type = KoInlineObject::ChapterNumber;
    } else if (display == "number-and-name") {
        m_type = KoInlineObject::ChapterNumberName;
    } else if (display == "plain-number") {
        m_type = KoInlineObject::ChapterPlainNumber;
    } else if (display == "plain-number-and-name") {
        m_type = KoInlineObject::ChapterPlainNumberName;
    } else { // fallback
        m_type = KoInlineObject::ChapterNumberName;
    }

    m_level = element.attributeNS(KoXmlNS::text, "outline-level", QString()).toInt();

    return true;
}
