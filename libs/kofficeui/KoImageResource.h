/*
 *  koImageResource.h - part of KOffice
 *
 *  Copyright (c) 2005 Thomas Zander <zander@kde.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation;version 2.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#ifndef __ko_imageresource__
#define __ko_imageresource__
#include <koffice_export.h>

class KOFFICEUI_EXPORT KoImageResource
{

public:

    KoImageResource();

    /// returns a 24 pixels xpm-format image of a chain.
    const char** chain();
    /// returns a 24 pixels xpm-format image of a broken chain.
    const char** chainBroken();
};
#endif // __ko_imageresource__

