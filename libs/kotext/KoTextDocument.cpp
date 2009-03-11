/* This file is part of the KDE project
 * Copyright (C) 2008 Girish Ramakrishnan <girish@forwardbias.in>
 * Copyright (C) 2009 Thomas Zander <zander@kde.org>
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

#include <QTextDocument>
#include <QTextCursor>
#include <QUrl>
#include <QVariant>

#include <kdebug.h>

#include "KoTextDocument.h"
#include "styles/KoStyleManager.h"
#include "KoInlineTextObjectManager.h"
#include "KoTextDocumentLayout.h"
#include "styles/KoParagraphStyle.h"
#include "KoList.h"

const QUrl KoTextDocument::StyleManagerURL = QUrl("kotext://stylemanager");
const QUrl KoTextDocument::ListsURL = QUrl("kotext://lists");

KoTextDocument::KoTextDocument(QTextDocument *document)
: m_document(document)
{
    Q_ASSERT(m_document);
}

KoTextDocument::KoTextDocument(const QTextDocument *document)
: m_document(const_cast<QTextDocument *>(document))
{
    Q_ASSERT(m_document);
}

KoTextDocument::~KoTextDocument()
{
}

QTextDocument *KoTextDocument::document() const
{
    return m_document;
}

void KoTextDocument::setStyleManager(KoStyleManager *sm)
{
    QVariant v;
    v.setValue(sm);
    m_document->addResource(KoTextDocument::StyleManager, StyleManagerURL, v);
}

KoStyleManager *KoTextDocument::styleManager() const
{
    QVariant resource = m_document->resource(KoTextDocument::StyleManager, StyleManagerURL);
    return resource.value<KoStyleManager *>();
}

void KoTextDocument::setLists(const QList<KoList *> &lists)
{
    QVariant v;
    v.setValue(lists);
    m_document->addResource(KoTextDocument::Lists, ListsURL, v);
}

QList<KoList *> KoTextDocument::lists() const
{
    QVariant resource = m_document->resource(KoTextDocument::Lists, ListsURL);
    return resource.value<QList<KoList *> >();
}

void KoTextDocument::addList(KoList *list)
{
    Q_ASSERT(list);
    list->setParent(m_document);
    QList<KoList *> l = lists();
    if (l.contains(list))
        return;
    l.append(list);
    setLists(l);
}

void KoTextDocument::removeList(KoList *list)
{
    QList<KoList *> l = lists();
    if (l.contains(list)) {
        l.removeAll(list);
        setLists(l);
    }
}

KoList *KoTextDocument::list(const QTextBlock &block) const
{
    QTextList *textList = block.textList();
    if (!textList)
        return 0;
    return list(textList);
}

KoList *KoTextDocument::list(QTextList *textList) const
{
    // FIXME: this is horrible.
    foreach(KoList *l, lists()) {
        if (l->textLists().contains(textList))
            return l;
    }
    return 0;
}

void KoTextDocument::clearText()
{
    QTextCursor cursor(m_document);
    cursor.select(QTextCursor::Document);
    cursor.removeSelectedText();
}

KoInlineTextObjectManager *KoTextDocument::inlineTextObjectManager() const
{
    KoTextDocumentLayout *lay = dynamic_cast<KoTextDocumentLayout*>(m_document->documentLayout());
    if (lay)
        return lay->inlineObjectTextManager();
    return 0;
}
