/* This file is part of the KDE project
   SPDX-FileCopyrightText: 1998, 1999 Torben Weis <weis@kde.org>
   SPDX-FileCopyrightText: 2002, 2003 David Faure <faure@kde.org>
   SPDX-FileCopyrightText: 2003 Nicolas GOUTTE <goutte@kde.org>

   SPDX-License-Identifier: LGPL-2.0-or-later
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
