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

#include "ChangeStylesCommand.h"

#include "KoList.h"

#include <KoStyleManager.h>
#include <KoCharacterStyle.h>
#include <KoParagraphStyle.h>
#include <KoTextDocument.h>
#include <KoTextEditor.h>

#include <QTextDocument>
#include <QTextCursor>
#include <QTextBlock>

ChangeStylesCommand::ChangeStylesCommand(QTextDocument *qDoc
        , const QList<KoCharacterStyle *> &origCharacterStyles
        , const QList<KoParagraphStyle *> &origParagraphStyles
        , const QSet<int> &changedStyles
        , KUndo2Command *parent)
    : KUndo2Command(kundo2_noi18n("stylechangecommand"),parent)
    , m_origCharacterStyles(origCharacterStyles)
    , m_origParagraphStyles(origParagraphStyles)
    , m_changedStyles(changedStyles)
    , m_document(qDoc)
    , m_first(true)
{
    // TODO optimization strategy;  store the formatid of the formats we checked into
    // a qset for 'hits' and 'ignores' and avoid the copying of the format
    // (fragment.charFormat() / block.blockFormat()) when the formatId is
    // already checked previously

    KoStyleManager *sm = KoTextDocument(m_document).styleManager();
    QTextCursor cursor(m_document);
    QTextBlock block = cursor.block();
    Memento *memento = new Memento;

    while (block.isValid()) {
        memento->blockPosition = block.position();
        memento->blockParentCharFormat = block.charFormat();
        memento->blockParentFormat = KoTextDocument(m_document).frameBlockFormat();
        memento->paragraphStyleId = 0;

        if (!memento->blockParentCharFormat.isTableCellFormat()) {
            memento->blockParentCharFormat = KoTextDocument(m_document).frameCharFormat();
        }

        bool blockChanged = false;
        int id =  block.blockFormat().intProperty(KoParagraphStyle::StyleId);
        if (id > 0 && changedStyles.contains(id)) {
            KoParagraphStyle *style = sm->paragraphStyle(id);
            Q_ASSERT(style);

            // Calculate block format of direct formatting.
            memento->blockDirectFormat = block.blockFormat(); // frame + style + direct
            style->applyStyle(memento->blockParentFormat);
            clearCommonProperties(&memento->blockDirectFormat, memento->blockParentFormat);

            // Calculate char format of direct formatting.
            memento->blockDirectCharFormat = block.charFormat(); // frame + style + direct
            style->KoCharacterStyle::applyStyle(memento->blockParentCharFormat);
            style->KoCharacterStyle::ensureMinimalProperties(memento->blockParentCharFormat);
            clearCommonProperties(&memento->blockDirectCharFormat, memento->blockParentCharFormat);

            memento->paragraphStyleId = id;
            blockChanged = true;
        }

        QTextBlock::iterator iter = block.begin();
        while (!iter.atEnd()) {
            QTextFragment fragment = iter.fragment();
            QTextCharFormat cf(fragment.charFormat());
            id = cf.intProperty(KoCharacterStyle::StyleId);
            if (blockChanged || (id > 0 && changedStyles.contains(id))) {
                // create selection
                cursor.setPosition(fragment.position());
                cursor.setPosition(fragment.position() + fragment.length(), QTextCursor::KeepAnchor);
                QTextCharFormat blockCharFormat = block.charFormat(); // with old parstyle applied

                KoCharacterStyle *style = sm->characterStyle(id);
                if (style) {
                    style->applyStyle(blockCharFormat);
                    style->ensureMinimalProperties(blockCharFormat);
                }

                clearCommonProperties(&cf, blockCharFormat);

                memento->fragmentStyleId.append(id);
                memento->fragmentDirectFormats.append(cf);
                memento->fragmentCursors.append(cursor);
            }
            ++iter;
        }
        if (blockChanged || memento->fragmentCursors.length()) {
            m_mementos.append(memento);
            memento = new Memento;
        }
        block = block.next();
    }

    delete memento; // we always have one that is unused
}

ChangeStylesCommand::~ChangeStylesCommand()
{
}

void ChangeStylesCommand::redo()
{
    KUndo2Command::redo();

    if (m_first) {
        m_first = false;
        KoStyleManager *sm = KoTextDocument(m_document).styleManager();

        QTextCursor cursor(m_document);
        foreach (Memento *memento, m_mementos) {

            cursor.setPosition(memento->blockPosition);
            QTextBlock block = cursor.block();

            if (memento->paragraphStyleId > 0) {
                KoParagraphStyle *style = sm->paragraphStyle(memento->paragraphStyleId);
                Q_ASSERT(style);

                // apply paragraph style with direct formatting on top.
                style->applyStyle(memento->blockParentFormat);
                memento->blockParentFormat.merge(memento->blockDirectFormat);
                cursor.setBlockFormat(memento->blockParentFormat);

                // apply list style formatting
                if (KoTextDocument(m_document).list(block.textList())) {
                    if (style->list() == KoTextDocument(m_document).list(block.textList())) {
                        style->applyParagraphListStyle(block, memento->blockParentFormat);
                    }
                } else {
                    style->applyParagraphListStyle(block, memento->blockParentFormat);
                }

                // apply character style with direct formatting on top.
                style->KoCharacterStyle::applyStyle(memento->blockParentCharFormat);
                style->KoCharacterStyle::ensureMinimalProperties(memento->blockParentCharFormat);
                memento->blockParentCharFormat.merge(memento->blockDirectCharFormat);

                cursor.setBlockCharFormat(memento->blockParentCharFormat);
            }

            QList<QTextCharFormat>::Iterator fmtIt = memento->fragmentDirectFormats.begin();
            QList<int>::Iterator idIt = memento->fragmentStyleId.begin();
            Q_FOREACH (QTextCursor fragCursor, memento->fragmentCursors) {
                QTextCharFormat cf(block.charFormat()); // start with block formatting

                if (*idIt > 0) {
                    KoCharacterStyle *style = sm->characterStyle(*idIt);
                    if (style) {
                        style->applyStyle(cf); // possibly apply charstyle formatting
                    }
                }

                cf.merge(*fmtIt); //apply direct formatting

                fragCursor.setCharFormat(cf);

                ++idIt;
                ++fmtIt;
            }
        }
        qDeleteAll(m_mementos);
        m_mementos.clear();
    }
}

void ChangeStylesCommand::undo()
{
    KUndo2Command::undo();
}


void ChangeStylesCommand::clearCommonProperties(QTextFormat *firstFormat, const QTextFormat &secondFormat)
{
    Q_ASSERT(firstFormat);
    Q_FOREACH (int key, secondFormat.properties().keys()) {
        if (firstFormat->property(key) == secondFormat.property(key)) {
            firstFormat->clearProperty(key);
        }
    }
}

