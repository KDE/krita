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
#include "changetracker/KoChangeTracker.h"
#include "styles/KoStyleManager.h"

#include <KDebug>

#include <QTextDocumentFragment>
#include <QTextDocument>
#include <QTextCursor>
#include <QString>
#include <QTextInlineObject>
#include <QFontMetricsF>
#include <QTextOption>
#include <QDateTime>
#include <QPointer>

class KoInlineNote::Private
{
public:
    Private(KoInlineNote::Type t) : autoNumbering(false), type(t) {}
    QTextDocumentFragment text;
    QString label;
    QString id;
    QString author;
    QDateTime date;
    bool autoNumbering;
    KoInlineNote::Type type;
    QPointer<KoStyleManager> styleManager;
};

KoInlineNote::KoInlineNote(Type type)
    : d(new Private(type))
{
}

KoInlineNote::~KoInlineNote()
{
    delete d;
}

void KoInlineNote::setText(const QTextDocumentFragment &text)
{
    d->text = text;
}

void KoInlineNote::setText(const QString &text)
{
    setText(QTextDocumentFragment::fromPlainText(text));
}

void KoInlineNote::setLabel(const QString &text)
{
    d->label = text;
}

void KoInlineNote::setId(const QString &id)
{
    d->id = id;
}

QTextDocumentFragment KoInlineNote::text() const
{
    return d->text;
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

void KoInlineNote::updatePosition(const QTextDocument *document, QTextInlineObject object, int posInDocument, const QTextCharFormat &format)
{
    Q_UNUSED(document);
    Q_UNUSED(object);
    Q_UNUSED(posInDocument);
    Q_UNUSED(format);
}

void KoInlineNote::resize(const QTextDocument *document, QTextInlineObject object, int posInDocument, const QTextCharFormat &format, QPaintDevice *pd)
{
    Q_UNUSED(document);
    Q_UNUSED(posInDocument);
    if (d->label.isEmpty())
        return;
    Q_ASSERT(format.isCharFormat());
    QFontMetricsF fm(format.font(), pd);
    object.setWidth(fm.width(d->label));
    object.setAscent(fm.ascent());
    object.setDescent(fm.descent());
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

bool KoInlineNote::loadOdf(const KoXmlElement &element, KoShapeLoadingContext &context)
{
    return loadOdf(element, context, 0, 0);
}

bool KoInlineNote::loadOdf(const KoXmlElement & element, KoShapeLoadingContext &context, KoStyleManager *styleManager, KoChangeTracker *changeTracker)
{
    QTextDocument *document = new QTextDocument();
    QTextCursor cursor(document);
    KoTextDocument textDocument(document);
    textDocument.setStyleManager(styleManager);
    d->styleManager = styleManager;
    textDocument.setChangeTracker(changeTracker);

    KoTextLoader loader(context);

    if (element.namespaceURI() == KoXmlNS::text && element.localName() == "note") {

        QString className = element.attributeNS(KoXmlNS::text, "note-class");
        if (className == "footnote") {
            d->type = Footnote;
        }
        else if (className == "endnote") {
            d->type = Endnote;
        }
        else {
            delete document;
            return false;
        }

        d->id = element.attributeNS(KoXmlNS::text, "id");
        for (KoXmlNode node = element.firstChild(); !node.isNull(); node = node.nextSibling()) {
            setAutoNumbering(false);
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
        delete document;
        return false;
    }

    d->text = QTextDocumentFragment(document);
    delete document;
    return true;
}

void KoInlineNote::saveOdf(KoShapeSavingContext & context)
{
    KoXmlWriter *writer = &context.xmlWriter();
    QTextDocument *document = new QTextDocument();
    KoTextDocument textDocument(document);
    textDocument.setStyleManager(d->styleManager);

    QTextCursor cursor(document);
    cursor.insertFragment(d->text);

    if (d->type == Footnote || d->type == Endnote) {
        writer->startElement("text:note", false);
        if (d->type == Footnote)
            writer->addAttribute("text:note-class", "footnote");
        else
            writer->addAttribute("text:note-class", "endnote");
        writer->addAttribute("text:id", d->id);
        writer->startElement("text:note-citation", false);
        if (!autoNumbering())
            writer->addAttribute("text:label", d->label);
        writer->addTextNode(d->label);
        writer->endElement();

        KoTextWriter textWriter(context);
        textWriter.write(document, 0);

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
        textWriter.write(document, 0);

        writer->endElement();
    }

    delete document;
}
