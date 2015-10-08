/* This file is part of the KDE project
   Copyright (C) 1998, 1999 Torben Weis <weis@kde.org>
   Copyright 2002, 2003 David Faure <faure@kde.org>
   Copyright 2003 Nicolas GOUTTE <goutte@kde.org>

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

#ifndef KO_DPI_h
#define KO_DPI_h

#include <QStringList>
#include <QFont>
#include <QMap>

#include "kritawidgets_export.h"

/**
 * Singleton to store user-overwritten DPI information.
 */
class KRITAWIDGETS_EXPORT KoDpi
{
public:
    KoDpi();
    /// For KoApplication
    static void initialize()  {
        (void)self(); // I don't want to make KoDpi instances public, so self() is private
    }

    static int dpiX() {
        return self()->m_dpiX;
    }

    static int dpiY() {
        return self()->m_dpiY;
    }

    /// @internal, for KoApplication
    static void setDPI(int x, int y);

    ~KoDpi();

private:
    static KoDpi* self();


    int m_dpiX;
    int m_dpiY;
};

#endif // KO_DPI
