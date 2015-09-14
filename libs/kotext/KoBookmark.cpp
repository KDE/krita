/* This file is part of the KDE project
 * Copyright (C) 2007-2008 Fredy Yanardi <fyanardi@gmail.com>
 * Copyright (C) 2011 Boudewijn Rempt <boud@kogmbh.com>
 * Copyright (C) 2012 C. Boemann <cbo@boemann.dk>
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
#include <KoXmlReader.h>
#include <KoTextInlineRdf.h>
#include <KoTextRangeManager.h>
#include <KoXmlNS.h>

#include <QTextDocument>
#include <QTextBlock>
#include <QTextCursor>
#include "TextDebug.h"

// Include Q_UNSUSED classes, for building on Windows
#include <KoShapeLoadingContext.h>

class Q_DECL_HIDDEN KoBookmark::Private
{
public:
    Private(const QTextDocument *doc)
        : document(doc)
    {
    }
    const QTextDocument *document;
    QString name;
};

KoBookmark::KoBookmark(const QTextCursor &cursor)
    : KoTextRange(cursor),
      d(new Private(cursor.block().document()))
{
}

KoBookmark::~KoBookmark()
{
    delete d;
}

void KoBookmark::setName(const QString &name)
{
    d->name = name;
}

QString KoBookmark::name() const
{
    return d->name;
}

bool KoBookmark::loadOdf(const KoXmlElement &element, KoShapeLoadingContext &context)
{
    Q_UNUSED(context);

    QString bookmarkName = element.attribute("name");
    const QString localName(element.localName());

    if (manager()) {
        // For cut and paste, make sure that the name is unique.
        d->name = createUniqueBookmarkName(manager()->bookmarkManager(), bookmarkName, false);

        if (localName == "bookmark" || localName == "bookmark-start") {
            setPositionOnlyMode(localName == "bookmark");

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
        else {
            // NOTE: "bookmark-end" is handled in KoTextLoader
            // if we ever come here then something pretty weird is going on...
            return false;
        }
        return true;
    }
    return false;
}

void KoBookmark::saveOdf(KoShapeSavingContext &context, int position, TagType tagType) const
{
    KoXmlWriter *writer = &context.xmlWriter();

    if (!hasRange()) {
        if (tagType == StartTag) {
            writer->startElement("text:bookmark", false);
            writer->addAttribute("text:name", d->name.toUtf8());
            if (inlineRdf()) {
                inlineRdf()->saveOdf(context, writer);
            }
            writer->endElement();
        }
    } else if ((tagType == StartTag) && (position == rangeStart())) {
        writer->startElement("text:bookmark-start", false);
        writer->addAttribute("text:name", d->name.toUtf8());
        if (inlineRdf()) {
            inlineRdf()->saveOdf(context, writer);
        }
        writer->endElement();
    } else if ((tagType == EndTag) && (position == rangeEnd())) {
        writer->startElement("text:bookmark-end", false);
        writer->addAttribute("text:name", d->name.toUtf8());
        writer->endElement();
    }
    // else nothing
}

QString KoBookmark::createUniqueBookmarkName(const KoBookmarkManager* bmm, QString bookmarkName, bool isEndMarker)
{
    QString ret = bookmarkName;
    int uniqID = 0;

    while (true) {
        if (bmm->bookmark(ret)) {
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

