/* This file is part of the KDE project
 * Copyright (C) 2008 Girish Ramakrishnan <girish@forwardbias.in>
 * Copyright (C) 2008 Pierre Stirnweiss <pierre.stirnweiss_koffice@gadz.org>
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

#ifndef KOLIST_P_H
#define KOLIST_P_H


#include "KoList.h"
#include "styles/KoListStyle.h"
#include "KoTextBlockData.h"

#include <QTextDocument>
#include <QTextBlock>
#include <QVector>
#include <QWeakPointer>
#include <QVariant>
#include <QTextList>

class KoListPrivate
{
public:
    KoListPrivate(KoList *q, const QTextDocument *document)
    : q(q), type(KoList::TextList), style(0), textLists(10), textListIds(10), document(document)
    {
    }

    ~KoListPrivate()
    {
    }

    static void invalidate(const QTextBlock &block)
    {
        if (KoTextBlockData *userData = dynamic_cast<KoTextBlockData*>(block.userData()))
            userData->setCounterWidth(-1.0);
    }

    static void invalidateList(const QTextBlock &block)
    {
        for (int i = 0; i < block.textList()->count(); i++) {
            if (block.textList()->item(i) != block) {
                invalidate(block.textList()->item(i));
                break;
            }
        }
    }

    void styleChanged(int level)
    {
        Q_UNUSED(level);
        q->setStyle(style);
    }

    KoList *q;
    KoList::Type type;
    KoListStyle *style;
    QVector<QWeakPointer<QTextList> > textLists;
    QVector<KoListStyle::ListIdType> textListIds;
    const QTextDocument *document;
    QMap<int, QVariant> properties;
};

#endif // KOLIST_P_H
