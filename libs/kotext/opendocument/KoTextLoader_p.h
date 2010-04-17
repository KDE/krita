/* This file is part of the KDE project
 * Copyright (C) 2010 Thomas Zander <zander@kde.org>
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

#include <QString>


// we cannot use QString::simplifyWhiteSpace() because it removes
// leading and trailing whitespace, but such whitespace is significant
// in ODF -- so we use this function to compress sequences of space characters
// into single spaces
class KoTextLoaderP {
public:
static QString normalizeWhitespace(const QString &in, bool leadingSpace)
{
    QString text = in;
    int r, w = 0;
    int len = text.length();
    for (r = 0; r < len; ++r) {
        QChar ch = text.at(r);
        // check for space, tab, line feed, carriage return
        if (ch.unicode() == ' ' || ch.unicode() == '\t' || ch.unicode() == '\r' ||  ch.unicode() == '\n') {
            // if we were lead by whitespace in some parent or previous sibling element,
            // we completely collapse this space
            if (r != 0 || !leadingSpace)
                text[w++] = QChar(' ');
            // find the end of the whitespace run
            while (r < len && text.at(r).isSpace())
                ++r;
            // and then record the next non-whitespace character
            if (r < len)
                text[w++] = text[r];
        } else {
            text[w++] = ch;
        }
    }
    // and now trim off the unused part of the string
    text.truncate(w);
    return text;
}
};
