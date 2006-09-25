/*
 *  kis_cursor.h - part of KImageShop
 *
 *  Copyright (c) 1999 Matthias Elter <elter@kde.org>
 *  Copyright (c) 2004 Adrian Page <adrian@pagenet.plus.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef __kis_cursor_h__
#define __kis_cursor_h__
#include <krita_export.h>
class QCursor;

class KRITAUI_EXPORT KisCursor
{

public:

    KisCursor();

    // Predefined Qt cursors.
    static QCursor arrowCursor();         // standard arrow cursor
    static QCursor upArrowCursor();       // upwards arrow
    static QCursor crossCursor();         // crosshair
    static QCursor waitCursor();          // hourglass/watch
    static QCursor ibeamCursor();         // ibeam/text entry
    static QCursor sizeVerCursor();       // vertical resize
    static QCursor sizeHorCursor();       // horizontal resize
    static QCursor sizeBDiagCursor();     // diagonal resize (/)
    static QCursor sizeFDiagCursor();     // diagonal resize (\)
    static QCursor sizeAllCursor();       // all directions resize
    static QCursor blankCursor();         // blank/invisible cursor
    static QCursor splitVCursor();        // vertical splitting
    static QCursor splitHCursor();        // horizontal splitting
    static QCursor pointingHandCursor();  // a pointing hand

    // Existing custom KimageShop cursors. Use the 'load' function for all new cursors.
    static QCursor moveCursor();          // move tool cursor
    static QCursor penCursor();           // pen tool cursor
    static QCursor brushCursor();         // brush tool cursor
    static QCursor airbrushCursor();      // airbrush tool cursor
    static QCursor eraserCursor();        // eraser tool cursor
    static QCursor fillerCursor();        // filler tool cursor
    static QCursor pickerCursor();        // color picker cursor
    static QCursor pickerPlusCursor();        // color picker cursor
    static QCursor pickerMinusCursor();        // color picker cursor
    static QCursor colorChangerCursor();  // color changer tool cursor
    static QCursor selectCursor();        // select cursor
    static QCursor zoomCursor();          // zoom tool cursor
    static QCursor handCursor();          // hand tool cursor
    static QCursor openHandCursor();      // Pan tool cursor
    static QCursor closedHandCursor();    // Pan tool cursor
    static QCursor rotateCursor();    // Transform tool cursor

    // Load a cursor from an image file. The image should have an alpha channel
    // and will be converted to black and white on loading. Any format loadable by
    // QImage can be used.
    static QCursor load(const QString & imageFilename, int hotspotX = -1, int hotspotY = -1);
};
#endif // __kis_cursor_h__
