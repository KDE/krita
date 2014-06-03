/* This file is part of the KDE project
   Copyright (C) 2013 Jos van den Oever <jos@vandenoever.info>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/
#include "helpers.h"

using namespace writeodf;

namespace {
template <typename T>
void
addTab(T& e, int ref) {
    text_tab tab = e.add_text_tab();
    if (ref >= 0) {
        tab.set_text_tab_ref(ref);
    }
}
}

void writeodf::addTextSpan(group_paragraph_content& content, const QString& text, const QMap<int, int>& tabCache)
{
    int len = text.length();
    int nrSpaces = 0; // number of consecutive spaces
    bool leadingSpace = false;
    QString str;
    str.reserve(len);

    // Accumulate chars either in str or in nrSpaces (for spaces).
    // Flush str when writing a subelement (for spaces or for another reason)
    // Flush nrSpaces when encountering two or more consecutive spaces
    for (int i = 0; i < len ; ++i) {
        const QChar ch = text[i];
        ushort unicode = ch.unicode();
        if (unicode == ' ') {
            if (i == 0) {
                leadingSpace = true;
            }
            ++nrSpaces;
        } else {
            if (nrSpaces > 0) {
                // For the first space we use ' '.
                // "it is good practice to use (text:s) for the second and all following SPACE
                // characters in a sequence." (per the ODF spec)
                // however, per the HTML spec, "authors should not rely on user agents to render
                // white space immediately after a start tag or immediately before an end tag"
                // (and both we and OO.o ignore leading spaces in <text:p> or <text:h> elements...)
                if (!leadingSpace) {
                    str += ' ';
                    --nrSpaces;
                }
                if (nrSpaces > 0) {   // there are more spaces
                    if (!str.isEmpty()) {
                        content.addTextNode(str);
                    }
                    str.clear();
                    text_s s = content.add_text_s();
                    if (nrSpaces > 1) {  // it's 1 by default
                        s.set_text_c(nrSpaces);
                    }
                }
            }
            nrSpaces = 0;
            leadingSpace = false;

            switch (unicode) {
            case '\t':
                if (!str.isEmpty()) {
                    content.addTextNode(str);
                }
                str.clear();
                addTab(content, tabCache.contains(i) ?tabCache[i] + 1 :-1);
                break;
            // gracefully handle \f form feed in text input.
            // otherwise the xml will not be valid.
            // \f can be added e.g. in ascii import filter.
            case '\f':
            case '\n':
            case QChar::LineSeparator:
                if (!str.isEmpty()) {
                    content.addTextNode(str);
                }
                str.clear();
                content.add_text_line_break();
                break;
            default:
                // don't add stuff that is not allowed in xml. The stuff we need we have already handled above
                if (ch.unicode() >= 0x20) {
                    str += ch;
                }
                break;
            }
        }
    }
    // either we still have text in str or we have spaces in nrSpaces
    if (!str.isEmpty()) {
        content.addTextNode(str);
    }
    if (nrSpaces > 0) {   // there are more spaces
        text_s s = content.add_text_s();
        if (nrSpaces > 1) {  // it's 1 by default
            s.set_text_c(nrSpaces);
        }
    }
}
