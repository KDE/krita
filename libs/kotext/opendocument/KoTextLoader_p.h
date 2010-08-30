/* This file is part of the KDE project
 * Copyright (C) 2001-2006 David Faure <faure@kde.org>
 * Copyright (C) 2007 Sebastian Sauer <mail@dipe.org>
 * Copyright (C) 2007 Pierre Ducroquet <pinaraf@gmail.com>
 * Copyright (C) 2007-2009 Thorsten Zachmann <zachmann@kde.org>
 * Copyright (C) 2008 Girish Ramakrishnan <girish@forwardbias.in>
 * Copyright (C) 2009 KO GmbH <cbo@kogmbh.com>
 * Copyright (C) 2009 Pierre Stirnweiss <pstirnweiss@googlemail.com>
 * Copyright (C) 2010 KO GmbH <ben.martin@kogmbh.com>
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
    inline static bool isspace(ushort ch) {
        // options are ordered by likelyhood
        return ch == ' ' || ch== '\n' || ch == '\r' ||  ch == '\t';
    }

static QString normalizeWhitespace(const QString &in, bool leadingSpace)
{
    QString textstring = in;
    ushort *text = (ushort*)textstring.data(); // this detaches from the string 'in'
    int r, w = 0;
    int len = textstring.length();
    for (r = 0; r < len; ++r) {
        const ushort ch = text[r];
        // check for space, tab, line feed, carriage return
        if (isspace(ch)) {
            // if we were lead by whitespace in some parent or previous sibling element,
            // we completely collapse this space
            if (r != 0 || !leadingSpace)
                text[w++] = ' ';
            // find the end of the whitespace run
            while (r < len && isspace(text[r]))
                ++r;
            // and then record the next non-whitespace character
            if (r < len)
                text[w++] = text[r];
        } else {
            text[w++] = ch;
        }
    }
    // and now trim off the unused part of the string
    textstring.truncate(w);
    return textstring;
}
};
