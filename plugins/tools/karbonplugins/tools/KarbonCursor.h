/* This file is part of the KDE project
   Copyright (C) 2006,2008 Jan Hambrecht <jaham@gmx.net>
   Copyright (C) 2007 David Faure <faure@kde.org>
   Copyright (C) 2007 Dirk Mueller <mueller@kde.org>
   Copyright (C) 2008 Patrick Spendrin <ps_ml@gmx.de>

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
 * Boston, MA 02110-1301, USA.
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
