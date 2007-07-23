/* This file is part of the KDE project
 * Copyright (C) 2007 Thomas Zander <zander@kde.org>
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
#include "Autocorrect.h"

#include <QTextBlock>
#include <klocale.h>
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
    if (m_word.isEmpty()) return;

    emit startMacro(i18n("Autocorrection"));

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

    QString link = autoDetectURL(m_word);
    if (link.isNull()) return false;

    QString trimmed = m_word.trimmed();
    int startPos = m_cursor.selectionStart();
    m_cursor.setPosition(startPos);
    m_cursor.setPosition(startPos + trimmed.length(), QTextCursor::KeepAnchor);
    
    QTextCharFormat format;
    format.setAnchor(true);
    format.setAnchorHref(link);
    format.setFontItalic(true); // TODO: formatting
    m_cursor.mergeCharFormat(format);

    m_word = m_cursor.selectedText();
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

    QString trimmed = m_word.trimmed();

    if (trimmed.length() < 3) return false;

    bool underline = (trimmed.at(0) == '_' && trimmed.at(trimmed.length() - 1) == '_');
    bool bold = (trimmed.at(0) == '*' && trimmed.at(trimmed.length() - 1) == '*');

    if (underline || bold) {
        int startPos = m_cursor.selectionStart();
        QString replacement = trimmed.mid(1, trimmed.length() - 2);
        bool foundLetterNumber = false;

        QString::ConstIterator constIter = replacement.constBegin();
        while (constIter != replacement.constEnd()) {
            if (constIter->isLetterOrNumber()) {
                foundLetterNumber = true;
                break;
            }
            constIter++;
        }

        // if no letter/number found, don't apply autocorrection like in OOo 2.x
        if (!foundLetterNumber)
            return false;

        m_cursor.setPosition(startPos);
        m_cursor.setPosition(startPos + trimmed.length(), QTextCursor::KeepAnchor);
        m_cursor.insertText(replacement);
        m_cursor.setPosition(startPos);
        m_cursor.setPosition(startPos + replacement.length(), QTextCursor::KeepAnchor);

        QTextCharFormat format;
        format.setFontUnderline(underline);
        format.setFontWeight(bold ? QFont::Bold : QFont::Normal);
        m_cursor.mergeCharFormat(format);

        // to avoid the selection being replaced by m_word
        m_word = m_cursor.selectedText();
    }
    else
        return false;

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

QString Autocorrect::autoDetectURL(const QString &_word) const
{
    QString word = _word;

    /* this method is ported from lib/kotext/KoAutoFormat.cpp KoAutoFormat::doAutoDetectUrl
     * from KOffice 1.x branch */
    // kDebug() << "link:" << word << endl;

    char link_type = 0;
    int pos = word.indexOf("http://");
    int tmp_pos = word.indexOf("https://");

    if (tmp_pos < pos && tmp_pos != -1)
          pos = tmp_pos;
    tmp_pos = word.indexOf("mailto:/");
    if ((tmp_pos < pos || pos == -1) && tmp_pos != -1)
          pos = tmp_pos;
    tmp_pos = word.indexOf("ftp://");
    if ((tmp_pos < pos || pos == -1) && tmp_pos != -1)
          pos = tmp_pos;
    tmp_pos = word.indexOf("ftp.");
    if ((tmp_pos < pos || pos == -1) && tmp_pos != -1) {
          pos = tmp_pos;
          link_type = 3;
    }
    tmp_pos = word.indexOf("file:/");
    if ((tmp_pos < pos || pos == -1) && tmp_pos != -1)
          pos = tmp_pos;
    tmp_pos = word.indexOf("news:");
    if ((tmp_pos < pos || pos == -1) && tmp_pos != -1)
          pos = tmp_pos;
    tmp_pos = word.indexOf("www.");
    if ((tmp_pos < pos || pos == -1) && tmp_pos != -1 && word.indexOf('.', tmp_pos+4) != -1 ) {
          pos = tmp_pos;
          link_type = 2;
    }
    tmp_pos = word.indexOf('@');
    if (pos == -1 && tmp_pos != -1) {
        pos = tmp_pos-1;
        QChar c;

        while (pos >= 0) {
            c = word.at(pos);
            if (c.isPunct() && c != '.' && c != '_') break;
            else --pos;
        }
        if (pos == tmp_pos - 1) // not a valid address
            pos = -1;
        else
            ++pos;
        link_type = 1;
    }

    if (pos != -1) {
        // A URL inside e.g. quotes (like "http://www.koffice.org" with the quotes) shouldn't include the quote in the URL.
	    while (!word.at(word.length()-1).isLetter() &&  !word.at(word.length()-1).isDigit() && word.at(word.length()-1) != '/')
            word.truncate(word.length() - 1);
        word.remove(0, pos);
        QString newWord = word;

        if (link_type == 1)
            newWord = QString("mailto:") + word;
        else if (link_type == 2)
            newWord = QString("http://") + word;
        else if (link_type == 3)
            newWord = QString("ftp://") + word;

        kDebug() << "newWord: " << newWord << endl;
        return newWord;
    }

    return QString();
}
