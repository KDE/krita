/* This file is part of the KDE project
 * Copyright (C) 2007, 2008 Fredy Yanardi <fyanardi@gmail.com>
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

#include "BgSpellCheck.h"
#include "SpellCheck.h"

#include <KoCharacterStyle.h>

#include <QTextDocument>
#include <QTextBlock>
#include <KDebug>

BgSpellCheck::BgSpellCheck(const Speller &speller, QObject *parent):
    BackgroundChecker(speller, parent)
{
    connect(this, SIGNAL(misspelling(const QString &, int)), this, SLOT(foundMisspelling(const QString &, int)));
    setDefaultLanguage(speller.language());
}

void BgSpellCheck::setDefaultLanguage(const QString &language)
{
    m_defaultLanguage = language;
    int index = m_defaultLanguage.indexOf('_');
    if (index > 0) {
        m_defaultCountry = m_defaultLanguage.mid(index+1);
        m_defaultLanguage = m_defaultLanguage.left(index);
    }
}

void BgSpellCheck::startRun(QTextDocument *document, int startPosition, int endPosition)
{
    m_document = document;
    m_cursor = QTextCursor(document);
    m_cursor.setPosition(startPosition);
    m_currentPosition = -1;
    m_endPosition = endPosition;
    if (m_currentPosition < m_endPosition) {
        kDebug(31000) << "Starting:" << m_currentPosition << m_endPosition;
        BackgroundChecker::start();
    }
}

QString BgSpellCheck::fetchMoreText()
{
    m_cursor.select(QTextCursor::WordUnderCursor);
    QString word = m_cursor.selectedText();
    int position = m_cursor.selectionStart();

    QString language = m_defaultLanguage;
    QString country = m_defaultCountry;
    QTextCharFormat cf = m_cursor.blockCharFormat();
    if (cf.hasProperty(KoCharacterStyle::Language))
        language = cf.property(KoCharacterStyle::Language).toString();
    if (cf.hasProperty(KoCharacterStyle::Country))
        country = cf.property(KoCharacterStyle::Country).toString();
    cf = m_cursor.charFormat();
    if (cf.hasProperty(KoCharacterStyle::Language))
        language = cf.property(KoCharacterStyle::Language).toString();
    if (cf.hasProperty(KoCharacterStyle::Country))
        country = cf.property(KoCharacterStyle::Country).toString();
    if (m_currentLanguage != language || m_currentCountry != country) {
        kDebug(31000) << "switching to language" << language << country;
        // hmm, seems we can't set country. *shrug*
        changeLanguage(language);
        m_currentLanguage = language;
        m_currentCountry = country;
    }

    // checking should end here
    if (position >= m_endPosition)
        return QString();

    // check whether we can move to next word (moveNextWord)
    // and whether we are keep selecting the same word again and again (samePosition)
    const bool moveNextWord = m_cursor.movePosition(QTextCursor::NextWord);
    const bool samePosition = m_currentPosition == position;
    if (!moveNextWord || samePosition || word.isEmpty()) {
        if (samePosition) {
            // analyze the remaining of the text in this block, whether we still have words to check
            m_cursor.movePosition(QTextCursor::EndOfBlock, QTextCursor::KeepAnchor);
            QString remaining = m_cursor.selectedText();
            // kDebug(31000) << "Remaining text from this block: " << remaining;
            int pos = m_cursor.selectionStart();
            QString::ConstIterator constIter = remaining.constBegin();
            while (constIter != remaining.constEnd()) {
                if (constIter->isLetter()) {
                    m_cursor.setPosition(++pos);
                    return word;
                }
                constIter++;
                pos++;
            }
        }

        while (true) { // found an end of text block, search for a non-empty text block
            if (word.isEmpty()) {
                // kDebug(31000) << "Empty word";
                QString space(" "); // just to avoid returning empty string which will stop bg checker
                if (!m_cursor.movePosition(QTextCursor::NextBlock))
                    return QString();
                return space;
            }
            if (m_cursor.movePosition(QTextCursor::NextBlock)) {
                if (m_cursor.block().length() == 1) // keep searching if current block is empty
                    continue;
                else // found non-empty block
                    break;
            } else {
                return QString(); // end of document, return empty string to finish background spelling
            }
        }
    }
    m_currentPosition = position;

    return word;
}

void BgSpellCheck::foundMisspelling(const QString &word, int start)
{
    // kDebug(31000) << "Mispelling: " << word << " : " << start;
    emit misspelledWord(word, m_currentPosition + start, true);
    BackgroundChecker::continueChecking();
}

#include <BgSpellCheck.moc>
