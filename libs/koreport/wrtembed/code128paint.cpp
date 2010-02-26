/*
 * OpenRPT report writer and rendering engine
 * Copyright (C) 2001-2007 by OpenMFG, LLC (info@openmfg.com)
 * Copyright (C) 2007-2008 by Adam Pigg (adam@piggz.co.uk)
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library.  If not, see <http://www.gnu.org/licenses/>.
 */

/*
 *     This file contains the implementation of the Code 128 barcode renderer.
 * All this code assumes a 100dpi rendering surface for it's calculations.
 */

#include <QString>
#include <QVector>
#include <QRect>
#include <QPainter>
#include <QPen>
#include <QBrush>

#include "renderobjects.h"

static const int SETA = 0;
static const int SETB = 1;
static const int SETC = 2;

static const char FNC1   = (char)130;
static const char FNC2   = (char)131;
static const char FNC3   = (char)132;
static const char FNC4   = (char)133;
static const char SHIFT  = (char)134;
static const char CODEA  = (char)135;
static const char CODEB  = (char)136;
static const char CODEC  = (char)137;
static const char STARTA = (char)138;
static const char STARTB = (char)139;
static const char STARTC = (char)140;


struct code128 {
    char codea;
    char codeb;
    char codec;

    int values[6];

    bool _null;
};

static const struct code128 _128codes[] = {
    // A ,     B ,     C ,    { B  S  B  S  B  S }, NULL? },
    { ' ',    ' ',      0,    { 2, 1, 2, 2, 2, 2 }, false },
    { '!',    '!',      1,    { 2, 2, 2, 1, 2, 2 }, false },
    { '"',    '"',      2,    { 2, 2, 2, 2, 2, 1 }, false },
    { '#',    '#',      3,    { 1, 2, 1, 2, 2, 3 }, false },
    { '$',    '$',      4,    { 1, 2, 1, 3, 2, 2 }, false },
    { '%',    '%',      5,    { 1, 3, 1, 2, 2, 2 }, false },
    { '&',    '&',      6,    { 1, 2, 2, 2, 1, 3 }, false },
    { '\'',   '\'',     7,    { 1, 2, 2, 3, 1, 2 }, false },
    { '(',    '(',      8,    { 1, 3, 2, 2, 1, 2 }, false },
    { ')',    ')',      9,    { 2, 2, 1, 2, 1, 3 }, false },
    { '*',    '*',     10,    { 2, 2, 1, 3, 1, 2 }, false },
    { '+',    '+',     11,    { 2, 3, 1, 2, 1, 2 }, false },
    { ',',    ',',     12,    { 1, 1, 2, 2, 3, 2 }, false },
    { '-',    '-',     13,    { 1, 2, 2, 1, 3, 2 }, false },
    { '.',    '.',     14,    { 1, 2, 2, 2, 3, 1 }, false },
    { '/',    '/',     15,    { 1, 1, 3, 2, 2, 2 }, false },
    { '0',    '0',     16,    { 1, 2, 3, 1, 2, 2 }, false },
    { '1',    '1',     17,    { 1, 2, 3, 2, 2, 1 }, false },
    { '2',    '2',     18,    { 2, 2, 3, 2, 1, 1 }, false },
    { '3',    '3',     19,    { 2, 2, 1, 1, 3, 2 }, false },
    { '4',    '4',     20,    { 2, 2, 1, 2, 3, 1 }, false },
    { '5',    '5',     21,    { 2, 1, 3, 2, 1, 2 }, false },
    { '6',    '6',     22,    { 2, 2, 3, 1, 1, 2 }, false },
    { '7',    '7',     23,    { 3, 1, 2, 1, 3, 1 }, false },
    { '8',    '8',     24,    { 3, 1, 1, 2, 2, 2 }, false },
    { '9',    '9',     25,    { 3, 2, 1, 1, 2, 2 }, false },
    { ':',    ':',     26,    { 3, 2, 1, 2, 2, 1 }, false },
    { ';',    ';',     27,    { 3, 1, 2, 2, 1, 2 }, false },
    { '<',    '<',     28,    { 3, 2, 2, 1, 1, 2 }, false },
    { '=',    '=',     29,    { 3, 2, 2, 2, 1, 1 }, false },
    { '>',    '>',     30,    { 2, 1, 2, 1, 2, 3 }, false },
    { '?',    '?',     31,    { 2, 1, 2, 3, 2, 1 }, false },
    { '@',    '@',     32,    { 2, 3, 2, 1, 2, 1 }, false },
    { 'A',    'A',     33,    { 1, 1, 1, 3, 2, 3 }, false },
    { 'B',    'B',     34,    { 1, 3, 1, 1, 2, 3 }, false },
    { 'C',    'C',     35,    { 1, 3, 1, 3, 2, 1 }, false },
    { 'D',    'D',     36,    { 1, 1, 2, 3, 1, 3 }, false },
    { 'E',    'E',     37,    { 1, 3, 2, 1, 1, 3 }, false },
    { 'F',    'F',     38,    { 1, 3, 2, 3, 1, 1 }, false },
    { 'G',    'G',     39,    { 2, 1, 1, 3, 1, 3 }, false },
    { 'H',    'H',     40,    { 2, 3, 1, 1, 1, 3 }, false },
    { 'I',    'I',     41,    { 2, 3, 1, 3, 1, 1 }, false },
    { 'J',    'J',     42,    { 1, 1, 2, 1, 3, 3 }, false },
    { 'K',    'K',     43,    { 1, 1, 2, 3, 3, 1 }, false },
    { 'L',    'L',     44,    { 1, 3, 2, 1, 3, 1 }, false },
    { 'M',    'M',     45,    { 1, 1, 3, 1, 2, 3 }, false },
    { 'N',    'N',     46,    { 1, 1, 3, 3, 2, 1 }, false },
    { 'O',    'O',     47,    { 1, 3, 3, 1, 2, 1 }, false },
    { 'P',    'P',     48,    { 3, 1, 3, 1, 2, 1 }, false },
    { 'Q',    'Q',     49,    { 2, 1, 1, 3, 3, 1 }, false },
    { 'R',    'R',     50,    { 2, 3, 1, 1, 3, 1 }, false },
    { 'S',    'S',     51,    { 2, 1, 3, 1, 1, 3 }, false },
    { 'T',    'T',     52,    { 2, 1, 3, 3, 1, 1 }, false },
    { 'U',    'U',     53,    { 2, 1, 3, 1, 3, 1 }, false },
    { 'V',    'V',     54,    { 3, 1, 1, 1, 2, 3 }, false },
    { 'W',    'W',     55,    { 3, 1, 1, 3, 2, 1 }, false },
    { 'X',    'X',     56,    { 3, 3, 1, 1, 2, 1 }, false },
    { 'Y',    'Y',     57,    { 3, 1, 2, 1, 1, 3 }, false },
    { 'Z',    'Z',     58,    { 3, 1, 2, 3, 1, 1 }, false },
    { '[',    '[',     59,    { 3, 3, 2, 1, 1, 1 }, false },
    { '\\',   '\\',    60,    { 3, 1, 4, 1, 1, 1 }, false },
    { ']',    ']',     61,    { 2, 2, 1, 4, 1, 1 }, false },
    { '^',    '^',     62,    { 4, 3, 1, 1, 1, 1 }, false },
    { '_',    '_',     63,    { 1, 1, 1, 2, 2, 4 }, false },
    { 0x00,   '`',     64,    { 1, 1, 1, 4, 2, 2 }, false }, // NUL
    { 0x01,   'a',     65,    { 1, 2, 1, 1, 2, 4 }, false }, // SOH
    { 0x02,   'b',     66,    { 1, 2, 1, 4, 2, 1 }, false }, // STX
    { 0x03,   'c',     67,    { 1, 4, 1, 1, 2, 2 }, false }, // ETX
    { 0x04,   'd',     68,    { 1, 4, 1, 2, 2, 1 }, false }, // EOT
    { 0x05,   'e',     69,    { 1, 1, 2, 2, 1, 4 }, false }, // ENQ
    { 0x06,   'f',     70,    { 1, 1, 2, 4, 1, 2 }, false }, // ACK
    { 0x07,   'g',     71,    { 1, 2, 2, 1, 1, 4 }, false }, // BEL
    { 0x08,   'h',     72,    { 1, 2, 2, 4, 1, 1 }, false }, // BS
    { 0x09,   'i',     73,    { 1, 4, 2, 1, 1, 2 }, false }, // HT
    { 0x0A,   'j',     74,    { 1, 4, 2, 2, 1, 1 }, false }, // LF
    { 0x0B,   'k',     75,    { 2, 4, 1, 2, 1, 1 }, false }, // VT
    { 0x0C,   'l',     76,    { 2, 2, 1, 1, 1, 4 }, false }, // FF
    { 0x0D,   'm',     77,    { 4, 1, 3, 1, 1, 1 }, false }, // CR
    { 0x0E,   'n',     78,    { 2, 4, 1, 1, 1, 2 }, false }, // SO
    { 0x0F,   'o',     79,    { 1, 3, 4, 1, 1, 1 }, false }, // SI
    { 0x10,   'p',     80,    { 1, 1, 1, 2, 4, 2 }, false }, // DLE
    { 0x11,   'q',     81,    { 1, 2, 1, 1, 4, 2 }, false }, // DC1
    { 0x12,   'r',     82,    { 1, 2, 1, 2, 4, 1 }, false }, // DC2
    { 0x13,   's',     83,    { 1, 1, 4, 2, 1, 2 }, false }, // DC3
    { 0x14,   't',     84,    { 1, 2, 4, 1, 1, 2 }, false }, // DC4
    { 0x15,   'u',     85,    { 1, 2, 4, 2, 1, 1 }, false }, // NAK
    { 0x16,   'v',     86,    { 4, 1, 1, 2, 1, 2 }, false }, // SYN
    { 0x17,   'w',     87,    { 4, 2, 1, 1, 1, 2 }, false }, // ETB
    { 0x18,   'x',     88,    { 4, 2, 1, 2, 1, 1 }, false }, // CAN
    { 0x19,   'y',     89,    { 2, 1, 2, 1, 4, 1 }, false }, // EM
    { 0x1A,   'z',     90,    { 2, 1, 4, 1, 2, 1 }, false }, // SUB
    { 0x1B,   '{',     91,    { 4, 1, 2, 1, 2, 1 }, false }, // ESC
    { 0x1C,   '|',     92,    { 1, 1, 1, 1, 4, 3 }, false }, // FS
    { 0x1D,   '}',     93,    { 1, 1, 1, 3, 4, 1 }, false }, // GS
    { 0x1E,   '~',     94,    { 1, 3, 1, 1, 4, 1 }, false }, // RS
    { 0x1F,   0x7F,    95,    { 1, 1, 4, 1, 1, 3 }, false }, // US    DEL
    { FNC3,   FNC3,    96,    { 1, 1, 4, 3, 1, 1 }, false }, // FNC3  FNC3
    { FNC2,   FNC2,    97,    { 4, 1, 1, 1, 1, 3 }, false }, // FNC2  FNC2
    { SHIFT,  SHIFT,   98,    { 4, 1, 1, 3, 1, 1 }, false }, // SHIFT SHIFT
    { CODEC,  CODEC,   99,    { 1, 1, 3, 1, 4, 1 }, false }, // CODEC CODEC
    { CODEB,  FNC4,   CODEB,  { 1, 1, 4, 1, 3, 1 }, false }, // CODEB FNC4  CODEB
    { FNC4,   CODEA,  CODEA,  { 3, 1, 1, 1, 4, 1 }, false }, // FNC4  CODEA CODEA
    { FNC1,   FNC1,   FNC1,   { 4, 1, 1, 1, 3, 1 }, false }, // FNC1  FNC1  FNC1
    { STARTA, STARTA, STARTA, { 2, 1, 1, 4, 1, 2 }, false }, // STARTA
    { STARTB, STARTB, STARTB, { 2, 1, 1, 2, 1, 4 }, false }, // STARTB
    { STARTC, STARTC, STARTC, { 2, 1, 1, 2, 3, 2 }, false }, // STARTC

    { '\0', '\0', '\0', { 0, 0, 0, 0, 0, 0 }, true } // null termininator of list
};

// STOP CHARACTER { 2 3 3 1 1 1 2 }

int code128IndexP(QChar code, int set)
{
    for (int idx = 0; _128codes[idx]._null == false; idx++) {
        if (set == SETA && _128codes[idx].codea == code.toAscii()) return idx;
        if (set == SETB && _128codes[idx].codeb == code.toAscii()) return idx;
        if (set == SETC && _128codes[idx].codec == code.toAscii()) return idx;
    }
    return -1;  // couldn't find it
}

void renderCode128(const QRect & r, const QString & _str, int align, QPainter * pPainter)
{
    QVector<int> str;
    int i = 0;

    // create the list.. if the list is empty then just set a start code and move on
    if (_str.isEmpty()) {
        str.push_back(104);
    } else {
        int rank_a = 0;
        int rank_b = 0;
        int rank_c = 0;

        QChar c;
        for (i = 0; i < _str.length(); ++i) {
            c = _str.at(i);
            rank_a += (code128IndexP(c, SETA) != -1 ? 1 : 0);
            rank_b += (code128IndexP(c, SETB) != -1 ? 1 : 0);
            rank_c += (c >= '0' && c <= '9' ? 1 : 0);
        }
        if (rank_c == _str.length() && ((rank_c % 2) == 0 || rank_c > 4)) {
            // every value in the is a digit so we are going to go with mode C
            // and we have an even number or we have more than 4 values
            i = 0;
            if ((rank_c % 2) == 1) {
                str.push_back(104); // START B
                c = _str.at(0);
                str.push_back(code128IndexP(c, SETB));
                str.push_back(99); // MODE C
                i = 1;
            } else {
                str.push_back(105); // START C
            }
            for (i = i; i < _str.length(); i += 2) {
                char a, b;
                c = _str.at(i);
                a = c.toAscii();
                a -= 48;
                c = _str.at(i + 1);
                b = c.toAscii();
                b -= 48;
                str.push_back(int((a * 10) + b));
            }
        } else {
            // start in the mode that had the higher number of hits and then
            // just shift into the opposite mode as needed
            int set = (rank_a > rank_b ? SETA : SETB);
            str.push_back((rank_a > rank_b ? 103 : 104));
            int v = -1;
            for (i = 0; i < _str.length(); ++i) {
                c = _str.at(i);
                v = code128IndexP(c, set);
                if (v == -1) {
                    v = code128IndexP(c, (set == SETA ? SETB : SETA));
                    if (v != -1) {
                        str.push_back(98); // SHIFT
                        str.push_back(v);
                    }
                } else {
                    str.push_back(v);
                }
            }
        }
    }

    // calculate and append the checksum value to the list
    int checksum = str.at(0);
    for (i = 1; i < str.size(); ++i) {
        checksum += (str.at(i) * i);
    }
    checksum = checksum % 103;
    str.push_back(checksum);

    // lets determine some core attributes about this barcode
    int bar_width = 1; // the width of the base unit bar

    // this is are mandatory minimum quiet zone
    int quiet_zone = bar_width * 10;
    if (quiet_zone < 10) quiet_zone = 10;

    // what kind of area do we have to work with
    int draw_width = r.width();
    int draw_height = r.height();

    // how long is the value we need to encode?
    int val_length = str.size() - 2; // we include start and checksum in are list so
    // subtract them out for our calculations

    // L = (11C + 35)X
    // L length of barcode (excluding quite zone) in units same as X and I
    // C the number of characters in the value excluding the start/stop and checksum characters
    // X the width of a bar (pixels in our case)
    int L;

    int C = val_length;
    int X = bar_width;

    L = (((11 * C) + 35) * X);

    // now we have the actual width the barcode will be so can determine the actual
    // size of the quiet zone (we assume we center the barcode in the given area
    // what should we do if the area is too small????
    // At the moment the way the code is written is we will always start at the minimum
    // required quiet zone if we don't have enough space.... I guess we'll just have over-run
    // to the right
    //
    // calculate the starting position based on the alignment option
    // for left align we don't need to do anything as the values are already setup for it
    if (align == 1) { // center
        int nqz = (draw_width - L) / 2;
        if (nqz > quiet_zone) quiet_zone = nqz;
    } else if (align > 1) { // right
        quiet_zone = draw_width - (L + quiet_zone);
    } // else if(align < 1) {} // left : do nothing

    int pos = r.left() + quiet_zone;
    int top = r.top();

    if (pPainter) {
        pPainter->save();

        QPen oneWide(pPainter->pen());
        oneWide.setWidth(1);
#ifndef Q_WS_WIN32
        oneWide.setJoinStyle(Qt::MiterJoin);
#endif
        pPainter->setPen(oneWide);
        pPainter->setBrush(pPainter->pen().color());
    }


    bool space = false;
    int idx = 0, b = 0, w = 0;
    for (i = 0; i < str.size(); ++i) {
        // loop through each value and render the barcode
        idx = str.at(i);
        if (idx < 0 || idx > 105) {
            qDebug("Encountered a non-compliant element while rendering a 3of9 barcode -- skipping");
            continue;
        }
        space = false;
        for (b = 0; b < 6; b++, space = !space) {
            w = _128codes[idx].values[b] * bar_width;
            if (!space && pPainter) {
                pPainter->fillRect(pos, top, w, draw_height, pPainter->pen().color());
            }
            pos += w;
        }
    }

    // we have to do the stop character separately like this because it has
    // 7 elements in it's bar sequence rather than 6 like the others
    int STOP_CHARACTER[] = { 2, 3, 3, 1, 1, 1, 2 };
    space = false;
    for (b = 0; b < 7; b++, space = !space) {
        w = STOP_CHARACTER[b] * bar_width;
        if (!space && pPainter) {
            pPainter->fillRect(pos, top, w, draw_height, pPainter->pen().color());
        }
        pos += w;
    }

    if (pPainter) {
        pPainter->restore();
    }
}
