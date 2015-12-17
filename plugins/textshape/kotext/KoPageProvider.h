/* This file is part of the KDE project

   Copyright (C) 2009 Thorsten Zachmann <zachmann@kde.org>

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

#ifndef KOPAGEPROVIDER_H
#define KOPAGEPROVIDER_H

#include "kritatext_export.h"

class KoShape;
class KoTextPage;

/// \internal  this is a hack for kpresenter
class KRITATEXT_EXPORT KoPageProvider
{
public:
    KoPageProvider();
    virtual ~KoPageProvider();

    /**
     * Get the page number for the given shape
     */
    virtual KoTextPage *page(KoShape *shape) = 0;
};
#endif // KOPAGEPROVIDER_H
