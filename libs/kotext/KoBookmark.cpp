/* This file is part of the KDE project
 * Copyright (C) 2007 Fredy Yanardi <fyanardi@gmail.com>
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

#include <KoShape.h>

#include <QTextDocument>
#include <QTextInlineObject>
#include <QTextList>
#include <QTextBlock>
#include <QTextCursor>

#include <KDebug>

class KoBookmark::Private {
public:
    Private(KoShape *s) : document(0), shape(s) /*, cursorPosition(0), chapterPosition(-1), pageNumber(1)*/ { }
    const QTextDocument *document;
    int posInDocument;
    KoShape *shape;
    KoBookmark *endBookmark;
    bool selection;
};

KoBookmark::KoBookmark(KoShape *shape)
    : KoInlineObject(false),
    d(new Private(shape))
{
    d->selection = false;
    d->endBookmark = 0;
}

KoBookmark::~KoBookmark()
{
    delete d;
}

void KoBookmark::updatePosition(const QTextDocument *document, QTextInlineObject object, int posInDocument, const QTextCharFormat &format) {
    Q_UNUSED(object);
    Q_UNUSED(format);
    d->document = document;
    d->posInDocument = posInDocument;
}

void KoBookmark::resize(const QTextDocument *document, QTextInlineObject object, int posInDocument, const QTextCharFormat &format, QPaintDevice *pd) {
    Q_UNUSED(object);
    Q_UNUSED(pd);
    Q_UNUSED(format);
    Q_UNUSED(document);
    Q_UNUSED(posInDocument);
}

void KoBookmark::paint (QPainter &, QPaintDevice *, const QTextDocument *, const QRectF &, QTextInlineObject , int , const QTextCharFormat &) {
    // nothing to paint.
}

void KoBookmark::setEndBookmark(KoBookmark *bookmark) {
    d->endBookmark = bookmark;
    d->selection = true;
}

KoBookmark *KoBookmark::endBookmark() {
    return d->endBookmark;
}

KoShape *KoBookmark::shape() {
    return d->shape;
}

int KoBookmark::position() {
    return d->posInDocument;
}

bool KoBookmark::hasSelection() {
    return d->selection;
}

