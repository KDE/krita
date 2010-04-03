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



//
// TODO: New Renderer Functions
////////////////////////////////////////////////////////

void renderCodeEAN13(OROPage * page, const QRectF & r, const QString & _str, int align)
{
    int val[13];
    int i = 0;

    // initialize all the values just so we can be predictable
    for (i = 0; i < 13; ++i)
        val[i] = -1;

    // verify that the passed in string is valid
    // if it's not either twelve or thirteen characters
    // then it must be invalid to begin with
    if (_str.length() != 12 && _str.length() != 13)
        return;
    // loop through and convert each char to a digit.
    // if we can't convert all characters then this is
    // an invalid number
    for (i = 0; i < _str.length(); ++i) {
        val[i] = ((QChar)_str.at(i)).digitValue();
        if (val[i] == -1)
            return;
    }

    // calculate and append the checksum value
    int old_sum = val[12]; // get the old check sum value (-1 if none was set)
    int checksum = 0;
    for (i = 0; i < 12; ++i)
        checksum += val[i] * (i % 2 ? 3 : 1);
    checksum = (checksum % 10);
    if (checksum) checksum = 10 - checksum;
    val[12] = checksum;

    // if we had an old checksum value and if it doesn't match what we came
    // up with then the string must be invalid so we will bail
    if (old_sum != -1 && old_sum != checksum)
        return;


    // lets determine some core attributes about this barcode
    qreal bar_width = 1; // the width of the base unit bar 1/100 inch

    // this is are mandatory minimum quiet zone
    qreal quiet_zone = bar_width * 10;
    if (quiet_zone < 10)
        quiet_zone = 10;

    // what kind of area do we have to work with
    qreal draw_width = r.width();
    qreal draw_height = r.height() - 2;

    // L = 95X
    // L length of barcode (excluding quite zone) in units same as X and I
    // X the width of a bar (pixels in our case)
    qreal L;

    qreal X = bar_width;

    L = (95.0 * X);

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
        qreal nqz = (draw_width - L) / 2;
        if (nqz > quiet_zone)
            quiet_zone = nqz;
    } else if (align > 1) // right
        quiet_zone = draw_width - (L + quiet_zone);
    // else if(align < 1) {} // left : do nothing

    qreal pos = r.left() + quiet_zone;
    qreal top = r.top();


    QPen pen(Qt::NoPen);
    QBrush brush(QColor("black"));

    int b = 0;
    int w = 0;

    // render open guard
    ORORect * rect = new ORORect();
    rect->setPen(pen);
    rect->setBrush(brush);
    rect->setRect(QRectF(pos, top, bar_width, draw_height));
    page->addPrimitive(rect);

    pos += (bar_width * 2.0);

    rect = new ORORect();
    rect->setPen(pen);
    rect->setBrush(brush);
    rect->setRect(QRectF(pos, top, bar_width, draw_height));
    page->addPrimitive(rect);

    pos += bar_width;

    // render first set
    for (i = 0; i < 6; ++i) {
        b = val[i+1];
        for (w = 0; w < 7; ++w) {
            if (_encodings[b][_parity[val[0]][i]][w]) {
                rect = new ORORect();
                rect->setPen(pen);
                rect->setBrush(brush);
                rect->setRect(QRectF(pos, top, bar_width, draw_height - 0.07));
                page->addPrimitive(rect);
            }
            pos += bar_width;
        }
    }

    // render center guard
    pos += bar_width;

    rect = new ORORect();
    rect->setPen(pen);
    rect->setBrush(brush);
    rect->setRect(QRectF(pos, top, bar_width, draw_height));
    page->addPrimitive(rect);

    pos += (bar_width * 2.0);

    rect = new ORORect();
    rect->setPen(pen);
    rect->setBrush(brush);
    rect->setRect(QRectF(pos, top, bar_width, draw_height));
    page->addPrimitive(rect);

    pos += (bar_width * 2.0);

    // render last set
    for (i = 0; i < 6; ++i) {
        b = val[i+7];
        for (w = 0; w < 7; ++w) {
            if (_encodings[b][RIGHTHAND][w]) {
                rect = new ORORect();
                rect->setPen(pen);
                rect->setBrush(brush);
                rect->setRect(QRectF(pos, top, bar_width, draw_height - 0.07));
                page->addPrimitive(rect);
            }
            pos += bar_width;
        }
    }

    // render close guard
    rect = new ORORect();
    rect->setPen(pen);
    rect->setBrush(brush);
    rect->setRect(QRectF(pos, top, bar_width, draw_height));
    page->addPrimitive(rect);

    pos += (bar_width * 2.0);

    rect = new ORORect();
    rect->setPen(pen);
    rect->setBrush(brush);
    rect->setRect(QRectF(pos, top, bar_width, draw_height));
    page->addPrimitive(rect);

    QString parstr = QString("%1").arg(val[0]);
    QString leftstr = QString().sprintf("%d%d%d%d%d%d",
                                        val[1], val[2], val[3], val[4], val[5], val[6]);
    QString rightstr = QString().sprintf("%d%d%d%d%d%d",
                                         val[7], val[8], val[9], val[10], val[11], val[12]);
    QFont font("Arial", 6);

    OROTextBox * tb = new OROTextBox();
    tb->setPosition(QPointF(r.left(), r.top() + draw_height - 0.12));
    tb->setSize(QSizeF(quiet_zone - 0.02, 0.12));
    tb->setFont(font);
    tb->setText(parstr);
    tb->setFlags(Qt::AlignRight | Qt::AlignTop);
    page->addPrimitive(tb);

    tb = new OROTextBox();
    tb->setPosition(QPointF(r.left() + quiet_zone + 0.03, (r.top() + draw_height) - 0.07));
    tb->setSize(QSizeF(0.42, 0.1));
    tb->setFont(font);
    tb->setText(leftstr);
    tb->setFlags(Qt::AlignHCenter | Qt::AlignTop);
    page->addPrimitive(tb);

    tb = new OROTextBox();
    tb->setPosition(QPointF(r.left() + quiet_zone + 0.5, (r.top() + draw_height) - 0.07));
    tb->setSize(QSizeF(0.42, 0.1));
    tb->setFont(font);
    tb->setText(rightstr);
    tb->setFlags(Qt::AlignHCenter | Qt::AlignTop);
    page->addPrimitive(tb);
}

void renderCodeUPCA(OROPage * page, const QRectF & r, const QString & _str, int align)
{
    int val[13];
    int i = 0;

    // initialize all the values just so we can be predictable
    for (i = 0; i < 13; ++i)
        val[i] = -1;

    // verify that the passed in string is valid
    // if it's not either twelve or thirteen characters
    // then it must be invalid to begin with
    if (_str.length() != 11 && _str.length() != 12)
        return;
    // loop through and convert each char to a digit.
    // if we can't convert all characters then this is
    // an invalid number
    val[0] = 0;
    for (i = 0; i < _str.length(); ++i) {
        val[i+1] = ((QChar)_str.at(i)).digitValue();
        if (val[i+1] == -1)
            return;
    }

    // calculate and append the checksum value
    int old_sum = val[12]; // get the old check sum value (-1 if none was set)
    int checksum = 0;
    for (i = 0; i < 12; ++i)
        checksum += val[i] * (i % 2 ? 3 : 1);
    checksum = (checksum % 10);
    if (checksum) checksum = 10 - checksum;
    val[12] = checksum;

    // if we had an old checksum value and if it doesn't match what we came
    // up with then the string must be invalid so we will bail
    if (old_sum != -1 && old_sum != checksum)
        return;


    // lets determine some core attributes about this barcode
    qreal bar_width = 1; // the width of the base unit bar

    // this is are mandatory minimum quiet zone
    qreal quiet_zone = bar_width * 10;
    if (quiet_zone < 10) quiet_zone = 10;

    // what kind of area do we have to work with
    qreal draw_width = r.width();
    qreal draw_height = r.height() - 2;

    // L = 95X
    // L length of barcode (excluding quite zone) in units same as X and I
    // X the width of a bar (pixels in our case)
    qreal L;

    qreal X = bar_width;

    L = (95.0 * X);

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
        qreal nqz = (draw_width - L) / 2;
        if (nqz > quiet_zone)
            quiet_zone = nqz;
    } else if (align > 1) // right
        quiet_zone = draw_width - (L + quiet_zone);
    // else if(align < 1) {} // left : do nothing

    qreal pos = r.left() + quiet_zone;
    qreal top = r.top();

    QPen pen(Qt::NoPen);
    QBrush brush(QColor("black"));

    int b = 0;
    int w = 0;

    // render open guard
    ORORect * rect = new ORORect();
    rect->setPen(pen);
    rect->setBrush(brush);
    rect->setRect(QRectF(pos, top, bar_width, draw_height));
    page->addPrimitive(rect);

    pos += (bar_width * 2.0);

    rect = new ORORect();
    rect->setPen(pen);
    rect->setBrush(brush);
    rect->setRect(QRectF(pos, top, bar_width, draw_height));
    page->addPrimitive(rect);

    pos += bar_width;

    // render first set
    for (i = 0; i < 6; ++i) {
        b = val[i+1];
        for (w = 0; w < 7; ++w) {
            if (_encodings[b][_parity[val[0]][i]][w]) {
                rect = new ORORect();
                rect->setPen(pen);
                rect->setBrush(brush);
                rect->setRect(QRectF(pos, top, bar_width, draw_height - (i == 0 ? 0 : 0.07)));
                page->addPrimitive(rect);
            }
            pos += bar_width;
        }
    }

    // render center guard
    pos += bar_width;
    rect = new ORORect();
    rect->setPen(pen);
    rect->setBrush(brush);
    rect->setRect(QRectF(pos, top, bar_width, draw_height));
    page->addPrimitive(rect);

    pos += (bar_width * 2.0);

    rect = new ORORect();
    rect->setPen(pen);
    rect->setBrush(brush);
    rect->setRect(QRectF(pos, top, bar_width, draw_height));
    page->addPrimitive(rect);

    pos += (bar_width * 2.0);

    // render last set
    for (i = 0; i < 6; ++i) {
        b = val[i+7];
        for (w = 0; w < 7; ++w) {
            if (_encodings[b][RIGHTHAND][w]) {
                rect = new ORORect();
                rect->setPen(pen);
                rect->setBrush(brush);
                rect->setRect(QRectF(pos, top, bar_width, draw_height - (i == 5 ? 0 : 0.07)));
                page->addPrimitive(rect);
            }
            pos += bar_width;
        }
    }

    // render close guard
    rect = new ORORect();
    rect->setPen(pen);
    rect->setBrush(brush);
    rect->setRect(QRectF(pos, top, bar_width, draw_height));
    page->addPrimitive(rect);

    pos += (bar_width * 2.0);

    rect = new ORORect();
    rect->setPen(pen);
    rect->setBrush(brush);
    rect->setRect(QRectF(pos, top, bar_width, draw_height));
    page->addPrimitive(rect);

    QString parstr = QString("%1").arg(val[1]);
    QString chkstr = QString("%1").arg(val[12]);
    QString leftstr = QString().sprintf("%d%d%d%d%d",
                                        val[2], val[3], val[4], val[5], val[6]);
    QString rightstr = QString().sprintf("%d%d%d%d%d",
                                         val[7], val[8], val[9], val[10], val[11]);

    QFont font("Arial", 6);
    KRTextStyleData ts;
    ts.backgroundColor = Qt::white;
    ts.font = font;
    ts.foregroundColor = Qt::black;
    ts.backgroundOpacity = 100;
    ts.alignment = Qt::AlignRight | Qt::AlignTop;

    OROTextBox * tb = new OROTextBox();
    tb->setPosition(QPointF(r.left(), r.top() + draw_height - 12));
    tb->setSize(QSizeF(quiet_zone - 2, 12));
    tb->setTextStyle(ts);
    tb->setText(parstr);

    page->addPrimitive(tb);

    tb = new OROTextBox();
    tb->setPosition(QPointF(r.left() + quiet_zone + 10, (r.top() + draw_height) - 7));
    tb->setSize(QSizeF(35, 10));
    tb->setTextStyle(ts);
    tb->setText(leftstr);
    page->addPrimitive(tb);

    tb = new OROTextBox();
    tb->setPosition(QPointF(r.left() + quiet_zone + 50, (r.top() + draw_height) - 7));
    tb->setSize(QSizeF(35, 10));
    tb->setTextStyle(ts);
    tb->setText(rightstr);
    page->addPrimitive(tb);

    tb = new OROTextBox();
    tb->setPosition(QPointF(r.left() + quiet_zone + L + 2, (r.top() + draw_height) - 12));
    tb->setSize(QSizeF(8, 12));
    tb->setTextStyle(ts);
    tb->setText(chkstr);
    page->addPrimitive(tb);
}

void renderCodeEAN8(OROPage * page, const QRectF & r, const QString & _str, int align)
{
    int val[8];
    int i = 0;

    // initialize all the values just so we can be predictable
    for (i = 0; i < 8; ++i)
        val[i] = -1;

    // verify that the passed in string is valid
    // if it's not either twelve or thirteen characters
    // then it must be invalid to begin with
    if (_str.length() != 7 && _str.length() != 8)
        return;
    // loop through and convert each char to a digit.
    // if we can't convert all characters then this is
    // an invalid number
    for (i = 0; i < _str.length(); ++i) {
        val[i] = ((QChar)_str.at(i)).digitValue();
        if (val[i] == -1)
            return;
    }

    // calculate and append the checksum value
    int old_sum = val[7]; // get the old check sum value (-1 if none was set)
    int checksum = 0;
    for (i = 0; i < 7; ++i)
        checksum += val[i] * (i % 2 ? 1 : 3);
    checksum = (checksum % 10);
    if (checksum) checksum = 10 - checksum;
    val[7] = checksum;

    // if we had an old checksum value and if it doesn't match what we came
    // up with then the string must be invalid so we will bail
    if (old_sum != -1 && old_sum != checksum)
        return;


    // lets determine some core attributes about this barcode
    qreal bar_width = 1; // the width of the base unit bar

    // this is are mandatory minimum quiet zone
    qreal quiet_zone = bar_width * 10;
    if (quiet_zone < 10)
        quiet_zone = 10;

    // what kind of area do we have to work with
    qreal draw_width = r.width();
    qreal draw_height = r.height() - 0.02;

    // L = 60X
    // L length of barcode (excluding quite zone) in units same as X and I
    // X the width of a bar (pixels in our case)
    qreal L;

    qreal X = bar_width;

    L = (67.0 * X);

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
        qreal nqz = (draw_width - L) / 2;
        if (nqz > quiet_zone)
            quiet_zone = nqz;
    } else if (align > 1) // right
        quiet_zone = draw_width - (L + quiet_zone);
    // else if(align < 1) {} // left : do nothing

    qreal pos = r.left() + quiet_zone;
    qreal top = r.top();

    QPen pen(Qt::NoPen);
    QBrush brush(QColor("black"));

    int b = 0;
    int w = 0;

    // render open guard
    ORORect * rect = new ORORect();
    rect->setPen(pen);
    rect->setBrush(brush);
    rect->setRect(QRectF(pos, top, bar_width, draw_height));
    page->addPrimitive(rect);

    pos += (bar_width * 2.0);

    rect = new ORORect();
    rect->setPen(pen);
    rect->setBrush(brush);
    rect->setRect(QRectF(pos, top, bar_width, draw_height));
    page->addPrimitive(rect);

    pos += bar_width;

    // render first set
    for (i = 0; i < 4; ++i) {
        b = val[i];
        for (w = 0; w < 7; ++w) {
            if (_encodings[b][LEFTHAND_ODD][w]) {
                ORORect * rect = new ORORect();
                rect->setPen(pen);
                rect->setBrush(brush);
                rect->setRect(QRectF(pos, top, bar_width, draw_height - 0.06));
                page->addPrimitive(rect);
            }
            pos += bar_width;
        }
    }

    // render center guard
    pos += bar_width;

    rect = new ORORect();
    rect->setPen(pen);
    rect->setBrush(brush);
    rect->setRect(QRectF(pos, top, bar_width, draw_height));
    page->addPrimitive(rect);

    pos += (bar_width * 2.0);

    rect = new ORORect();
    rect->setPen(pen);
    rect->setBrush(brush);
    rect->setRect(QRectF(pos, top, bar_width, draw_height));
    page->addPrimitive(rect);

    pos += (bar_width * 2.0);

    // render last set
    for (i = 0; i < 4; ++i) {
        b = val[i+4];
        for (w = 0; w < 7; ++w) {
            if (_encodings[b][RIGHTHAND][w]) {
                ORORect * rect = new ORORect();
                rect->setPen(pen);
                rect->setBrush(brush);
                rect->setRect(QRectF(pos, top, bar_width, draw_height - 0.06));
                page->addPrimitive(rect);
            }
            pos += bar_width;
        }
    }

    // render close guard
    rect = new ORORect();
    rect->setPen(pen);
    rect->setBrush(brush);
    rect->setRect(QRectF(pos, top, bar_width, draw_height));
    page->addPrimitive(rect);

    pos += (bar_width * 2.0);

    rect = new ORORect();
    rect->setPen(pen);
    rect->setBrush(brush);
    rect->setRect(QRectF(pos, top, bar_width, draw_height));
    page->addPrimitive(rect);

    QString leftstr = QString().sprintf("%d%d%d%d",
                                        val[0], val[1], val[2], val[3]);
    QString rightstr = QString().sprintf("%d%d%d%d",
                                         val[4], val[5], val[6], val[7]);
    QFont font("Arial", 6);
    OROTextBox * tb = new OROTextBox();

    tb->setPosition(QPointF(r.left() + quiet_zone + 0.03, (r.top() + draw_height) - 0.06));
    tb->setSize(QSizeF(0.28, 0.10));
    tb->setFont(font);
    tb->setText(leftstr);
    tb->setFlags(Qt::AlignHCenter | Qt::AlignTop);
    page->addPrimitive(tb);

    tb = new OROTextBox();
    tb->setPosition(QPointF(r.left() + quiet_zone + 0.36, (r.top() + draw_height) - 0.06));
    tb->setSize(QSizeF(0.28, 0.10));
    tb->setFont(font);
    tb->setText(rightstr);
    tb->setFlags(Qt::AlignHCenter | Qt::AlignTop);
    page->addPrimitive(tb);
}

void renderCodeUPCE(OROPage * page, const QRectF & r, const QString & _str, int align)
{
    int val[8];
    int i = 0;

    // initialize all the values just so we can be predictable
    for (i = 0; i < 8; ++i)
        val[i] = -1;

    // verify that the passed in string is valid
    // if it's not either twelve or thirteen characters
    // then it must be invalid to begin with
    if (_str.length() != 8)
        return;
    // loop through and convert each char to a digit.
    // if we can't convert all characters then this is
    // an invalid number
    for (i = 0; i < _str.length(); ++i) {
        val[i] = ((QChar)_str.at(i)).digitValue();
        if (val[i] == -1)
            return;
    }

    // calculate and append the checksum value
    // because everything is so messed up we don't calculate
    // the checksum and require that it be passed in already
    // however we do have to verify that the first digit is
    // either 0 or 1 as that is our parity
    if (val[0] != 0 && val[0] != 1)
        return;

    // lets determine some core attributes about this barcode
    qreal bar_width = 1; // the width of the base unit bar

    // this is are mandatory minimum quiet zone
    qreal quiet_zone = bar_width * 0.10;
    if (quiet_zone < 0.10)
        quiet_zone = 0.10;

    // what kind of area do we have to work with
    qreal draw_width = r.width();
    qreal draw_height = r.height() - 2;

    // L = 51X
    // L length of barcode (excluding quite zone) in units same as X and I
    // X the width of a bar (pixels in our case)
    qreal L;

    qreal X = bar_width;

    L = (51.0 * X);

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
        qreal nqz = (draw_width - L) / 2;
        if (nqz > quiet_zone)
            quiet_zone = nqz;
    } else if (align > 1) // right
        quiet_zone = draw_width - (L + quiet_zone);
    // else if(align < 1) {} // left : do nothing

    qreal pos = r.left() + quiet_zone;
    qreal top = r.top();

    QPen pen(Qt::NoPen);
    QBrush brush(QColor("black"));

    int b = 0;
    int w = 0;

    // render open guard
    ORORect * rect = new ORORect();
    rect->setPen(pen);
    rect->setBrush(brush);
    rect->setRect(QRectF(pos, top, bar_width, draw_height));
    page->addPrimitive(rect);

    pos += (bar_width * 2.0);

    rect = new ORORect();
    rect->setPen(pen);
    rect->setBrush(brush);
    rect->setRect(QRectF(pos, top, bar_width, draw_height));
    page->addPrimitive(rect);

    pos += bar_width;

    // render first set
    for (i = 0; i < 6; ++i) {
        b = val[i+1];
        for (w = 0; w < 7; ++w) {
            if (_encodings[b][_upcparenc[val[7]][val[0]][i]][w]) {
                rect = new ORORect();
                rect->setPen(pen);
                rect->setBrush(brush);
                rect->setRect(QRectF(pos, top, bar_width, draw_height - 7));
                page->addPrimitive(rect);
            }
            pos += bar_width;
        }
    }

    // render center guard
    pos += bar_width;

    rect = new ORORect();
    rect->setPen(pen);
    rect->setBrush(brush);
    rect->setRect(QRectF(pos, top, bar_width, draw_height));
    page->addPrimitive(rect);

    pos += (bar_width * 2.0);

    rect = new ORORect();
    rect->setPen(pen);
    rect->setBrush(brush);
    rect->setRect(QRectF(pos, top, bar_width, draw_height));
    page->addPrimitive(rect);

    pos += (bar_width * 2.0);

    // render close guard

    rect = new ORORect();
    rect->setPen(pen);
    rect->setBrush(brush);
    rect->setRect(QRectF(pos, top, bar_width, draw_height));
    page->addPrimitive(rect);

    QString parstr = QString("%1").arg(val[0]);
    QString chkstr = QString("%1").arg(val[7]);
    QString leftstr = QString().sprintf("%d%d%d%d%d%d",
                                        val[1], val[2], val[3], val[4], val[5], val[6]);
    QFont font("Arial", 6);
    KRTextStyleData ts;
    ts.backgroundColor = Qt::white;
    ts.font = font;
    ts.foregroundColor = Qt::black;
    ts.backgroundOpacity = 100;
    ts.alignment = Qt::AlignRight | Qt::AlignTop;

    OROTextBox * tb = new OROTextBox();
    tb->setPosition(QPointF(r.left(), r.top() + draw_height - 12));
    tb->setSize(QSizeF(quiet_zone - 2, 12));
    tb->setTextStyle(ts);
    tb->setText(parstr);
    page->addPrimitive(tb);

    tb = new OROTextBox();
    tb->setPosition(QPointF(r.left() + quiet_zone + 3, (r.top() + draw_height) - 7));
    tb->setSize(QSizeF(42, 10));
    tb->setTextStyle(ts);
    tb->setText(leftstr);
    page->addPrimitive(tb);

    tb = new OROTextBox();
    tb->setPosition(QPointF(r.left() + quiet_zone + L + 2, r.top() + draw_height - 12));
    tb->setSize(QSizeF(8, 12));
    tb->setTextStyle(ts);
    tb->setText(chkstr);
    page->addPrimitive(tb);
}
