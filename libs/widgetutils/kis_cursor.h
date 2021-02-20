/*
 *  kis_cursor.h - part of KImageShop
 *
 *  SPDX-FileCopyrightText: 1999 Matthias Elter <elter@kde.org>
 *  SPDX-FileCopyrightText: 2004 Adrian Page <adrian@pagenet.plus.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef __kis_cursor_h__
#define __kis_cursor_h__

#include <kritawidgetutils_export.h>

#include <QCursor>

class KRITAWIDGETUTILS_EXPORT KisCursor
{

public:

    KisCursor();

    // Predefined Qt cursors.
    static QCursor arrowCursor();         // standard arrow cursor
    static QCursor upArrowCursor();       // upwards arrow
    static QCursor crossCursor();         // crosshair
    static QCursor roundCursor();         // small open circle
    static QCursor pixelBlackCursor();    // black single pixel
    static QCursor pixelWhiteCursor();    // white single pixel
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

    static QCursor zoomSmoothCursor();
    static QCursor zoomDiscreteCursor();
    static QCursor rotateCanvasSmoothCursor();
    static QCursor rotateCanvasDiscreteCursor();
    static QCursor samplerImageForegroundCursor();
    static QCursor samplerImageBackgroundCursor();
    static QCursor samplerLayerForegroundCursor();
    static QCursor samplerLayerBackgroundCursor();
    static QCursor changeExposureCursor();
    static QCursor changeGammaCursor();
    static QCursor triangleLeftHandedCursor();
    static QCursor triangleRightHandedCursor();

    // Existing custom KimageShop cursors. Use the 'load' function for all new cursors.
    static QCursor moveCursor();          // move tool cursor
    static QCursor moveSelectionCursor(); // move selection action cursor
    static QCursor penCursor();           // pen tool cursor
    static QCursor brushCursor();         // brush tool cursor
    static QCursor airbrushCursor();      // airbrush tool cursor
    static QCursor eraserCursor();        // eraser tool cursor
    static QCursor fillerCursor();        // filler tool cursor
    static QCursor samplerCursor();        // color sampler cursor
    static QCursor samplerPlusCursor();    // color sampler cursor
    static QCursor samplerMinusCursor();   // color sampler cursor
    static QCursor pickLayerCursor();     // pick layer cursor
    static QCursor colorChangerCursor();  // color changer tool cursor
    static QCursor selectCursor();        // select cursor
    static QCursor handCursor();          // hand tool cursor
    static QCursor openHandCursor();      // Pan tool cursor
    static QCursor closedHandCursor();    // Pan tool cursor
    static QCursor rotateCursor();    // Transform tool cursor

    static QCursor meshCursorFree();    // Transform tool cursor
    static QCursor meshCursorLocked();    // Transform tool cursor


    // Load a cursor from an image file. The image should have an alpha channel
    // and will be converted to black and white on loading. Any format loadable
    // by QImage can be used. The file will be stored in the KisIconCache, so
    // each file will be loaded only once.
    static QCursor load(const QString & cursorName, int hotspotX = -1, int hotspotY = -1);

private:


};
#endif // __kis_cursor_h__
