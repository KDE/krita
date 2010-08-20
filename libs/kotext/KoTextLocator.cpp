/* This file is part of the KDE project
 * Copyright (C) 2007 Thomas Zander <zander@kde.org>
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

#include "KoTextLocator.h"
#include "KoTextBlockData.h"
#include "KoTextShapeData.h"
#include "KoTextReference.h"
#include "KoTextPage.h"
#include "styles/KoListStyle.h"

#include <KoShape.h>
#include <KoShapeLoadingContext.h>

#include <KDebug>
#include <QTextDocument>
#include <QTextList>
#include <QTextInlineObject>
#include <QTextBlock>
#include <QTextCursor>

class KoTextLocator::Private
{
public:
    Private(KoTextLocator *q) : q(q), document(0), dirty(false), cursorPosition(0), chapterPosition(-1), pageNumber(0) { }
    void update() {
        if (dirty == false)
            return;
        dirty = false;
        chapterPosition = -1;

        int pageTmp = pageNumber, chapterTmp = chapterPosition;
        if (document == 0)
            return;

        QTextBlock block = document->findBlock(cursorPosition);
        while (block.isValid()) {
            QTextList *list = block.textList();
            if (list) {
                QTextListFormat lf = list->format();
                int level = lf.intProperty(KoListStyle::Level);
                if (level == 1) {
                    chapterPosition = block.position();
                    break;
                }
            }
            block = block.previous();
        }

        KoShape *shape = shapeForPosition(document, cursorPosition);
        if (shape == 0)
            pageNumber = -1;
        else {
            KoTextShapeData *data = static_cast<KoTextShapeData*>(shape->userData());
            KoTextPage* page = data->page();
            pageNumber = page->pageNumber();
        }
        if (pageTmp != pageNumber || chapterTmp != chapterPosition) {
            foreach(KoTextReference* reference, listeners)
                reference->variableMoved(0, 0, 0);
        }
    }

    KoTextLocator *q;
    const QTextDocument *document;
    bool dirty;
    int cursorPosition;
    int chapterPosition;
    int pageNumber;

    QList<KoTextReference*> listeners;
};


KoTextLocator::KoTextLocator()
        : KoInlineObject(false),
        d(new Private(this))
{
}

KoTextLocator::~KoTextLocator()
{
    delete d;
}

void KoTextLocator::updatePosition(const QTextDocument *document, QTextInlineObject object, int posInDocument, const QTextCharFormat &format)
{
    Q_UNUSED(object);
    Q_UNUSED(format);
    if (d->document != document || d->cursorPosition != posInDocument) {
        d->dirty = true;
        d->document = document;
        d->cursorPosition = posInDocument;
//kDebug(32500) <<"KoTextLocator page:" << pageNumber() <<", chapter:" << chapter() <<", '" << word() <<"'";
    }
}

void KoTextLocator::resize(const QTextDocument *document, QTextInlineObject object, int posInDocument, const QTextCharFormat &format, QPaintDevice *pd)
{
    Q_UNUSED(object);
    Q_UNUSED(pd);
    Q_UNUSED(format);
    Q_UNUSED(document);
    Q_UNUSED(posInDocument);
}

void KoTextLocator::paint(QPainter &, QPaintDevice *, const QTextDocument *, const QRectF &, QTextInlineObject , int , const QTextCharFormat &)
{
    // nothing to paint.
}

QString KoTextLocator::chapter() const
{
    d->update();
    if (d->chapterPosition < 0)
        return QString();
    QTextBlock block = d->document->findBlock(d->chapterPosition);
    return block.text().remove(QChar::ObjectReplacementCharacter);
}

KoTextBlockData *KoTextLocator::chapterBlockData() const
{
    d->update();
    if (d->chapterPosition < 0)
        return 0;
    QTextBlock block = d->document->findBlock(d->chapterPosition);
    return dynamic_cast<KoTextBlockData*>(block.userData());
}

int KoTextLocator::pageNumber() const
{
    d->update();
    return d->pageNumber;
}

int KoTextLocator::indexPosition() const
{
    return d->cursorPosition;
}

QString KoTextLocator::word() const
{
    if (d->document == 0) // layout never started
        return QString();
    QTextCursor cursor(const_cast<QTextDocument*>(d->document));
    cursor.setPosition(d->cursorPosition);
    cursor.movePosition(QTextCursor::NextWord);
    cursor.movePosition(QTextCursor::WordLeft, QTextCursor::KeepAnchor);
    return cursor.selectedText().trimmed().remove(QChar::ObjectReplacementCharacter);
}

void KoTextLocator::addListener(KoTextReference *reference)
{
    d->listeners.append(reference);
}

void KoTextLocator::removeListener(KoTextReference *reference)
{
    d->listeners.removeAll(reference);
}

bool KoTextLocator::loadOdf(const KoXmlElement &element, KoShapeLoadingContext &context)
{
    Q_UNUSED(element);
    Q_UNUSED(context);
    // TODO
    return false;
}

void KoTextLocator::saveOdf(KoShapeSavingContext &context)
{
    Q_UNUSED(context);
    // TODO
}
