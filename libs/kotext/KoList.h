/* This file is part of the KDE project
 * Copyright (C) 2008 Girish Ramakrishnan <girish@forwardbias.in>
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

#ifndef KOLIST_H
#define KOLIST_H

#include "styles/KoListStyle.h"

#include <QMetaType>
#include <QVector>
#include <QWeakPointer>
#include <QTextList>
#include <QTextBlock>
#include <QTextDocument>

class KoListPrivate;

/**
 * This class represents an ODF list. An ODF list may have upto 10 levels
 * Each of the levels is represented as a QTextList (QTextList does not support
 * embedded lists). There is always an associated KoListStyle that holds the
 * styling information for various level of the ODF list.
 */
class KOTEXT_EXPORT KoList : public QObject
{
    Q_OBJECT
public:
    enum Type {
        TextList,
        NumberedParagraph
    };

    /// Constructor
    KoList(const QTextDocument *document, KoListStyle *style, Type type = TextList);

    /// Destructor
    ~KoList();

    enum {
        ContinueNumbering = 1
    };

    /// Adds \a block to \a level of this list
    void add(const QTextBlock &block, int level);

    /// Removes \a block from any KoList the block is a part of
    static void remove(const QTextBlock &block);

    /**
     * Adds \a block to a list that follows \a style at \a level. If the block is
     * already a part of a list, it is removed from that list. If the block before
     * or after this block is part of a list that follows \a style, this block is
     * added to that list. If required a new KoList is created.
     * Returns the KoList that this block was added to.
    */
    static KoList *applyStyle(const QTextBlock &block, KoListStyle *style, int level);

    /// Sets the style of this list
    void setStyle(KoListStyle *style);

    /// Returns the style of this list
    KoListStyle *style() const;

    /// Return true if this list contains \a textlist
    bool contains(QTextList *textList) const;

    /// Returns the QTextLists that form this list
    QVector<QWeakPointer<QTextList> > textLists() const;

    QVector<KoListStyle::ListIdType> textListIds() const;

    void setContinueNumbering(int level, bool enable);
    bool continueNumbering(int level) const;

    static int level(const QTextBlock &block);

    /// Update the stored QTextList pointer for the given block
    void updateStoredList(const QTextBlock &block);

private:
    KoListPrivate *d;

    Q_PRIVATE_SLOT(d, void styleChanged(int))
};

Q_DECLARE_METATYPE(KoList*)
Q_DECLARE_METATYPE(QList<KoList*>)

#endif // KOLIST_H
