/* This file is part of the KDE project
   Copyright (C) 1998, 1999 Reginald Stadlbauer <reggie@kde.org>

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

#ifndef KOTABULATOR_H
#define KOTABULATOR_H

#include <QChar>
#include <QList>

enum KoTabulators { T_LEFT = 0, T_CENTER = 1, T_RIGHT = 2, T_DEC_PNT = 3, T_INVALID = -1 };
enum KoTabulatorFilling { TF_BLANK = 0, TF_DOTS = 1, TF_LINE = 2, TF_DASH = 3, TF_DASH_DOT = 4, TF_DASH_DOT_DOT = 5};

/**
 * Struct: KoTabulator
 * Defines the position of a tabulation (in pt), and its type
 */
struct KoTabulator {
    /**
     * Position of the tab in pt
    */
    double ptPos;
    /**
     * Type of tab (left/center/right/decimalpoint)
    */
    KoTabulators type;
    /**
     * Type of tab filling.
    */
    KoTabulatorFilling filling;
    /**
     * Width of the tab filling line.
    */
    double ptWidth;
    /**
     * Alignment character.
    */
    QChar alignChar;

    bool operator==( const KoTabulator & t ) const {
        return QABS( ptPos - t.ptPos ) < 1E-4 && type == t.type &&
               filling == t.filling && QABS( ptWidth - t.ptWidth ) < 1E-4;
}
    bool operator!=( const KoTabulator & t ) const {
        return !operator==(t);
}
    // Operators used for sorting
    bool operator < ( const KoTabulator & t ) const {
        return ptPos < t.ptPos;
}
    bool operator <= ( const KoTabulator & t ) const {
        return ptPos <= t.ptPos;
}
    bool operator > ( const KoTabulator & t ) const {
        return ptPos > t.ptPos;
}
};

typedef QList<KoTabulator> KoTabulatorList;

#endif
