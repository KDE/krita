/* This file is part of the KDE project
 * Copyright (C) 2007 Thomas Zander <zander@kde.org>
 * Copyright (C) 2010 KO Gmbh <boud@kogmbh.com>
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
#include "KoInlineNote.h"

#include <KoXmlReader.h>
#include <KoXmlWriter.h>
#include <KoXmlNS.h>
#include <KoShapeSavingContext.h>
#include <KoTextLoader.h>
#include <KoTextWriter.h>
#include <KoTextDocument.h>
#include <KoText.h>
#include <KoInlineTextObjectManager.h>
#include <KoGenStyles.h>
#include <KDebug>

#include <QTextDocument>
#include <QTextFrame>
#include <QTextCursor>
#include <QString>
#include <QTextInlineObject>
#include <QFontMetricsF>
#include <QTextOption>
#include <QDateTime>
#include <QWeakPointer>
#include <QBuffer>

class KoInlineNote::Private
{
public:
    Private(KoInlineNote::Type t)
        : textFrame(0)
        , autoNumbering(false)
        , type(t)
    {
    }

    QTextFrame *textFrame;
    QString label;
    QString id;
    QString author;
    QDateTime date;
    bool autoNumbering;
    KoInlineNote::Type type;
    int posInDocument;
};

KoInlineNote::KoInlineNote(Type type)
    : KoInlineObject(true)
    , d(new Private(type))
{
}

KoInlineNote::~KoInlineNote()
{
    delete d;
}

QTextCursor KoInlineNote::textCursor() const
{
    return (d->textFrame->lastCursorPosition());
}

void KoInlineNote::setMotherFrame(QTextFrame *motherFrame)
{
    // We create our own subframe

    QTextCursor cursor(motherFrame->lastCursorPosition());
    QTextFrameFormat format;
    format.setProperty(KoText::SubFrameType, KoText::NoteFrameType);
    d->textFrame = cursor.insertFrame(format);
}

void KoInlineNote::setLabel(const QString &text)
{
    d->label = text;
}

void KoInlineNote::setId(const QString &id)
{
    d->id = id;
}

QTextFrame *KoInlineNote::textFrame() const
{
    return d->textFrame;
}

QString KoInlineNote::label() const
{
    return d->label;
}

QString KoInlineNote::id() const
{
    return d->id;
}

bool KoInlineNote::autoNumbering() const
{
    return d->autoNumbering;
}

void KoInlineNote::setAutoNumbering(bool on)
{
    d->autoNumbering = on;
}

KoInlineNote::Type KoInlineNote::type() const
{
    return d->type;
}

void KoInlineNote::updatePosition(const QTextDocument *document, int posInDocument, const QTextCharFormat &format)
{
    Q_UNUSED(document);
    Q_UNUSED(format);
    d->posInDocument = posInDocument;
}

void KoInlineNote::resize(const QTextDocument *document, QTextInlineObject object, int posInDocument, const QTextCharFormat &format, QPaintDevice *pd)
{
    Q_UNUSED(document);
    Q_UNUSED(posInDocument);
    if (d->label.isEmpty()) {
        object.setWidth(0);
        object.setAscent(0);
        object.setDescent(0);
    } else {
        Q_ASSERT(format.isCharFormat());
        QFontMetricsF fm(format.font(), pd);
        object.setWidth(fm.width(d->label));
        object.setAscent(fm.ascent());
        object.setDescent(fm.descent());
    }
}

void KoInlineNote::paint(QPainter &painter, QPaintDevice *pd, const QTextDocument *document, const QRectF &rect, QTextInlineObject object, int posInDocument, const QTextCharFormat &format)
{
    Q_UNUSED(document);
    Q_UNUSED(object);
    Q_UNUSED(posInDocument);

    if (d->label.isEmpty())
        return;
    QFont font(format.font(), pd);
    KoOdfNotesConfiguration *notesConfig = 0;
    if (d->type == KoInlineNote::Footnote) {
        notesConfig = KoTextDocument(this->textFrame()->document()).notesConfiguration(KoOdfNotesConfiguration::Footnote);
    } else if (d->type == KoInlineNote::Endnote) {
        notesConfig = KoTextDocument(this->textFrame()->document()).notesConfiguration(KoOdfNotesConfiguration::Endnote);
    }
    //assigning a formatted label to notes
    QString label;
    if (d->autoNumbering)
        label = notesConfig->numberFormat().formattedNumber(d->label.toInt()+notesConfig->startValue()-1);
    else label = d->label;
    QTextLayout layout(label, font, pd);
    layout.setCacheEnabled(true);
    QList<QTextLayout::FormatRange> layouts;
    QTextLayout::FormatRange range;
    range.start = 0;
    range.length = label.length();
    range.format = format;
    range.format.setVerticalAlignment(QTextCharFormat::AlignSuperScript);
    layouts.append(range);
    layout.setAdditionalFormats(layouts);

    QTextOption option(Qt::AlignLeft | Qt::AlignAbsolute);
    option.setTextDirection(object.textDirection());
    layout.setTextOption(option);
    layout.beginLayout();
    layout.createLine();
    layout.endLayout();
    layout.draw(&painter, rect.topLeft());
    label.clear();
}

bool KoInlineNote::loadOdf(const KoXmlElement & element, KoShapeLoadingContext &context)
{
    KoTextLoader loader(context);
    QTextCursor cursor(d->textFrame);

    if (element.namespaceURI() == KoXmlNS::text && element.localName() == "note") {

        QString className = element.attributeNS(KoXmlNS::text, "note-class");
        if (className == "footnote") {
            d->type = Footnote;
        }
        else if (className == "endnote") {
            d->type = Endnote;
        }
        else {
            return false;
        }

        d->id = element.attributeNS(KoXmlNS::text, "id");
        for (KoXmlNode node = element.firstChild(); !node.isNull(); node = node.nextSibling()) {
            KoXmlElement ts = node.toElement();
            if (ts.namespaceURI() != KoXmlNS::text)
                continue;
            if (ts.localName() == "note-body") {
                loader.loadBody(ts, cursor);
            } else if (ts.localName() == "note-citation") {
                d->label = ts.attributeNS(KoXmlNS::text, "label");
                if (d->label.isEmpty()) {
                    setAutoNumbering(true);
                    d->label = ts.text();
                }
            }
        }
    }
    else if (element.namespaceURI() == KoXmlNS::office && element.localName() == "annotation") {
        d->author = element.attributeNS(KoXmlNS::text, "dc-creator");
        d->date = QDateTime::fromString(element.attributeNS(KoXmlNS::text, "dc-date"), Qt::ISODate);
        loader.loadBody(element, cursor); // would skip author and date, and do just the <text-p> and <text-list> elements
    }
    else {
        return false;
    }

    return true;
}

void KoInlineNote::saveOdf(KoShapeSavingContext & context)
{
    KoXmlWriter *writer = &context.xmlWriter();
    //save note configuration in styles.xml
    if (KoTextDocument(d->textFrame->document()).inlineTextObjectManager()->getFirstNote(d->textFrame->document()->begin())->id() == this->id()) {
        QBuffer xmlBufferFootNote, xmlBufferEndNote;
        KoXmlWriter *xmlWriter = new KoXmlWriter(&xmlBufferFootNote);

        KoTextDocument(d->textFrame->document()).notesConfiguration(KoOdfNotesConfiguration::Footnote)->saveOdf(xmlWriter);
        context.mainStyles().insertRawOdfStyles(KoGenStyles::DocumentStyles, xmlBufferFootNote.data());

        xmlWriter = new KoXmlWriter(&xmlBufferEndNote);
        KoTextDocument(d->textFrame->document()).notesConfiguration(KoOdfNotesConfiguration::Endnote)->saveOdf(xmlWriter);
        context.mainStyles().insertRawOdfStyles(KoGenStyles::DocumentStyles, xmlBufferEndNote.data());

    }

    if (d->type == Footnote || d->type == Endnote) {
        writer->startElement("text:note", false);
        if (d->type == Footnote) {
            writer->addAttribute("text:note-class", "footnote");
        } else {
            writer->addAttribute("text:note-class", "endnote");
        }
        writer->addAttribute("text:id", d->id);
        writer->startElement("text:note-citation", false);
        if (!autoNumbering()) {
            writer->addAttribute("text:label", d->label);
        }
        writer->addTextNode(d->label);
        writer->endElement();

        writer->startElement("text:note-body", false);
        KoTextWriter textWriter(context);
        QTextCursor cursor = d->textFrame->firstCursorPosition();
        QTextFragment frag = cursor.block().begin().fragment();
        cursor.setPosition(frag.position(),QTextCursor::MoveAnchor);
        cursor.setPosition(frag.position()+frag.length(),QTextCursor::MoveAnchor);
        textWriter.write(d->textFrame->document(), cursor.position(),d->textFrame->lastPosition());
        cursor.setPosition(d->textFrame->lastPosition(),QTextCursor::KeepAnchor);
        writer->endElement();

        writer->endElement();
    }
    else if (d->type == Annotation) {
        writer->startElement("office:annotation");
        if (!d->author.isEmpty()) {
            writer->startElement("dc:creator");
            writer->addTextNode(d->author);
            writer->endElement();
        }
        if (d->date.isValid()) {
            writer->startElement("dc:date");
            writer->addTextSpan(d->date.toString(Qt::ISODate));
            writer->endElement();
        }

        KoTextWriter textWriter(context);
        textWriter.write(d->textFrame->document(), d->textFrame->firstPosition(),d->textFrame->lastPosition());

        writer->endElement();
    }
}

int KoInlineNote::getPosInDocument()
{
    return d->posInDocument;
}
