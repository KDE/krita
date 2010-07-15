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
#include "KoTextDocument.h"
#include "styles/KoListLevelProperties.h"
#include "KoTextBlockData.h"
#include "styles/KoParagraphStyle.h"
#include "styles/KoStyleManager.h"

#include <KDebug>

#include <QTextCursor>
#include <QWeakPointer>
#include <QBitArray>

#include "KoList_p.h"


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

    if (list)
        list->remove(block);

    list = document.list(block.previous());
    if (!list || *list->style() != *style) {
        list = document.list(block.next());
        if (!list || *list->style() != *style) {
            list = new KoList(block.document(), style);
        }
    }
    list->add(block, level);
    return list;
}

void KoList::add(const QTextBlock &block, int level)
{
    if (!block.isValid())
        return;

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
        if (continueNumbering(level))
            format.setProperty(KoListStyle::ContinueNumbering, true);
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
    if (QTextList *textList = block.textList()) {
        // invalidate the list before we remove the item
        // (since the list might disappear if the block is the only item)
        KoListPrivate::invalidateList(block);
        textList->remove(block);
    }
    KoListPrivate::invalidate(block);
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
        QTextListFormat format = textList->format();
        format.setProperty(KoListStyle::ListId, (KoListStyle::ListIdType)(textList)); 
        textList->setFormat(format);
        d->textLists[level-1] = textList;
        d->textListIds[level-1] = (KoListStyle::ListIdType)textList;
    }
}

bool KoList::contains(QTextList *list) const
{
    return list && d->textLists.contains(list);
}

void KoList::setContinueNumbering(int level, bool enable)
{
    Q_ASSERT(level > 0 && level <= 10);
    level = qMax(qMin(level, 10), 1);

    QBitArray bitArray = d->properties[ContinueNumbering].toBitArray();
    if (bitArray.isEmpty())
        bitArray.resize(10);
    bitArray.setBit(level-1, enable);
    d->properties[ContinueNumbering] = bitArray;

    QTextList *textList = d->textLists.value(level-1).data();
    if (!textList)
        return;
    QTextListFormat format = textList->format();
    if (enable) {
        format.setProperty(KoListStyle::ContinueNumbering, true);
    } else {
        format.clearProperty(KoListStyle::ContinueNumbering);
    }
    textList->setFormat(format);
}

bool KoList::continueNumbering(int level) const
{
    Q_ASSERT(level > 0 && level <= 10);
    level = qMax(qMin(level, 10), 1);

    QBitArray bitArray = d->properties.value(ContinueNumbering).toBitArray();
    if (bitArray.isEmpty())
        return false;
    return bitArray.testBit(level-1);
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

#include <KoList.moc>
