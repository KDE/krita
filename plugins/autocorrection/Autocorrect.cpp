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

#include <QTextCursor>
#include <kdebug.h>

Autocorrect::Autocorrect() {
    m_replaceQuotes = true;
}

void Autocorrect::finishedWord(QTextDocument *document, int cursorPosition) {
    if(m_replaceQuotes) {
        QTextCursor cursor(document);
        selectWord(cursor, cursorPosition);

        QString text = cursor.selectedText();
        text = text.replace('\"', QChar(0x201c)); // qoute open
        //word.replace('\"', QChar(0x201d)); // qoute close
        if(cursor.selectedText() != text)
            cursor.insertText(text);
    }
}

void Autocorrect::finishedParagraph(QTextDocument *document, int cursorPosition) {
    kDebug() << "Autocorrect::finishedParagraph\n";
}
