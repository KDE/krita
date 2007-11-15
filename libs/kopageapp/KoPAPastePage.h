/* This file is part of the KDE project
   Copyright (C) 2007 Thorsten Zachmann <zachmann@kde.org>

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

#ifndef KOPAPASTEPAGE_H
#define KOPAPASTEPAGE_H

#include "KoOdfPaste.h"

#include "kopageapp_export.h"

class KoPADocument;
class KoPAPageBase;

class KOPAGEAPP_TEST_EXPORT KoPAPastePage : public KoOdfPaste
{
public:
    /**
     * Paste pages
     *
     * This uses intelligent paste of pages. 
     * o When copying a page and the master page of that page already exists the 
     *   master page is not created, instead the existing master page is used. 
     * o When copying a page and the master page of that page does not yet exists
     *   the master page of that page is also created.
     * o When copying a master page a copy of the page is always created.
     *
     * @param doc The document in which the pages are pasted.
     * @param activePage The page after which the pages are pasted. If 0 at the 
     *        pages are inserted at the beginning.
     */
    KoPAPastePage( KoPADocument * doc, KoPAPageBase * activePage );

protected:
    bool process( const KoXmlElement & body, KoOdfReadStore & odfStore );

    KoPADocument * m_doc;
    KoPAPageBase * m_activePage;
};

#endif /* KOPAPASTEPAGE_H */
