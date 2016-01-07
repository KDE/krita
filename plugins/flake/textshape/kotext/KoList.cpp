/* This file is part of the KDE project
 * Copyright (C) 2008 Girish Ramakrishnan <girish@forwardbias.in>
 * Copyright (C) 2010 Thomas Zander <zander@kde.org>
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

#include "KoList.h"
#include "KoList_p.h"

#include "KoTextDocument.h"
#include "styles/KoListLevelProperties.h"
#include "styles/KoParagraphStyle.h"
#include "styles/KoStyleManager.h"

#include "TextDebug.h"

#include <QTextCursor>

KoList::KoList(const QTextDocument *document, KoListStyle *style, KoList::Type type)
    : QObject(const_cast<QTextDocument *>(document)), d(new KoListPrivate(this, document))
{
    Q_ASSERT(document);
    d->type = type;
    setStyle(style);
    KoTextDocument(document).addList(this);
}

KoList::~KoList()
{
    KoTextDocument(d->document).removeList(this);
    delete d;
}

QVector<QWeakPointer<QTextList> > KoList::textLists() const
{
    return d->textLists;
}

QVector<KoListStyle::ListIdType> KoList::textListIds() const
{
    return d->textListIds;
}

KoList *KoList::applyStyle(const QTextBlock &block, KoListStyle *style, int level)
{
    Q_ASSERT(style);
    KoTextDocument document(block.document());
    KoList *list = document.list(block);
    if (list && *list->style() == *style) {
        list->add(block, level);
        return list;
    }

    //the block was already another list but with a different style - remove block from list
    if (list)
        list->remove(block);

    // Ok, so we are now ready to add the block to another list, but which other list?
    // For headers we always want to continue from any previous header
    // For normal lists we either want to continue an adjecent list or create a new one
    if (block.blockFormat().hasProperty(KoParagraphStyle::OutlineLevel)) {
        for (QTextBlock b = block.previous();b.isValid(); b = b.previous()) {
            list = document.list(b);
            if (list && *list->style() == *style) {
                break;
            }
        }
        if (!list || *list->style() != *style) {
            list = new KoList(block.document(), style);
        }
    } else {
        list = document.list(block.previous());
        if (!list || *list->style() != *style) {
            list = document.list(block.next());
            if (!list || *list->style() != *style) {
                list = new KoList(block.document(), style);
            }
        }
    }
    list->add(block, level);
    return list;
}

void KoList::add(const QTextBlock &block, int level)
{
    if (!block.isValid())
        return;

    Q_ASSERT(level >= 0 && level <= 10);
    if (level == 0) { // fetch the first proper level we have
        level = 1; // if nothing works...
        for (int i = 1; i <= 10; i++) {
            if (d->style->hasLevelProperties(i)) {
                level = i;
                break;
            }
        }
    }
    remove(block);

    QTextList *textList = d->textLists.value(level-1).data();
    if (!textList) {
        QTextCursor cursor(block);
        QTextListFormat format = d->style->listFormat(level);
        textList = cursor.createList(format);
        format.setProperty(KoListStyle::ListId, (KoListStyle::ListIdType)(textList));
        textList->setFormat(format);
        d->textLists[level-1] = textList;
        d->textListIds[level-1] = (KoListStyle::ListIdType)textList;
    } else {
        textList->add(block);
    }

    QTextCursor cursor(block);
    QTextBlockFormat blockFormat = cursor.blockFormat();
    if (d->style->styleId()) {
        blockFormat.setProperty(KoParagraphStyle::ListStyleId, d->style->styleId());
    } else {
        blockFormat.clearProperty(KoParagraphStyle::ListStyleId);
    }
    if (d->type == KoList::TextList) {
        blockFormat.clearProperty(KoParagraphStyle::ListLevel);
    } else {
        blockFormat.setProperty(KoParagraphStyle::ListLevel, level);
    }
    cursor.setBlockFormat(blockFormat);

    d->invalidate(block);
}

void KoList::remove(const QTextBlock &block)
{
    //QTextLists are created with a blockIndent of 1. When a block is removed from a QTextList, it's blockIndent is set to (block.indent + list.indent).
    //Since we do not use Qt's indentation for lists, we need to clear the block's blockIndent, otherwise the block's style will appear as modified.
    bool clearIndent = !block.blockFormat().hasProperty(4160);

    if (QTextList *textList = block.textList()) {
        // invalidate the list before we remove the item
        // (since the list might disappear if the block is the only item)
        KoListPrivate::invalidateList(block);
        textList->remove(block);
    }
    KoListPrivate::invalidate(block);

    if (clearIndent) {
        QTextBlockFormat format = block.blockFormat();
        format.clearProperty(4160);
        QTextCursor cursor(block);
        cursor.setBlockFormat(format);
    }
}

void KoList::setStyle(KoListStyle *style)
{
    if (style == 0) {
        KoStyleManager *styleManager = KoTextDocument(d->document).styleManager();
        Q_ASSERT(styleManager);
        style = styleManager->defaultListStyle();
    }

    if (style != d->style) {
        if (d->style)
            disconnect(d->style, 0, this, 0);
        d->style = style->clone(this);
        connect(d->style, SIGNAL(styleChanged(int)), this, SLOT(styleChanged(int)));
    }

    for (int i = 0; i < d->textLists.count(); i++) {
        QTextList *textList = d->textLists.value(i).data();
        if (!textList)
            continue;
        KoListLevelProperties properties = d->style->levelProperties(i+1);
        if (properties.listId())
            d->textListIds[i] = properties.listId();
        QTextListFormat format;
        properties.applyStyle(format);
        textList->setFormat(format);
        d->invalidate(textList->item(0));
    }

    //if this list is a heading list then update the style manager with the list proprerties
    if (KoTextDocument(d->document).headingList() == this) {
        if (KoStyleManager *styleManager = KoTextDocument(d->document).styleManager()) {
            if (styleManager->outlineStyle()) {
                styleManager->outlineStyle()->copyProperties(style);
            }
        }
    }
}

KoListStyle *KoList::style() const
{
    return d->style;
}

void KoList::updateStoredList(const QTextBlock &block)
{
    if (block.textList()) {
        int level = block.textList()->format().property(KoListStyle::Level).toInt();
        QTextList *textList = block.textList();
        textList->format().setProperty(KoListStyle::ListId, (KoListStyle::ListIdType)(textList));
        d->textLists[level-1] = textList;
        d->textListIds[level-1] = (KoListStyle::ListIdType)textList;
    }
}

bool KoList::contains(QTextList *list) const
{
    return list && d->textLists.contains(list);
}

int KoList::level(const QTextBlock &block)
{
    if (!block.textList())
        return 0;
    int l = block.blockFormat().intProperty(KoParagraphStyle::ListLevel);
    if (!l) { // not a numbered-paragraph
        QTextListFormat format = block.textList()->format();
        l = format.intProperty(KoListStyle::Level);
    }
    return l;
}

KoList *KoList::listContinuedFrom() const
{
    return d->listToBeContinuedFrom;
}

void KoList::setListContinuedFrom(KoList *list)
{
    d->listToBeContinuedFrom = list;
}

//have to include this because of Q_PRIVATE_SLOT
#include "moc_KoList.cpp"
