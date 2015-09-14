/* This file is part of the KDE project
 * Copyright (C) 2010 KO GmbH <ben.martin@kogmbh.com>
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

#include "KoTextMeta.h"

#include <KoShapeSavingContext.h>
#include <KoXmlReader.h>
#include <KoXmlWriter.h>
#include <KoTextInlineRdf.h>

#include <QTextDocument>
#include <QTextInlineObject>
#include <QWeakPointer>

#include "TextDebug.h"

// Include Q_UNSUSED classes, for building on Windows
#include <KoShapeLoadingContext.h>

class Q_DECL_HIDDEN KoTextMeta::Private
{
public:
    Private(const QTextDocument *doc)
            : document(doc),
            posInDocument(0) { }
    const QTextDocument *document;
    int posInDocument;
    QWeakPointer<KoTextMeta> endBookmark;
    BookmarkType type;
};

KoTextMeta::KoTextMeta(const QTextDocument *document)
        : KoInlineObject(false),
        d(new Private(document))
{
    d->endBookmark.clear();
}

KoTextMeta::~KoTextMeta()
{
    delete d;
}

void KoTextMeta::saveOdf(KoShapeSavingContext &context)
{
    KoXmlWriter &writer = context.xmlWriter();

    debugText << "kom.save() this:" << (void*)this << " d->type:" << d->type;
    if (inlineRdf()) {
        debugText << "kom.save() have inline Rdf";
    }

    if (d->type == StartBookmark) {
        writer.startElement("text:meta", false);
        writer.addAttribute("text:name", "foo");

        if (inlineRdf()) {
            inlineRdf()->saveOdf(context, &writer);
        }
    } else {
        debugText << "adding endelement.";
        writer.endElement();
    }
    debugText << "kom.save() done this:" << (void*)this << " d->type:" << d->type;
}

bool KoTextMeta::loadOdf(const KoXmlElement &element, KoShapeLoadingContext &context)
{
    Q_UNUSED(element);
    Q_UNUSED(context);
    debugText << "kom.load()";
    return true;
}

void KoTextMeta::updatePosition(const QTextDocument *document, int posInDocument, const QTextCharFormat &format)
{
    Q_UNUSED(format);
    d->document = document;
    d->posInDocument = posInDocument;
}

void KoTextMeta::resize(const QTextDocument *document, QTextInlineObject &object, int posInDocument, const QTextCharFormat &format, QPaintDevice *pd)
{
    Q_UNUSED(document);
    Q_UNUSED(posInDocument);
    Q_UNUSED(format);
    Q_UNUSED(pd);
    object.setWidth(0);
    object.setAscent(0);
    object.setDescent(0);
}

void KoTextMeta::paint(QPainter &, QPaintDevice *, const QTextDocument *, const QRectF &, const QTextInlineObject &, int , const QTextCharFormat &)
{
    // nothing to paint.
}

void KoTextMeta::setType(BookmarkType type)
{
    d->type = type;
}

KoTextMeta::BookmarkType KoTextMeta::type() const
{
    return d->type;
}

void KoTextMeta::setEndBookmark(KoTextMeta *bookmark)
{
    d->type = StartBookmark;
    bookmark->d->type = EndBookmark;
    d->endBookmark = bookmark;
}

KoTextMeta *KoTextMeta::endBookmark() const
{
    return d->endBookmark.data();
}

int KoTextMeta::position() const
{
    return d->posInDocument;
}
