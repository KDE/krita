/* This file is part of the KDE project
   SPDX-FileCopyrightText: 2006, 2008 Jan Hambrecht <jaham@gmx.net>
   SPDX-FileCopyrightText: 2007 David Faure <faure@kde.org>
   SPDX-FileCopyrightText: 2007 Dirk Mueller <mueller@kde.org>
   SPDX-FileCopyrightText: 2008 Patrick Spendrin <ps_ml@gmx.de>

   SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KARBONCURSOR_H
#define KARBONCURSOR_H

#include <QCursor>

/**
* A helper class for easily creating cursors from XPMs.
*
* One can create a predefined unthemed cursor or create a cursor from two given XMPs,
* the cursor bitmap and the cursor mask.
*/
class KarbonCursor
{
public:
    /** Predefined cursor types */
    enum CursorType {
        CrossHair = 0, /**< unthemed crosshair cursor */
        ZoomPlus  = 1, /**< zoom in cursor */
        ZoomMinus = 2, /**< zoom out cursor */
        NeedleArrow = 3  /**< needle arrow */
    };

    /**
    * Creates a predefined cursor of the specified type.
    *
    * @param type the requested cursor id
    * @return the predefined cursor
    */
    static QCursor createCursor(CursorType type);

    /**
    * Creates a cursor from two specified XPM images.
    * This is only a wrapper function for a QCursor ctor.
    */
    static QCursor createCursor(const char *bitmap[], const char *mask[], int hotX = -1, int hotY = -1);

    /** crosshair cursor */
    static QCursor crossHair();

    /** needle arraow cursor */
    static QCursor needleArrow();

    /** needle arrow with four way arrow */
    static QCursor needleMoveArrow();

    static QCursor horzMove();

private:
    // prevent instantiation
    KarbonCursor() {}
};

#endif // KARBONCURSOR_H
