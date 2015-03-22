/*
 * OpenRPT report writer and rendering engine
 * Copyright (C) 2001-2012 by OpenMFG, LLC
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
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 * Please contact info@openmfg.com with any questions on this license.
 */

/*
 *     This file contains the implementation of the interleaved 2 of 5 barcode renderer.
 * All this code assumes a 100dpi rendering surface for it's calculations.
 */

#include <QString>
#include <QRectF>
#include <QPainter>
#include <QPen>
#include <QBrush>

#include "renderobjects.h"

const char* _i2of5charmap[] = {
    "NNWWN",
    "WNNNW",
    "NWNNW",
    "WWNNN",
    "NNWNW",
    "WNWNN",
    "NWWNN",
    "NNNWW",
    "WNNWN",
    "NWNWN"
};


static QPointF addElement(OROPage * page, const QRectF &r, QPointF startPos, qreal width, bool isSpace)
{
    QPen pen(Qt::NoPen);
    QBrush brush(QColor("black"));

    if (!isSpace) {
        ORORect * rect = new ORORect();
        rect->setPen(pen);
        rect->setBrush(brush);
        rect->setRect(QRectF(startPos.x(),startPos.y(), width, r.height()));
        //rect->setRotationAxis(bc->rect.topLeft()); //todo: check this
        page->addPrimitive(rect);
    }
    return QPointF(startPos.x() + width, startPos.y());
}

static QPointF addBar(OROPage * page, const QRectF &r, QPointF startPos, qreal width)
{
    return addElement(page, r, startPos, width, false);
}
static QPointF addSpace(OROPage * page, const QRectF &r, QPointF startPos, qreal width)
{
    return addElement(page, r, startPos, width, true);
}


void renderI2of5(OROPage * page, const QRectF &r, const QString & _str, int align)
{
    QString str = _str;
    qreal narrow_bar = 1; // a narrow bar is 1/100th inch wide
    qreal bar_width_mult = 2.5; // the wide bar width multiple of the narrow bar
    qreal wide_bar = narrow_bar * bar_width_mult;

    if (str.length() % 2) {
        str = "0" + str; // padding zero if number of characters is not even
    }

    // this is our mandatory minimum quiet zone
    qreal quiet_zone = narrow_bar * 10;
    if (quiet_zone < 0.1) {
        quiet_zone = 0.1;
    }

    // what kind of area do we have to work with
    qreal draw_width = r.width();

    // how long is the value we need to encode?
    int val_length = str.length();

    // L = (C(2N+3)+6+N)X
    // L length of barcode (excluding quite zone
    // C the number of characters in the value excluding the start/stop
    // N the bar width multiple for wide bars
    // X the width of a bar (pixels in our case)
    qreal L;
    qreal C = val_length;
    qreal N = bar_width_mult;
    qreal X = narrow_bar;

    L = (C * (2.0*N + 3.0) + 6.0 + N) * X;

    // now we have the actual width the barcode will be so can determine the actual
    // size of the quiet zone (we assume we center the barcode in the given area)
    // At the moment the way the code is written is we will always start at the minimum
    // required quiet zone if we don't have enough space.... I guess we'll just have over-run
    // to the right
    //
    // calculate the starting position based on the alignment option
    if (align == 1) { // center
        qreal nqz = (draw_width - L) / 2.0;
        if (nqz > quiet_zone) {
            quiet_zone = nqz;
        }
    }
    else if (align > 1) { // right
        quiet_zone = draw_width - (L + quiet_zone);
    }
    //else if (align < 1) {} // left : do nothing

    QPointF pos(r.left() + quiet_zone, r.top());

    // start character
    pos = addBar(page, r, pos, narrow_bar);
    pos = addSpace(page, r, pos, narrow_bar);
    pos = addBar(page, r, pos, narrow_bar);
    pos = addSpace(page, r, pos, narrow_bar);

    for (int i = 0; i < str.length()-1; i+=2) {
        for (int iElt = 0; _i2of5charmap [0][iElt] != '\0'; iElt++) {
            for (int offset=0; offset<=1; offset++) {
                QChar c = str.at(i+offset);
                if (!c.isDigit()) {
                    break; // invalid character
                }

                int iChar = c.digitValue();
                qreal width = _i2of5charmap[iChar][iElt] == 'W' ? wide_bar : narrow_bar;
                pos = addElement(page, r, pos, width, offset==1);
            }
        }
    }

    // stop character
    pos = addBar(page, r, pos, wide_bar);
    pos = addSpace(page, r, pos, narrow_bar);
    pos = addBar(page, r, pos, narrow_bar);
}
