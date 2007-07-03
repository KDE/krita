/* This file is part of the KDE project
 * Copyright (C) 2007 Thomas Zander <zander@kde.org>
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
#include "Autocorrect.h"

#include <QTextBlock>
#include <kdebug.h>

Autocorrect::Autocorrect() {
    m_singleSpaces = true;
    m_uppercaseFirstCharOfSentence = false;
    m_fixTwoUppercaseChars = false;
    m_autoFormatURLs = false;
    m_trimParagraphs = true;
    m_autoBoldUnderline = false;
    m_autoFractions = true;
    m_autoNumbering = false;
    m_capitalizeWeekDays = false;
    m_autoFormatBulletList = false;
    m_replaceDoubleQuotes = false;
    m_replaceSingleQuotes = false;

    // default double quote open 0x201c
    // default double quote close 0x201d
}

void Autocorrect::finishedWord(QTextDocument *document, int cursorPosition) {
    m_cursor = QTextCursor(document);
    selectWord(m_cursor, cursorPosition);
    m_word = m_cursor.selectedText();

    emit startMacro("Autocorrection");

    bool done = autoFormatURLs();
    if(!done) done = singleSpaces();
    if(!done) done = autoBoldUnderline();
    if(!done) done = autoFractions();
    if(!done) uppercaseFirstCharOfSentence();
    if(!done) fixTwoUppercaseChars();
    if(!done) autoNumbering();
    if(!done) superscriptAppendix();
    if(!done) capitalizeWeekDays();
    if(!done) autoFormatBulletList();
    if(!done) replaceDoubleQuotes();
    if(!done) replaceSingleQuotes();

    if(m_cursor.selectedText() != m_word)
        m_cursor.insertText(m_word);

    emit stopMacro();
}

void Autocorrect::finishedParagraph(QTextDocument *document, int cursorPosition) {
    if(! m_trimParagraphs) return;
    // TODO
}

// ******************** individual features;

void Autocorrect::uppercaseFirstCharOfSentence() {
    if(! m_uppercaseFirstCharOfSentence) return;

    int startPos = m_cursor.selectionStart();
    QTextBlock block = m_cursor.block();

    m_cursor.setPosition(block.position());
    m_cursor.setPosition(startPos, QTextCursor::KeepAnchor);

    QString text = m_cursor.selectedText();

    if (text.isEmpty()) // start of a paragraph
        m_word.replace(0, 1, m_word.at(0).toUpper());
    else {
        QString::ConstIterator constIter = text.constEnd();
        constIter--;

        while (constIter != text.constBegin()) {
            while (constIter != text.begin() && constIter->isSpace())
                constIter--;

            if (constIter != text.constBegin() && (*constIter == QChar('.') || *constIter == QChar('!') || *constIter == QChar('?'))) {
                m_word.replace(0, 1, m_word.at(0).toUpper());
                break;
            }
            else
                break;
        }
    }

    m_cursor.setPosition(startPos);
    m_cursor.setPosition(startPos + m_word.length(), QTextCursor::KeepAnchor);
}

void Autocorrect::fixTwoUppercaseChars() {
    if(! m_fixTwoUppercaseChars) return;
    if (m_word.length() <= 2) return;

    QChar secondChar = m_word.at(1);

    if (secondChar.isUpper()) {
        QChar thirdChar = m_word.at(2);

        if (thirdChar.isLower()) // TODO: fix two uppercase chars exceptions
            m_word.replace(1, 1, secondChar.toLower());
    }
}

bool Autocorrect::autoFormatURLs() {
    if(! m_autoFormatURLs) return false;
    // TODO
    return true;
}

bool Autocorrect::singleSpaces() {
    if(! m_singleSpaces) return false;
    if(!m_cursor.atBlockStart() && m_word.length() == 1 && m_word.at(0) == ' ') {
        // then when the prev char is also a space, don't insert one.
        QTextBlock block = m_cursor.block();
        QString text = block.text();
        if(text.at(m_cursor.position() -1 - block.position()) == ' ') {
            m_word.clear();
            return true;
        }
    }
    return false;
}

bool Autocorrect::autoBoldUnderline() {
    if(! m_autoBoldUnderline) return false;
    // TODO
    return true;
}

bool Autocorrect::autoFractions() {
    if(! m_autoFractions) return false;

    QString trimmed = m_word.trimmed();

    if (trimmed == QString("1/2"))
        m_word.replace(0, 3, QString("½"));
    else if (trimmed == QString("1/4"))
        m_word.replace(0, 3, QString("¼"));
    else if (trimmed == QString("3/4"))
        m_word.replace(0, 3, QString("¾"));
    else
        return false;

    return true;
}

void Autocorrect::autoNumbering() {
    if(! m_autoNumbering) return;
    // TODO
}

void Autocorrect::superscriptAppendix() {
    if(! m_superscriptAppendix) return;
    // TODO
}

void Autocorrect::capitalizeWeekDays() {
    if(! m_capitalizeWeekDays) return;
    // TODO
}

void Autocorrect::autoFormatBulletList() {
    if(! m_autoFormatBulletList) return;
    // TODO
}

void Autocorrect::replaceDoubleQuotes() {
    if(! m_replaceDoubleQuotes) return;
    // TODO
}

void Autocorrect::replaceSingleQuotes() {
    if(! m_replaceSingleQuotes) return;
    // TODO
}

