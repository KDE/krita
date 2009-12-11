/* This file is part of the KDE project
 * Copyright (C) 2008 Pierre Ducroquet <pinaraf@gmail.com>
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

#include "TOCVariable.h"

#include <KoXmlReader.h>
#include <KoXmlWriter.h>
#include <KoProperties.h>
#include <KoParagraphStyle.h>
#include <kdebug.h>
#include <KoTextShapeData.h>
#include <KoShape.h>
#include <KoShapeSavingContext.h>
#include <KoShapeLoadingContext.h>
#include <QFontMetricsF>
#include <QTextDocument>
#include <QTextInlineObject>
#include <QTextCursor>
#include <QPainter>
#include <QTextBlock>
#include <KoTextDocumentLayout.h>
#include <KoXmlNS.h>
#include <KoTextLoader.h>
#include <KoTextSharedLoadingData.h>
#include <KoTextBlockData.h>
#include <KoTextPage.h>
#include <KoTextDocument.h>
#include <KoStyleManager.h>

TOCVariable::TOCVariable()
        : KoVariable(true), currentDoc(0), source(TOCSource(this))
{
    KoTextDocument doc(&indexBody);
    doc.setStyleManager(new KoStyleManager());
}

void TOCVariable::setProperties(const KoProperties *props)
{
    Q_UNUSED(props);
}

void TOCVariable::propertyChanged(Property property, const QVariant &value)
{
    Q_UNUSED(property);
    Q_UNUSED(value);
}

void TOCVariable::variableMoved(const KoShape *shape, const QTextDocument *document, int posInDocument)
{
    Q_UNUSED(posInDocument);
    Q_UNUSED(shape);
    setCurrentDocument(document);
// Refresh forced here....
//update();
}

void TOCSourceTemplate::saveOdf(KoShapeSavingContext & context)
{
    KoXmlWriter *writer = &context.xmlWriter();
    writer->startElement("text:table-of-content-entry-template");
    writer->addAttribute("text:outline-level", m_outlineLevel);
    // writer->addAttribute("text:style-name", "something"); // I don't remember this, sorry.
    writer->endElement(); // text:table-of-content-entry-template
}

void TOCSource::saveOdf(KoShapeSavingContext & context)
{
    KoXmlWriter *writer = &context.xmlWriter();
    writer->startElement("text:table-of-content-source");
    writer->addAttribute("text:outline-level", m_outlineLevel);
    writer->startElement("text:index-title-template");
    // writer->addAttribute("text:style-name", "something"); // I don't remember this, sorry.
    writer->addTextNode(m_titleTemplate);
    writer->endElement(); // text:index-title-template
    foreach(TOCSourceTemplate tpl, m_sources) {
        tpl.saveOdf(context);
    }
    writer->endElement();
}

void TOCVariable::saveOdf(KoShapeSavingContext & context)
{
    KoXmlWriter *writer = &context.xmlWriter();
    writer->startElement("text:table-of-content");
    writer->addAttribute("text:protected", true);
    source.saveOdf(context);
    writer->startElement("text:index-body");
    // Wonderful, store the index-body here... But what about the text:index-title thing ?
    writer->addTextNode("Work in progress, sorry, you must refresh your table of content manually after saving it.");

    writer->endElement(); // text:index-body
    writer->endElement(); // text:table-of-content
}

void TOCSource::buildFromDocument(const QTextDocument *source, QTextCursor *target)
{
    QTextBlock block;

    target->beginEditBlock();
    // First, insert our TOC title.
    target->insertText(m_titleTemplate);
    target->insertBlock();
    if (m_titleStyle) {
        block = target->block().previous();
        m_titleStyle->applyStyle(block, true);
    }
    // Ok, look in the document, find the title... :)
    KoTextDocumentLayout *docLayout = qobject_cast<KoTextDocumentLayout*>(source->documentLayout());
    if (!docLayout) {
        kWarning(31000) << "No layout for the document ??? I cancel.";
        return;
    }
    block = source->begin();
    while (block.isValid()) {
        int outlineLevel = block.blockFormat().intProperty(KoParagraphStyle::OutlineLevel);
        if (outlineLevel > 0) {
            KoShape *shape = docLayout->shapeForPosition(block.position());
            if (shape) {
                KoTextShapeData *shapeData = qobject_cast<KoTextShapeData *>(shape->userData());
                Q_ASSERT( shapeData );
                if( shapeData )
                {
                    target->insertText("TOC entry " +
                                       QString::number(outlineLevel) + " :" + block.text() +
                                       ";page" + QString::number(shapeData->page()->pageNumber() + 1)
                                      );
                    target->insertBlock();
                }
            }
        }
        block = block.next();
    }
    target->endEditBlock();
}

bool TOCSourceTemplate::loadOdf(const KoXmlElement & element, KoShapeLoadingContext & context)
{
    KoSharedLoadingData * sharedData = context.sharedData(KOTEXT_SHARED_LOADING_ID);
    Q_ASSERT( sharedData );
    KoTextSharedLoadingData * textSharedData = dynamic_cast<KoTextSharedLoadingData *>(sharedData);
    Q_ASSERT( textSharedData );
    m_style = textSharedData->paragraphStyle(element.attributeNS(KoXmlNS::text, "style-name", ""), false);
    if (!m_style) {
        m_style = textSharedData->paragraphStyle(element.attributeNS(KoXmlNS::text, "style-name", ""), true);
    }
    m_outlineLevel = element.attributeNS(KoXmlNS::text, "outline-level", "10").toInt();
    return true;
}

bool TOCSource::loadOdf(const KoXmlElement & element, KoShapeLoadingContext & context)
{
    KoSharedLoadingData * sharedData = context.sharedData(KOTEXT_SHARED_LOADING_ID);
    KoTextSharedLoadingData * textSharedData = 0;
    if (sharedData) {
        textSharedData = dynamic_cast<KoTextSharedLoadingData *>(sharedData);
    }

    m_outlineLevel = element.attributeNS(KoXmlNS::text, "outline-level", "10").toInt();
    KoXmlElement e;
    forEachElement(e, element) {
        if (e.namespaceURI() != KoXmlNS::text)
            continue;
        if (e.tagName() == "table-of-content-entry-template") {
            TOCSourceTemplate tpl;
            tpl.loadOdf(e, context);
            m_sources << tpl;
        } else if (e.tagName() == "index-title-template") {
            m_titleTemplate = e.text();
            m_titleStyle = textSharedData->paragraphStyle(e.attributeNS(KoXmlNS::text, "style-name", ""), false);
            if (!m_titleStyle) {
                m_titleStyle = textSharedData->paragraphStyle(e.attributeNS(KoXmlNS::text, "style-name", ""), true);
            }
        }
    }
    return true;
}

bool TOCVariable::loadOdf(const KoXmlElement & element, KoShapeLoadingContext & context)
{
    KoXmlElement e;
    forEachElement(e, element) {
        //kDebug() << e.namespaceURI() << e.tagName();
        if (e.namespaceURI() != KoXmlNS::text)
            continue;
        if (e.tagName() == "table-of-content-source") {
            source.loadOdf(e, context);
        } else if (e.tagName() == "index-body") {
            KoTextLoader indexLoader(context);
            QTextCursor cursor(&indexBody);
            KoXmlElement bodyElem;
            forEachElement(bodyElem, e) {
                if (bodyElem.namespaceURI() != KoXmlNS::text)
                    continue;
                if (bodyElem.tagName() == "index-title") {
                    indexLoader.loadBody(bodyElem, cursor);
                    break;
                }
            }
            indexLoader.loadBody(e, cursor);
        }
    }
    return true;
}

void TOCVariable::resize(const QTextDocument *document, QTextInlineObject object, int posInDocument, const QTextCharFormat &format, QPaintDevice *pd)
{
    Q_UNUSED(document);
    Q_UNUSED(posInDocument);
    Q_ASSERT(format.isCharFormat());
    QFontMetricsF fm(format.font(), pd);
    object.setWidth(indexBody.documentLayout()->documentSize().width());
    object.setAscent(fm.ascent());
    object.setDescent(fm.descent() + indexBody.documentLayout()->documentSize().height()); // HACK ? Who said hack ? It's clean of course... :/
}

void TOCVariable::paint(QPainter &painter, QPaintDevice *pd, const QTextDocument *document, const QRectF &rect, QTextInlineObject object, int posInDocument, const QTextCharFormat &format)
{
    Q_UNUSED(document);
    Q_UNUSED(object);
    Q_UNUSED(posInDocument);
    Q_UNUSED(pd);
    Q_UNUSED(format);

    //kDebug(31000) << "Painting a TOCVariable with rect=" << rect;
    painter.translate(QPointF(-2, -2) + rect.topLeft());
    indexBody.drawContents(&painter);
    painter.translate(QPointF(2, 2) - rect.topLeft());
}

void TOCVariable::update()
{
    if (!currentDoc)
        return;
    indexBody.clear();
    QTextCursor cursor(&indexBody);
    source.buildFromDocument(currentDoc, &cursor);
}

void TOCVariable::setCurrentDocument(const QTextDocument *document)
{
    if (currentDoc == document)
        return;
    currentDoc = document;
    KoTextDocument orig(currentDoc);
    KoTextDocument doc(&indexBody);
    doc.setStyleManager(orig.styleManager());
}
