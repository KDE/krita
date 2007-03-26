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
#ifndef AUTOCORRECT_H
#define AUTOCORRECT_H

#include <KoTextEditingPlugin.h>
#include <QTextCursor>

class Autocorrect : public KoTextEditingPlugin {
public:
    Autocorrect();

    void finishedWord(QTextDocument *document, int cursorPosition);
    void finishedParagraph(QTextDocument *document, int cursorPosition);
    void checkSection(QTextDocument *document, int startPosition, int endPosition);

private:
    void uppercaseFirstCharOfSentence();
    void fixTwoUppercaseChars();
    /// @returns true if processing should stop here.
    bool autoFormatURLs();
    /// @returns true if processing should stop here.
    bool singleSpaces();
    /// @returns true if processing should stop here.
    bool autoBoldUnderline();
    /// @returns true if processing should stop here.
    bool autoFractions();
    void autoNumbering();
    void superscriptAppendix();
    void capitalizeWeekDays();
    void autoFormatBulletList();
    void replaceDoubleQuotes();
    void replaceSingleQuotes();

private:
    bool m_uppercaseFirstCharOfSentence; // convert first letter of a sentence automaticall to uppercase
    bool m_fixTwoUppercaseChars;  // convert two uppercase characters to one upper and one lowercase.
    bool m_autoFormatURLs;
    bool m_singleSpaces; // suppress double spaces.
    bool m_trimParagraphs; // remove spaces at beginning and end of paragraphs
    bool m_autoBoldUnderline; // automatically do bold and underline formatting
    bool m_autoFractions; // replace 1/2 with ½
    bool m_autoNumbering; //use autonumbering for numbered paragraphs
    bool m_superscriptAppendix; // replace 1st with 1 and a superscript "st"
    bool m_capitalizeWeekDays;
    bool m_autoFormatBulletList; // use list formatting for bulletted paragraphs.

    bool m_replaceDoubleQuotes;  // replace double quotes with typographical quotes
    bool m_replaceSingleQuotes;  // replace single quotes with typographical quotes

    QString m_word;
    QTextCursor m_cursor;
};

#endif
