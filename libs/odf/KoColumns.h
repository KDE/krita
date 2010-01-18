/* This file is part of the KDE project
   Copyright (C) 1998, 1999 Torben Weis <weis@kde.org>
   Copyright 2002, 2003 David Faure <faure@kde.org>
   Copyright 2003 Nicolas GOUTTE <goutte@kde.org>
   Copyright 2007 Thomas Zander <zander@kde.org>

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

#ifndef KOCOLUMNS_H
#define KOCOLUMNS_H


#include <QtGlobal>

/** structure for columns */
struct KoColumns {
    int columns;
    qreal columnSpacing;
    bool operator==(const KoColumns& rhs) const {
        return columns == rhs.columns &&
               qAbs(columnSpacing - rhs.columnSpacing) <= 1E-10;
    }
    bool operator!=(const KoColumns& rhs) const {
        return columns != rhs.columns ||
               qAbs(columnSpacing - rhs.columnSpacing) > 1E-10;
    }
};

#endif /* KOCOLUMNS_H */

