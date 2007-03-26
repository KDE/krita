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
}

void Autocorrect::finishedParagraph(QTextDocument *document, int cursorPosition) {
    if(! m_trimParagraphs) return;
    // TODO
}

void Autocorrect::checkSection(QTextDocument *document, int startPosition, int endPosition) {
    // TODO
}

// ******************** individual features;

void Autocorrect::uppercaseFirstCharOfSentence() {
    if(! m_uppercaseFirstCharOfSentence) return;
    // TODO
}

void Autocorrect::fixTwoUppercaseChars() {
    if(! m_fixTwoUppercaseChars) return;
    // TODO
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
    // TODO
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

