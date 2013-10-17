/*
 *  Copyright (c) 2012 C. Boemann <cbo@boemann.dk>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#ifndef CHANGESTYLESCOMMAND_H
#define CHANGESTYLESCOMMAND_H

#include <kundo2command.h>

#include <QList>
#include <QSet>
#include <QTextBlockFormat>
#include <QTextCharFormat>

class KoCharacterStyle;
class KoParagraphStyle;
class QTextDocument;

class ChangeStylesCommand : public KUndo2Command
{
public:
    ChangeStylesCommand(QTextDocument *qDoc
        , const QList<KoCharacterStyle *> &origCharacterStyles
        , const QList<KoParagraphStyle *> &origParagraphStyles
        , const QSet<int> &changedStyles
        , KUndo2Command *parent);
    virtual ~ChangeStylesCommand();

    /// redo the command
    void redo();
    /// revert the actions done in redo
    void undo();
private:
    /**
     * Helper function for clearing common properties.
     *
     * Clears properties in @a firstFormat that have the same value in @a secondFormat.
     */
    void clearCommonProperties(QTextFormat *firstFormat, const QTextFormat &secondFormat);

private:
    struct Memento // documents all change to the textdocument by a single style change
    {
        QTextDocument *document;
        int blockPosition;
        int paragraphStyleId;
        QTextBlockFormat blockDirectFormat;
        QTextBlockFormat blockParentFormat;
        QTextCharFormat blockDirectCharFormat;
        QTextCharFormat blockParentCharFormat;
        QList<QTextCharFormat> fragmentDirectFormats;
        QList<QTextCursor> fragmentCursors;
        QList<int> fragmentStyleId;
    };
    QList<Memento *> m_mementos;

private:
    QList<KoCharacterStyle *> m_origCharacterStyles;
    QList<KoParagraphStyle *> m_origParagraphStyles;
    QSet<int> m_changedStyles;
    QTextDocument *m_document;
    bool m_first;
};

#endif // CHANGESTYLESCOMMAND_H
