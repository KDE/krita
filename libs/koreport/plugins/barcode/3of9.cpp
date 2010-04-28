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
 *     This file contains the implementation of the 3of9 barcode renderer.
 * All this code assumes a 100dpi rendering surface for it's calculations.
 */

#include <QString>
#include <QRectF>
#include <QPainter>
#include <QPen>
#include <QBrush>
#include <kdebug.h>

#include "renderobjects.h"

struct code3of9 {
    char code;
    int values[9];
};

const struct code3of9 _3of9codes[] = {
    { '0', { 0, 0, 0, 1, 1, 0, 1, 0, 0 } },
    { '1', { 1, 0, 0, 1, 0, 0, 0, 0, 1 } },
    { '2', { 0, 0, 1, 1, 0, 0, 0, 0, 1 } },
    { '3', { 1, 0, 1, 1, 0, 0, 0, 0, 0 } },
    { '4', { 0, 0, 0, 1, 1, 0, 0, 0, 1 } },
    { '5', { 1, 0, 0, 1, 1, 0, 0, 0, 0 } },
    { '6', { 0, 0, 1, 1, 1, 0, 0, 0, 0 } },
    { '7', { 0, 0, 0, 1, 0, 0, 1, 0, 1 } },
    { '8', { 1, 0, 0, 1, 0, 0, 1, 0, 0 } },
    { '9', { 0, 0, 1, 1, 0, 0, 1, 0, 0 } },

    { 'A', { 1, 0, 0, 0, 0, 1, 0, 0, 1 } },
    { 'B', { 0, 0, 1, 0, 0, 1, 0, 0, 1 } },
    { 'C', { 1, 0, 1, 0, 0, 1, 0, 0, 0 } },
    { 'D', { 0, 0, 0, 0, 1, 1, 0, 0, 1 } },
    { 'E', { 1, 0, 0, 0, 1, 1, 0, 0, 0 } },
    { 'F', { 0, 0, 1, 0, 1, 1, 0, 0, 0 } },
    { 'G', { 0, 0, 0, 0, 0, 1, 1, 0, 1 } },
    { 'H', { 1, 0, 0, 0, 0, 1, 1, 0, 0 } },
    { 'I', { 0, 0, 1, 0, 0, 1, 1, 0, 0 } },
    { 'J', { 0, 0, 0, 0, 1, 1, 1, 0, 0 } },
    { 'K', { 1, 0, 0, 0, 0, 0, 0, 1, 1 } },
    { 'L', { 0, 0, 1, 0, 0, 0, 0, 1, 1 } },
    { 'M', { 1, 0, 1, 0, 0, 0, 0, 1, 0 } },
    { 'N', { 0, 0, 0, 0, 1, 0, 0, 1, 1 } },
    { 'O', { 1, 0, 0, 0, 1, 0, 0, 1, 0 } },
    { 'P', { 0, 0, 1, 0, 1, 0, 0, 1, 0 } },
    { 'Q', { 0, 0, 0, 0, 0, 0, 1, 1, 1 } },
    { 'R', { 1, 0, 0, 0, 0, 0, 1, 1, 0 } },
    { 'S', { 0, 0, 1, 0, 0, 0, 1, 1, 0 } },
    { 'T', { 0, 0, 0, 0, 1, 0, 1, 1, 0 } },
    { 'U', { 1, 1, 0, 0, 0, 0, 0, 0, 1 } },
    { 'V', { 0, 1, 1, 0, 0, 0, 0, 0, 1 } },
    { 'W', { 1, 1, 1, 0, 0, 0, 0, 0, 0 } },
    { 'X', { 0, 1, 0, 0, 1, 0, 0, 0, 1 } },
    { 'Y', { 1, 1, 0, 0, 1, 0, 0, 0, 0 } },
    { 'Z', { 0, 1, 1, 0, 1, 0, 0, 0, 0 } },

    { '-', { 0, 1, 0, 0, 0, 0, 1, 0, 1 } },
    { '.', { 1, 1, 0, 0, 0, 0, 1, 0, 0 } },
    { ' ', { 0, 1, 1, 0, 0, 0, 1, 0, 0 } },
    { '$', { 0, 1, 0, 1, 0, 1, 0, 0, 0 } },
    { '/', { 0, 1, 0, 1, 0, 0, 0, 1, 0 } },
    { '+', { 0, 1, 0, 0, 0, 1, 0, 1, 0 } },
    { '%', { 0, 0, 0, 1, 0, 1, 0, 1, 0 } },

    { '*', { 0, 1, 0, 0, 1, 0, 1, 0, 0 } }, // this is a special start/stop character

    { '\0', { 0, 0, 0, 0, 0, 0, 0, 0, 0 } } // null termininator of list
};

int codeIndex(QChar code)
{
    // we are a case insensitive search
    code = code.toUpper();
    for (int idx = 0; _3of9codes[idx].code != '\0'; idx++) {
        if (_3of9codes[idx].code == code.toAscii()) return idx;
    }
    return -1;  // couldn't find it
}


void render3of9(OROPage * page, const QRectF & r, const QString & _str, int align)
{
    QString str = _str;
    // lets determine some core attributes about this barcode
    qreal narrow_bar = 1; // a narrow bar is 1/100th inch wide
    qreal interchange_gap = narrow_bar; // the space between each 'set' of bars
    int bar_width_mult = 2; // the wide bar width multiple of the narrow bar

    // this is our mandatory minimum quiet zone
    qreal quiet_zone = narrow_bar * 10;
    if (quiet_zone < 0.1)
        quiet_zone = 0.1;

    // what kind of area do we have to work with
    qreal draw_width = r.width();
    qreal draw_height = r.height();

    // how long is the value we need to encode?
    int val_length = str.length();

    // L = (C + 2)(3N + 6)X + (C + 1)I
    // L length of barcode (excluding quite zone) in units same as X and I
    // C the number of characters in the value excluding the start/stop
    // N the bar width multiple for wide bars
    // X the width of a bar (pixels in our case)
    // I the interchange gap in the same units as X (value is same as X for our case)
    qreal L;

    qreal C = val_length;
    qreal N = bar_width_mult;
    qreal X = narrow_bar;
    qreal I = interchange_gap;

    L = ((C + 2.0) * (3.0 * N + 6.0) * X) + ((C + 1.0) * I);

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
        qreal nqz = (draw_width - L) / 2.0;
        if (nqz > quiet_zone)
            quiet_zone = nqz;
    } else if (align > 1)  // right
        quiet_zone = draw_width - (L + quiet_zone);
    //else if(align < 1) {} // left : do nothing

    qreal pos = r.left() + quiet_zone;
    qreal top = r.top();

    // ok we need to prepend and append the str with a *
    //str = QString().sprintf("*%s*",(const char*)str);
    str = '*' + str + '*';

    QPen pen(Qt::NoPen);
    QBrush brush(QColor("black"));
    for (int i = 0; i < str.length(); i++) {
        // loop through each char and render the barcode
        QChar c = str.at(i);
        int idx = codeIndex(c);
        kDebug() << idx;
        if (idx == -1) {
            qDebug("Encountered a non-compliant character while rendering a 3of9 barcode -- skipping");
            continue;
        }

        bool space = false;
        for (int b = 0; b < 9; b++, space = !space) {
            qreal w = (_3of9codes[idx].values[b] == 1 ? narrow_bar * bar_width_mult : narrow_bar);
            kDebug() << w << space;
            if (!space) {
                ORORect * rect = new ORORect();
                rect->setPen(pen);
                rect->setBrush(brush);
                rect->setRect(QRectF(pos, top, w, draw_height));
                page->addPrimitive(rect);
            }
            pos += w;
        }
        pos += interchange_gap;
    }
}
