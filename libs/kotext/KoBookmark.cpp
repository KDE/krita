/* This file is part of the KDE project
 * Copyright (C) 2007-2008 Fredy Yanardi <fyanardi@gmail.com>
 * Copyright (C) 2011 Boudewijn Rempt <boud@kogmbh.com>
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
#include <KoShapeLoadingContext.h>
#include <KoXmlWriter.h>
#include <KoXmlReader.h>
#include <KoTextInlineRdf.h>
#include <KoInlineTextObjectManager.h>
#include <KoXmlNS.h>

#include <QTextDocument>
#include <QTextInlineObject>
#include <QTextList>
#include <QTextBlock>
#include <QTextCursor>
#include <QWeakPointer>
#include <KDebug>

class KoBookmark::Private
{
public:
    Private(const QTextDocument *doc)
        : document(doc),
          posInDocument(0) { }
    const QTextDocument *document;
    int posInDocument;
    QWeakPointer<KoBookmark> endBookmark;
    QString name;
    BookmarkType type;
};

KoBookmark::KoBookmark(const QTextDocument *document)
    : KoInlineObject(false),
      d(new Private(document))
{
    d->type = SinglePosition;
    d->endBookmark.clear();
}

KoBookmark::~KoBookmark()
{
    delete d;
}

void KoBookmark::updatePosition(const QTextDocument *document, int posInDocument, const QTextCharFormat &format)
{
    Q_UNUSED(format);
    d->document = document;
    d->posInDocument = posInDocument;
}

void KoBookmark::resize(const QTextDocument *document, QTextInlineObject object, int posInDocument, const QTextCharFormat &format, QPaintDevice *pd)
{
    Q_UNUSED(document);
    Q_UNUSED(posInDocument);
    Q_UNUSED(format);
    Q_UNUSED(pd);
    object.setWidth(0);
    object.setAscent(0);
    object.setDescent(0);
}

void KoBookmark::paint(QPainter &, QPaintDevice *, const QTextDocument *, const QRectF &, QTextInlineObject , int , const QTextCharFormat &)
{
    // nothing to paint.
}

void KoBookmark::setName(const QString &name)
{
    d->name = name;
    // Yeah... but usually, you create your startbookmark, give it a name,
    // insert it, then create your endbookmark and set the end on this. I
    // don't think this is particularly useful, but it cannot hurt.
    if (d->endBookmark) {
        d->endBookmark.data()->setName(name);
    }
}

QString KoBookmark::name() const
{
    return d->name;
}

void KoBookmark::setType(BookmarkType type)
{
    if (type == SinglePosition) {
        d->endBookmark.clear();
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
    // The spec says:
    // 19.837.5 <text:bookmark-end>
    // The text:name attribute specifies matching names for bookmarks.
    // 19.837.6 <text:bookmark-start>
    // The text:name attribute specifies matching names for bookmarks.
    // so let's set the endname to the startname.
    d->endBookmark.data()->setName(name());
}

KoBookmark *KoBookmark::endBookmark()
{
    return d->endBookmark.data();
}

int KoBookmark::position()
{
    return d->posInDocument;
}

bool KoBookmark::hasSelection()
{
    return (d->endBookmark != 0);
}

bool KoBookmark::loadOdf(const KoXmlElement &element, KoShapeLoadingContext &context)
{
    Q_UNUSED(context);

    QString bookmarkName = element.attribute("name");
    const QString localName(element.localName());

    if (manager()) {
        // For cut and paste, make sure that the name is unique.
        QString uniqBookmarkName = createUniqueBookmarkName(manager()->bookmarkManager(),
                                                            bookmarkName,
                                                            (localName == "bookmark-end"));


        d->name = uniqBookmarkName;

        if (localName == "bookmark") {
            setType(KoBookmark::SinglePosition);
        }
        else if (localName == "bookmark-start") {
            setType(KoBookmark::StartBookmark);

            // Add inline Rdf to the bookmark.
            if (element.hasAttributeNS(KoXmlNS::xhtml, "property") || element.hasAttribute("id")) {
                KoTextInlineRdf* inlineRdf = new KoTextInlineRdf(const_cast<QTextDocument*>(d->document), this);
                if (inlineRdf->loadOdf(element)) {
                    setInlineRdf(inlineRdf);
                }
                else {
                    delete inlineRdf;
                    inlineRdf = 0;
                }
            }
        }
        else if (localName == "bookmark-end") {
            setType(KoBookmark::EndBookmark);
            KoBookmark *startBookmark = manager()->bookmarkManager()->retrieveBookmark(uniqBookmarkName);
            if (startBookmark) {        // set end bookmark only if we got start bookmark (we might not have in case of broken document)
                startBookmark->setEndBookmark(this);
            } else {
                kWarning(32500) << "bookmark-end of non-existing bookmark - broken document?";
            }
        }
        else {
            // something pretty weird going on...
            return false;
        }
        return true;
    }
    return false;
}

void KoBookmark::saveOdf(KoShapeSavingContext &context)
{
    KoXmlWriter *writer = &context.xmlWriter();
    QString nodeName;
    if (d->type == SinglePosition)
        nodeName = "text:bookmark";
    else if (d->type == StartBookmark)
        nodeName = "text:bookmark-start";
    else if (d->type == EndBookmark) {
        nodeName = "text:bookmark-end";
    }
    writer->startElement(nodeName.toLatin1(), false);
    writer->addAttribute("text:name", d->name.toLatin1());

    if (d->type == StartBookmark && inlineRdf()) {
        inlineRdf()->saveOdf(context, writer);
    }
    writer->endElement();
}

QString KoBookmark::createUniqueBookmarkName(KoBookmarkManager* bmm, QString bookmarkName, bool isEndMarker)
{
    QString ret = bookmarkName;
    int uniqID = 0;

    while (true) {
        if (bmm->retrieveBookmark(ret)) {
            ret = QString("%1_%2").arg(bookmarkName).arg(++uniqID);
        } else {
            if (isEndMarker) {
                --uniqID;
                if (!uniqID)
                    ret = bookmarkName;
                else
                    ret = QString("%1_%2").arg(bookmarkName).arg(uniqID);
            }
            break;
        }
    }
    return ret;
}

