/* This file is part of the KDE project
   Copyright (C) 2001 David Faure <faure@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
*/
#ifndef __koDocument_p_h__
#define __koDocument_p_h__

#include <kparts/browserextension.h>

class KoDocument;

/**
 * Used in singleViewMode, when embedded into a browser
 * @internal
 */
class KoBrowserExtension : public KParts::BrowserExtension
{
    Q_OBJECT
public:
    KoBrowserExtension( KoDocument * doc, const char * name = 0 );

public slots:
    /// Automatically detected by konqueror
    void print();
};

#endif
