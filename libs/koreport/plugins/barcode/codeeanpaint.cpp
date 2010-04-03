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
 *     This file contains the implementation of the Code EAN and similar
 * formats for rendering purposes. All this code assumes a 100dpi
 * rendering surface for it's calculations.
 */

#include <QString>
#include <QVector>
#include <QRect>
#include <QPainter>
#include <QPen>
#include <QBrush>

#include "renderobjects.h"

static const int LEFTHAND_ODD = 0;
static const int LEFTHAND_EVEN = 1;
static const int RIGHTHAND = 2;

static int _encodings[10][3][7] = {
    /*   LEFTHAND_ODD   */  /*   LEFTHAND_EVEN   */  /*     RIGHTHAND     */
    { { 0, 0, 0, 1, 1, 0, 1}, { 0, 1, 0, 0, 1, 1, 1 }, { 1, 1, 1, 0, 0, 1, 0 } }, // 0
    { { 0, 0, 1, 1, 0, 0, 1}, { 0, 1, 1, 0, 0, 1, 1 }, { 1, 1, 0, 0, 1, 1, 0 } }, // 1
    { { 0, 0, 1, 0, 0, 1, 1}, { 0, 0, 1, 1, 0, 1, 1 }, { 1, 1, 0, 1, 1, 0, 0 } }, // 2
    { { 0, 1, 1, 1, 1, 0, 1}, { 0, 1, 0, 0, 0, 0, 1 }, { 1, 0, 0, 0, 0, 1, 0 } }, // 3
    { { 0, 1, 0, 0, 0, 1, 1}, { 0, 0, 1, 1, 1, 0, 1 }, { 1, 0, 1, 1, 1, 0, 0 } }, // 4
    { { 0, 1, 1, 0, 0, 0, 1}, { 0, 1, 1, 1, 0, 0, 1 }, { 1, 0, 0, 1, 1, 1, 0 } }, // 5
    { { 0, 1, 0, 1, 1, 1, 1}, { 0, 0, 0, 0, 1, 0, 1 }, { 1, 0, 1, 0, 0, 0, 0 } }, // 6
    { { 0, 1, 1, 1, 0, 1, 1}, { 0, 0, 1, 0, 0, 0, 1 }, { 1, 0, 0, 0, 1, 0, 0 } }, // 7
    { { 0, 1, 1, 0, 1, 1, 1}, { 0, 0, 0, 1, 0, 0, 1 }, { 1, 0, 0, 1, 0, 0, 0 } }, // 8
    { { 0, 0, 0, 1, 0, 1, 1}, { 0, 0, 1, 0, 1, 1, 1 }, { 1, 1, 1, 0, 1, 0, 0 } }  // 9
};

static const int odd = LEFTHAND_ODD;
static const int even = LEFTHAND_EVEN;

static int _parity[10][6] = {
    { odd,  odd,  odd,  odd,  odd,  odd }, // 0
    { odd,  odd, even,  odd, even, even }, // 1
    { odd,  odd, even, even,  odd, even }, // 2
    { odd,  odd, even, even, even,  odd }, // 3
    { odd, even,  odd,  odd, even, even }, // 4
    { odd, even, even,  odd,  odd, even }, // 5
    { odd, even, even, even,  odd,  odd }, // 6
    { odd, even,  odd, even,  odd, even }, // 7
    { odd, even,  odd, even, even,  odd }, // 8
    { odd, even, even,  odd, even,  odd }  // 9
};

static int _upcparenc[10][2][6] = {
    /*             PARITY 0             */  /*             PARITY 1             */
    { { even, even, even,  odd,  odd,  odd }, {  odd,  odd,  odd, even, even, even } }, // 0
    { { even, even,  odd, even,  odd,  odd }, {  odd,  odd, even,  odd, even, even } }, // 1
    { { even, even,  odd,  odd, even,  odd }, {  odd,  odd, even, even,  odd, even } }, // 2
    { { even, even,  odd,  odd,  odd, even }, {  odd,  odd, even, even, even,  odd } }, // 3
    { { even,  odd, even, even,  odd,  odd }, {  odd, even,  odd,  odd, even, even } }, // 4
    { { even,  odd,  odd, even, even,  odd }, {  odd, even, even,  odd,  odd, even } }, // 5
    { { even,  odd,  odd,  odd, even, even }, {  odd, even, even, even,  odd,  odd } }, // 6
    { { even,  odd, even,  odd, even,  odd }, {  odd, even,  odd, even,  odd, even } }, // 7
    { { even,  odd, even,  odd,  odd, even }, {  odd, even,  odd, even, even,  odd } }, // 8
    { { even,  odd,  odd, even,  odd, even }, {  odd, even, even,  odd, even,  odd } }  // 9
};


void renderCodeEAN13(const QRect & r, const QString & _str, int align, QPainter * pPainter)
{
    int val[13];
    int i = 0;

    // initialize all the values just so we can be predictable
    for (i = 0; i < 13; ++i) {
        val[i] = -1;
    }

    // verify that the passed in string is valid
    // if it's not either twelve or thirteen characters
    // then it must be invalid to begin with
    if (_str.length() != 12 && _str.length() != 13) return;
    // loop through and convert each char to a digit.
    // if we can't convert all characters then this is
    // an invalid number
    for (i = 0; i < _str.length(); ++i) {
        val[i] = ((QChar) _str.at(i)).digitValue();
        if (val[i] == -1) return;
    }

    // calculate and append the checksum value
    int old_sum = val[12]; // get the old check sum value (-1 if none was set)
    int checksum = 0;
    for (i = 0; i < 12; ++i) {
        checksum += val[i] * (i % 2 ? 3 : 1);
    }
    checksum = (checksum % 10);
    if (checksum) checksum = 10 - checksum;
    val[12] = checksum;

    // if we had an old checksum value and if it doesn't match what we came
    // up with then the string must be invalid so we will bail
    if (old_sum != -1 && old_sum != checksum) return;


    // lets determine some core attributes about this barcode
    int bar_width = 1; // the width of the base unit bar

    // this is are mandatory minimum quiet zone
    int quiet_zone = bar_width * 10;
    if (quiet_zone < 10) quiet_zone = 10;

    // what kind of area do we have to work with
    int draw_width = r.width();
    int draw_height = r.height() - 2;

    // L = 95X
    // L length of barcode (excluding quite zone) in units same as X and I
    // X the width of a bar (pixels in our case)
    int L;

    int X = bar_width;

    L = (95 * X);

    // now we have the actual width the barcode will be so can determine the actual
    // size of the quiet zone (we assume we center the barcode in the given area
    // what should we do if the area is too small????
    // At the moment the way the code is written is we will always start at the minimum
    // required quiet zone if we don't have enough space.... I guess we'll just have over-run
    // to the right
    //
    // calculate the starting position based on the alignment option
    // for left align we don't need to do anything as the values are already setup for it
    if (align == 1) {   // center
        int nqz = (draw_width - L) / 2;
        if (nqz > quiet_zone) quiet_zone = nqz;
    } else if (align > 1) {  // right
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

        int b = 0, w = 0;

        // render open guard
        pPainter->fillRect(pos, top, 1, draw_height, pPainter->pen().color());
        pos += 2;
        pPainter->fillRect(pos, top, 1, draw_height, pPainter->pen().color());
        pos ++;

        // render first set
        for (i = 0; i < 6; ++i) {
            b = val[i+1];
            for (w = 0; w < 7; ++w) {
                if (_encodings[b][_parity[val[0]][i]][w]) {
                    pPainter->fillRect(pos, top, 1, draw_height - 7, pPainter->pen().color());
                }
                pos++;
            }
        }

        // render center guard
        pos++;
        pPainter->fillRect(pos, top, 1, draw_height, pPainter->pen().color());
        pos += 2;
        pPainter->fillRect(pos, top, 1, draw_height, pPainter->pen().color());
        pos += 2;

        // render last set
        for (i = 0; i < 6; ++i) {
            b = val[i+7];
            for (w = 0; w < 7; ++w) {
                if (_encodings[b][RIGHTHAND][w]) {
                    pPainter->fillRect(pos, top, 1, draw_height - 7, pPainter->pen().color());
                }
                pos++;
            }
        }

        // render close guard
        pPainter->fillRect(pos, top, 1, draw_height, pPainter->pen().color());
        pos += 2;
        pPainter->fillRect(pos, top, 1, draw_height, pPainter->pen().color());

        QString parstr = QString("%1").arg(val[0]);
        QString leftstr = QString().sprintf("%d%d%d%d%d%d",
                                            val[1], val[2], val[3], val[4], val[5], val[6]);
        QString rightstr = QString().sprintf("%d%d%d%d%d%d",
                                             val[7], val[8], val[9], val[10], val[11], val[12]);
        pPainter->setFont(QFont("Arial", 6));
        pPainter->drawText(r.left(), r.top() + draw_height - 12,
                           quiet_zone - 2, 12, Qt::AlignRight | Qt::AlignTop,
                           parstr);
        pPainter->drawText(r.left() + quiet_zone + 3,
                           (r.top() + draw_height) - 7,
                           42, 10, Qt::AlignHCenter | Qt::AlignTop,
                           leftstr);
        pPainter->drawText(r.left() + quiet_zone + 50,
                           (r.top() + draw_height) - 7,
                           42, 10, Qt::AlignHCenter | Qt::AlignTop,
                           rightstr);

        pPainter->restore();
    }
}

void renderCodeUPCA(const QRect & r, const QString & _str, int align, QPainter * pPainter)
{
    int val[13];
    int i = 0;

    // initialize all the values just so we can be predictable
    for (i = 0; i < 13; ++i) {
        val[i] = -1;
    }

    // verify that the passed in string is valid
    // if it's not either twelve or thirteen characters
    // then it must be invalid to begin with
    if (_str.length() != 11 && _str.length() != 12) return;
    // loop through and convert each char to a digit.
    // if we can't convert all characters then this is
    // an invalid number
    val[0] = 0;
    for (i = 0; i < _str.length(); ++i) {
        val[i+1] = ((QChar) _str.at(i)).digitValue();
        if (val[i+1] == -1) return;
    }

    // calculate and append the checksum value
    int old_sum = val[12]; // get the old check sum value (-1 if none was set)
    int checksum = 0;
    for (i = 0; i < 12; ++i) {
        checksum += val[i] * (i % 2 ? 3 : 1);
    }
    checksum = (checksum % 10);
    if (checksum) checksum = 10 - checksum;
    val[12] = checksum;

    // if we had an old checksum value and if it doesn't match what we came
    // up with then the string must be invalid so we will bail
    if (old_sum != -1 && old_sum != checksum) return;


    // lets determine some core attributes about this barcode
    int bar_width = 1; // the width of the base unit bar

    // this is are mandatory minimum quiet zone
    int quiet_zone = bar_width * 10;
    if (quiet_zone < 10) quiet_zone = 10;

    // what kind of area do we have to work with
    int draw_width = r.width();
    int draw_height = r.height() - 2;

    // L = 95X
    // L length of barcode (excluding quite zone) in units same as X and I
    // X the width of a bar (pixels in our case)
    int L;

    int X = bar_width;

    L = (95 * X);

    // now we have the actual width the barcode will be so can determine the actual
    // size of the quiet zone (we assume we center the barcode in the given area
    // what should we do if the area is too small????
    // At the moment the way the code is written is we will always start at the minimum
    // required quiet zone if we don't have enough space.... I guess we'll just have over-run
    // to the right
    //
    // calculate the starting position based on the alignment option
    // for left align we don't need to do anything as the values are already setup for it
    if (align == 1) {   // center
        int nqz = (draw_width - L) / 2;
        if (nqz > quiet_zone) quiet_zone = nqz;
    } else if (align > 1) {  // right
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

        int b = 0, w = 0;

        // render open guard
        pPainter->fillRect(pos, top, 1, draw_height, pPainter->pen().color());
        pos += 2;
        pPainter->fillRect(pos, top, 1, draw_height, pPainter->pen().color());
        pos ++;

        // render first set
        for (i = 0; i < 6; ++i) {
            b = val[i+1];
            for (w = 0; w < 7; ++w) {
                if (_encodings[b][_parity[val[0]][i]][w]) {
                    pPainter->fillRect(pos, top, 1, draw_height - (i == 0 ? 0 : 7), pPainter->pen().color());
                }
                pos++;
            }
        }

        // render center guard
        pos++;
        pPainter->fillRect(pos, top, 1, draw_height, pPainter->pen().color());
        pos += 2;
        pPainter->fillRect(pos, top, 1, draw_height, pPainter->pen().color());
        pos += 2;

        // render last set
        for (i = 0; i < 6; ++i) {
            b = val[i+7];
            for (w = 0; w < 7; ++w) {
                if (_encodings[b][RIGHTHAND][w]) {
                    pPainter->fillRect(pos, top, 1, draw_height - (i == 5 ? 0 : 7), pPainter->pen().color());
                }
                pos++;
            }
        }

        // render close guard
        pPainter->fillRect(pos, top, 1, draw_height, pPainter->pen().color());
        pos += 2;
        pPainter->fillRect(pos, top, 1, draw_height, pPainter->pen().color());

        QString parstr = QString("%1").arg(val[1]);
        QString chkstr = QString("%1").arg(val[12]);
        QString leftstr = QString().sprintf("%d%d%d%d%d",
                                            val[2], val[3], val[4], val[5], val[6]);
        QString rightstr = QString().sprintf("%d%d%d%d%d",
                                             val[7], val[8], val[9], val[10], val[11]);
        pPainter->setFont(QFont("Arial", 6));
        pPainter->drawText(r.left(), r.top() + draw_height - 12,
                           quiet_zone - 2, 12, Qt::AlignRight | Qt::AlignTop,
                           parstr);
        pPainter->drawText(r.left() + quiet_zone + 10,
                           (r.top() + draw_height) - 7,
                           35, 10, Qt::AlignHCenter | Qt::AlignTop,
                           leftstr);
        pPainter->drawText(r.left() + quiet_zone + 50,
                           (r.top() + draw_height) - 7,
                           35, 10, Qt::AlignHCenter | Qt::AlignTop,
                           rightstr);
        pPainter->drawText(r.left() + quiet_zone + L + 2, r.top() + draw_height - 12,
                           8, 12, Qt::AlignLeft | Qt::AlignTop,
                           chkstr);

        pPainter->restore();
    }
}

void renderCodeEAN8(const QRect & r, const QString & _str, int align, QPainter * pPainter)
{
    int val[8];
    int i = 0;

    // initialize all the values just so we can be predictable
    for (i = 0; i < 8; ++i) {
        val[i] = -1;
    }

    // verify that the passed in string is valid
    // if it's not either twelve or thirteen characters
    // then it must be invalid to begin with
    if (_str.length() != 7 && _str.length() != 8) return;
    // loop through and convert each char to a digit.
    // if we can't convert all characters then this is
    // an invalid number
    for (i = 0; i < _str.length(); ++i) {
        val[i] = ((QChar) _str.at(i)).digitValue();
        if (val[i] == -1) return;
    }

    // calculate and append the checksum value
    int old_sum = val[7]; // get the old check sum value (-1 if none was set)
    int checksum = 0;
    for (i = 0; i < 7; ++i) {
        checksum += val[i] * (i % 2 ? 1 : 3);
    }
    checksum = (checksum % 10);
    if (checksum) checksum = 10 - checksum;
    val[7] = checksum;

    // if we had an old checksum value and if it doesn't match what we came
    // up with then the string must be invalid so we will bail
    if (old_sum != -1 && old_sum != checksum) return;


    // lets determine some core attributes about this barcode
    int bar_width = 1; // the width of the base unit bar

    // this is are mandatory minimum quiet zone
    int quiet_zone = bar_width * 10;
    if (quiet_zone < 10) quiet_zone = 10;

    // what kind of area do we have to work with
    int draw_width = r.width();
    int draw_height = r.height() - 2;

    // L = 60X
    // L length of barcode (excluding quite zone) in units same as X and I
    // X the width of a bar (pixels in our case)
    int L;

    int X = bar_width;

    L = (67 * X);

    // now we have the actual width the barcode will be so can determine the actual
    // size of the quiet zone (we assume we center the barcode in the given area
    // what should we do if the area is too small????
    // At the moment the way the code is written is we will always start at the minimum
    // required quiet zone if we don't have enough space.... I guess we'll just have over-run
    // to the right
    //
    // calculate the starting position based on the alignment option
    // for left align we don't need to do anything as the values are already setup for it
    if (align == 1) {   // center
        int nqz = (draw_width - L) / 2;
        if (nqz > quiet_zone) quiet_zone = nqz;
    } else if (align > 1) {  // right
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

        int b = 0, w = 0;

        // render open guard
        pPainter->fillRect(pos, top, 1, draw_height, pPainter->pen().color());
        pos += 2;
        pPainter->fillRect(pos, top, 1, draw_height, pPainter->pen().color());
        pos ++;

        // render first set
        for (i = 0; i < 4; ++i) {
            b = val[i];
            for (w = 0; w < 7; ++w) {
                if (_encodings[b][LEFTHAND_ODD][w]) {
                    pPainter->fillRect(pos, top, 1, draw_height - 6, pPainter->pen().color());
                }
                pos++;
            }
        }

        // render center guard
        pos++;
        pPainter->fillRect(pos, top, 1, draw_height, pPainter->pen().color());
        pos += 2;
        pPainter->fillRect(pos, top, 1, draw_height, pPainter->pen().color());
        pos += 2;

        // render last set
        for (i = 0; i < 4; ++i) {
            b = val[i+4];
            for (w = 0; w < 7; ++w) {
                if (_encodings[b][RIGHTHAND][w]) {
                    pPainter->fillRect(pos, top, 1, draw_height - 6, pPainter->pen().color());
                }
                pos++;
            }
        }

        // render close guard
        pPainter->fillRect(pos, top, 1, draw_height, pPainter->pen().color());
        pos += 2;
        pPainter->fillRect(pos, top, 1, draw_height, pPainter->pen().color());

        QString leftstr = QString().sprintf("%d%d%d%d",
                                            val[0], val[1], val[2], val[3]);
        QString rightstr = QString().sprintf("%d%d%d%d",
                                             val[4], val[5], val[6], val[7]);
        pPainter->setFont(QFont("Arial", 6));
        pPainter->drawText(r.left() + quiet_zone + 3,
                           (r.top() + draw_height) - 6,
                           28, 10, Qt::AlignHCenter | Qt::AlignTop,
                           leftstr);
        pPainter->drawText(r.left() + quiet_zone + 36,
                           (r.top() + draw_height) - 6,
                           28, 10, Qt::AlignHCenter | Qt::AlignTop,
                           rightstr);

        pPainter->restore();
    }
}

void renderCodeUPCE(const QRect & r, const QString & _str, int align, QPainter * pPainter)
{
    int val[8];
    int i = 0;

    // initialize all the values just so we can be predictable
    for (i = 0; i < 8; ++i) {
        val[i] = -1;
    }

    // verify that the passed in string is valid
    // if it's not either twelve or thirteen characters
    // then it must be invalid to begin with
    if (_str.length() != 8) return;
    // loop through and convert each char to a digit.
    // if we can't convert all characters then this is
    // an invalid number
    for (i = 0; i < _str.length(); ++i) {
        val[i] = ((QChar) _str.at(i)).digitValue();
        if (val[i] == -1) return;
    }

    // calculate and append the checksum value
    // because everything is so messed up we don't calculate
    // the checksum and require that it be passed in already
    // however we do have to verify that the first digit is
    // either 0 or 1 as that is our parity
    if (val[0] != 0 && val[0] != 1) return;

    // lets determine some core attributes about this barcode
    int bar_width = 1; // the width of the base unit bar

    // this is are mandatory minimum quiet zone
    int quiet_zone = bar_width * 10;
    if (quiet_zone < 10) quiet_zone = 10;

    // what kind of area do we have to work with
    int draw_width = r.width();
    int draw_height = r.height() - 2;

    // L = 51X
    // L length of barcode (excluding quite zone) in units same as X and I
    // X the width of a bar (pixels in our case)
    int L;

    int X = bar_width;

    L = (51 * X);

    // now we have the actual width the barcode will be so can determine the actual
    // size of the quiet zone (we assume we center the barcode in the given area
    // what should we do if the area is too small????
    // At the moment the way the code is written is we will always start at the minimum
    // required quiet zone if we don't have enough space.... I guess we'll just have over-run
    // to the right
    //
    // calculate the starting position based on the alignment option
    // for left align we don't need to do anything as the values are already setup for it
    if (align == 1) {   // center
        int nqz = (draw_width - L) / 2;
        if (nqz > quiet_zone) quiet_zone = nqz;
    } else if (align > 1) {  // right
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

        int b = 0, w = 0;

        // render open guard
        pPainter->fillRect(pos, top, 1, draw_height, pPainter->pen().color());
        pos += 2;
        pPainter->fillRect(pos, top, 1, draw_height, pPainter->pen().color());
        pos ++;

        // render first set
        for (i = 0; i < 6; ++i) {
            b = val[i+1];
            for (w = 0; w < 7; ++w) {
                if (_encodings[b][_upcparenc[val[7]][val[0]][i]][w]) {
                    pPainter->fillRect(pos, top, 1, draw_height - 7, pPainter->pen().color());
                }
                pos++;
            }
        }

        // render center guard
        pos++;
        pPainter->fillRect(pos, top, 1, draw_height, pPainter->pen().color());
        pos += 2;
        pPainter->fillRect(pos, top, 1, draw_height, pPainter->pen().color());
        pos += 2;

        // render close guard
        pPainter->fillRect(pos, top, 1, draw_height, pPainter->pen().color());

        QString parstr = QString("%1").arg(val[0]);
        QString chkstr = QString("%1").arg(val[7]);
        QString leftstr = QString().sprintf("%d%d%d%d%d%d",
                                            val[1], val[2], val[3], val[4], val[5], val[6]);
        pPainter->setFont(QFont("Arial", 6));
        pPainter->drawText(r.left(), r.top() + draw_height - 12,
                           quiet_zone - 2, 12, Qt::AlignRight | Qt::AlignTop,
                           parstr);
        pPainter->drawText(r.left() + quiet_zone + 3,
                           (r.top() + draw_height) - 7,
                           42, 10, Qt::AlignHCenter | Qt::AlignTop,
                           leftstr);
        pPainter->drawText(r.left() + quiet_zone + L + 2, r.top() + draw_height - 12,
                           8, 12, Qt::AlignLeft | Qt::AlignTop,
                           chkstr);

        pPainter->restore();
    }
}

