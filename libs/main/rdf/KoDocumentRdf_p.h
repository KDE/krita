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

#ifndef KO_DOCUMENT_Rdf_PRIVATE_H
#define KO_DOCUMENT_Rdf_PRIVATE_H

#include "KoDocumentRdf.h"

class KoDocumentRdfPrivate
{
public:
    Soprano::Model *m_model; ///< Main Model containing all Rdf for doc
    typedef QList<KoTextInlineRdf*> m_inlineRdfObjects_t;
    m_inlineRdfObjects_t m_inlineRdfObjects;  ///< Cache of weak pointers to inline Rdf
    KoRdfPrefixMapping *m_prefixMapping;     ///< prefix -> URI mapping

    QList<KoRdfFoaF*> m_foafObjects;
    QList<KoRdfCalendarEvent*> m_calObjects;
    QList<KoRdfLocation*> m_locObjects;

    QMap<QString,QList<KoSemanticStylesheet*> > m_userStylesheets;

    KoDocumentRdfPrivate();
    ~KoDocumentRdfPrivate();
};

#endif

