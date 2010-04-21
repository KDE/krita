/* This file is part of the KOffice project
 * Copyright (C) 2008 Sebastian Sauer <mail@dipe.org>
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

#ifndef KOTEXTPAGE_H
#define KOTEXTPAGE_H

#include "kotext_export.h"

/**
 * Interface for a single OpenDocumentText page.
 *
 * The KWord KWPageTextInfo class does implement this interface to provide
 * application specific functionality for single pages.
 * @see KoTextShapeData::setPage();
 */
class KOTEXT_EXPORT KoTextPage
{
public:

    /// Destructor.
    virtual ~KoTextPage();

    enum PageSelection {
        PreviousPage = -1,
        CurrentPage = 0,
        NextPage = 1
    };

    /**
     * Returns the number of this page for display purposes.
     *
     * Example how the parameters are used within ODF to display the
     * current page number on all pages except the first page;
     * \code
     * <text:page-number text:select-page="previous" text:page-adjust="1" />
     * \endcode
     *
     * \param select Defines the offset of the page to select for the
     * resulting page number.  If such a page does not exist, then -1 will be
     * returned before the adjustment will be taken into account. This
     * implements the ODF text:select-page attribute.
     * \param adjustment The value of the page number will be adjusted by this
     * specified number and if there exist a page with the resulting value it's
     * page number gets returned, otherwise -1 will be returned. This implements the
     * ODF text:page-adjust attribute.
     * \return the user visible page number, or -1 if the page referenced does not
     * exist.
     */
    virtual int pageNumber(PageSelection select = CurrentPage, int adjustment = 0) const = 0;
};

#endif
