/* This file is part of the KDE project
   Copyright (C) 2010 KO GmbH <ben.martin@kogmbh.com>

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

#ifndef __rdf_KoRdfSemanticItem_p_h__
#define __rdf_KoRdfSemanticItem_p_h__

#include <komain_export.h>

class KOMAIN_EXPORT KoRdfSemanticItemPrivate
{
public:
    const KoDocumentRdf *m_rdf;    //< For access to the Rdf model during CRUD operations
    Soprano::Node m_context; //< This determines the Rdf/XML file the Rdf is stored in (see context())

    KoRdfSemanticItemPrivate (const KoDocumentRdf *rdf);
    KoRdfSemanticItemPrivate (const KoDocumentRdf *rdf,Soprano::QueryResultIterator &it);
};

#endif
