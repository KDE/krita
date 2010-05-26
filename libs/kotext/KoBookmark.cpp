/* This file is part of the KDE project
 * Copyright (C) 2007-2008 Fredy Yanardi <fyanardi@gmail.com>
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

#include "KoBookmark.h"

#include <KoShapeSavingContext.h>
#include <KoXmlWriter.h>
#include <KoTextInlineRdf.h>

#include <QTextDocument>
#include <QTextInlineObject>
#include <QTextList>
#include <QTextBlock>
#include <QTextCursor>

#include <KDebug>

class KoBookmark::Private
{
public:
    Private(const QTextDocument *doc)
            : document(doc),
            posInDocument(0) { }
    const QTextDocument *document;
    int posInDocument;
    KoBookmark *endBookmark;
    bool selection;
    QString name;
    BookmarkType type;
};

KoBookmark::KoBookmark(const QString &name, const QTextDocument *document)
        : KoInlineObject(false),
        d(new Private(document))
{
    d->selection = false;
    d->endBookmark = 0;
    d->name = name;
}

KoBookmark::~KoBookmark()
{
    delete d;
}

void KoBookmark::saveOdf(KoShapeSavingContext &context)
{
    KoXmlWriter *writer = &context.xmlWriter();
    QString nodeName;
    if (d->type == SinglePosition)
        nodeName = "text:bookmark";
    else if (d->type == StartBookmark)
        nodeName = "text:bookmark-start";
    else if (d->type == EndBookmark)
        nodeName = "text:bookmark-end";
    writer->startElement(nodeName.toLatin1(), false);
    writer->addAttribute("text:name", d->name.toLatin1());

    if (d->type == StartBookmark && inlineRdf()) {
        inlineRdf()->saveOdf(context, writer);
    }
    writer->endElement();
}

void KoBookmark::updatePosition(const QTextDocument *document, QTextInlineObject object, int posInDocument, const QTextCharFormat &format)
{
    Q_UNUSED(object);
    Q_UNUSED(format);
    d->document = document;
    d->posInDocument = posInDocument;
}

void KoBookmark::resize(const QTextDocument *document, QTextInlineObject object, int posInDocument, const QTextCharFormat &format, QPaintDevice *pd)
{
    Q_UNUSED(object);
    Q_UNUSED(pd);
    Q_UNUSED(format);
    Q_UNUSED(document);
    Q_UNUSED(posInDocument);
}

void KoBookmark::paint(QPainter &, QPaintDevice *, const QTextDocument *, const QRectF &, QTextInlineObject , int , const QTextCharFormat &)
{
    // nothing to paint.
}

void KoBookmark::setName(const QString &name)
{
    d->name = name;
    if (d->selection)
        d->endBookmark->setName(name);
}

QString KoBookmark::name() const
{
    return d->name;
}

void KoBookmark::setType(BookmarkType type)
{
    if (type == SinglePosition) {
        d->selection = false;
        d->endBookmark = 0;
    }
    d->type = type;
}

KoBookmark::BookmarkType KoBookmark::type()
{
    return d->type;
}

void KoBookmark::setEndBookmark(KoBookmark *bookmark)
{
    d->endBookmark = bookmark;
    d->selection = true;
}

KoBookmark *KoBookmark::endBookmark()
{
    return d->endBookmark;
}

KoShape *KoBookmark::shape()
{
    return shapeForPosition(d->document, d->posInDocument);
}

int KoBookmark::position()
{
    return d->posInDocument;
}

bool KoBookmark::hasSelection()
{
    return d->selection;
}

bool KoBookmark::loadOdf(const KoXmlElement &element, KoShapeLoadingContext &context)
{
    Q_UNUSED(element);
    Q_UNUSED(context);
    // TODO
    return false;
}

