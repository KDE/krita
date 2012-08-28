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
#include <KoStyleManager.h>
#include <KoElementReference.h>
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

class KoInlineNote::Private
{
public:
    Private(KoInlineNote::Type t)
        : textFrame(0)
        , autoNumbering(false)
        , type(t)
    {
    }

    QTextDocument *document;
    QTextFrame *textFrame;
    QString label;
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

void KoInlineNote::setMotherFrame(QTextFrame *motherFrame)
{
    // We create our own subframe

    QTextCursor cursor(motherFrame->lastCursorPosition());
    QTextFrameFormat format;
    format.setProperty(KoText::SubFrameType, KoText::NoteFrameType);
    d->textFrame = cursor.insertFrame(format);
    d->document = motherFrame->document();
}

void KoInlineNote::setLabel(const QString &text)
{
    d->label = text;
}

void KoInlineNote::setAutoNumber(int autoNumber)
{
    if (d->autoNumbering) {
        KoOdfNotesConfiguration *notesConfig = 0;
        if (d->type == KoInlineNote::Footnote) {
            notesConfig = KoTextDocument(d->document).styleManager()->notesConfiguration(KoOdfNotesConfiguration::Footnote);
        } else if (d->type == KoInlineNote::Endnote) {
            notesConfig = KoTextDocument(d->document).styleManager()->notesConfiguration(KoOdfNotesConfiguration::Endnote);
        }
        d->label = notesConfig->numberFormat().formattedNumber(autoNumber + notesConfig->startValue());
    }
}

QTextFrame *KoInlineNote::textFrame() const
{
    return d->textFrame;
}

void KoInlineNote::setTextFrame(QTextFrame *textFrame)
{
    d->textFrame = textFrame;
}

QString KoInlineNote::label() const
{
    return d->label;
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
    if (d->posInDocument == -1) {
        qDebug() << "undo";
    }
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
    QTextLayout layout(d->label, font, pd);
    layout.setCacheEnabled(true);
    QList<QTextLayout::FormatRange> layouts;
    QTextLayout::FormatRange range;
    range.start = 0;
    range.length = d->label.length();
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

    if (d->type == Footnote || d->type == Endnote) {
        writer->startElement("text:note", false);
        if (d->type == Footnote) {
            writer->addAttribute("text:note-class", "footnote");
        } else {
            writer->addAttribute("text:note-class", "endnote");
        }

        writer->startElement("text:note-citation", false);
        if (!autoNumbering()) {
            writer->addAttribute("text:label", d->label);
        }
        writer->addTextNode(d->label);
        writer->endElement();

        writer->startElement("text:note-body", false);
        KoTextWriter textWriter(context);
        textWriter.write(d->document, d->textFrame->firstPosition(), d->textFrame->lastPosition());
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
        textWriter.write(d->document, d->textFrame->firstPosition(),d->textFrame->lastPosition());

        writer->endElement();
    }
}

int KoInlineNote::getPosInDocument()
{
    return d->posInDocument;
}
