/* This file is part of the KDE project
 * Copyright (C) 2007 Fredy Yanardi <fyanardi@gmail.com>
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

#include "SpellCheck.h"

#include <QTextBlock>
#include <QTextDocument>

#include <KLocale>
#include <KDebug>

#include <KoCharacterStyle.h>

#include "BgSpellCheck.h"

SpellCheck::SpellCheck()
{
    m_speller = Sonnet::Speller("en_US"); // TODO: make it configurable
    m_bgSpellCheck = new BgSpellCheck(m_speller, this);
    m_enableSpellCheck = false;

    connect(m_bgSpellCheck, SIGNAL(misspelledWord(const QString &, int)), this, SLOT(highlightMisspelled(const QString &, int)));
    connect(m_bgSpellCheck, SIGNAL(done()), this, SLOT(dequeueDocument()));
}

void SpellCheck::finishedWord(QTextDocument *document, int cursorPosition)
{
    if (!m_enableSpellCheck) return;

    // TODO: don't use finished word, but use bg spell checker
    m_cursor = QTextCursor(document);
    selectWord(m_cursor, cursorPosition);
    m_word = m_cursor.selectedText();

    QTextCharFormat format;
    if (!m_bgSpellCheck->checkWord(m_word.trimmed())) {
        int startPos = m_cursor.selectionStart();
        highlightMisspelled(m_word.trimmed(), startPos);
    }
    else {
        format.setProperty(KoCharacterStyle::Spelling, false);
        m_cursor.mergeCharFormat(format);
    }

}

void SpellCheck::finishedParagraph(QTextDocument *document, int cursorPosition)
{
    Q_UNUSED(document);
    Q_UNUSED(cursorPosition);
}

void SpellCheck::checkSection(QTextDocument *document, int startPosition, int endPosition)
{
    if (!m_enableSpellCheck) return;

    m_cursor = QTextCursor(document);
    if (m_documentsQueue.isEmpty()) {
        kDebug(31000) << "Document queue is empty";
        m_bgSpellCheck->start(document, startPosition, endPosition);
    }
    else {
        m_documentsQueue.enqueue(document);
    }
}

void SpellCheck::highlightMisspelled(const QString &word, int startPosition)
{
    // TODO: Exception, such as words that contain numbers, numbers
    kDebug(31000) << "Highlighting: " << word << " : " << startPosition;
    m_cursor.setPosition(startPosition);
    m_cursor.setPosition(startPosition + word.trimmed().length(), QTextCursor::KeepAnchor);

    QTextCharFormat format;
    format.setProperty(KoCharacterStyle::Spelling, true);
    m_cursor.mergeCharFormat(format);
}

void SpellCheck::dequeueDocument()
{
    kDebug(31000) << "Dequeueing document";
    if (!m_documentsQueue.isEmpty()) {
        m_bgSpellCheck->start(m_documentsQueue.dequeue(), 0, 0);
    }
}

#include "SpellCheck.moc"
