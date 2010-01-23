/* This file is part of the KDE project
 * Copyright (C) 2007 Thorsten Zachmann <zachmann@kde.org>
 * Copyright (C) 2008 Thomas Zander <zander@kde.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
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
#ifndef KOPAGEAPP_H
#define KOPAGEAPP_H

#include <KoResourceManager.h>

/// add docs please
namespace KoPageApp
{
    /// add docs please
    enum PageNavigation
    {
        PageFirst,
        PagePrevious,
        PageNext,
        PageLast
    };

    /**
     * This enum holds identifiers to the resources that can be stored in the KoResourceManager.
     */
    enum CanvasResource {
        CurrentPage = KoCanvasResource::KoPageAppStart+1 ///< The current page as a KoShape
    };

    /**
     * This enum defines if we should talk about pages or slides in the document
     */
    enum PageType {
        Page,
        Slide
    };
}

#endif // KOPAGEAPP_H
