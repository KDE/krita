/* This file is part of the KDE project
   Copyright (C) 2008 Pierre Stirnweiss \pierre.stirnweiss_koffice@gadz.org>

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

#ifndef KOTEXTSHAPESAVINGCONTEXT_H
#define KOTEXTSHAPESAVINGCONTEXT_H

#include "kotext_export.h"

#include <KoShapeSavingContext.h>

class KoGenChanges;

/**
 * The set of data for the ODF file format used during saving of a shape.
 */
class KOTEXT_EXPORT KoTextShapeSavingContext : public KoShapeSavingContext
{
public:

    /**
     * @brief Constructor
     * @param xmlWriter used for writing the xml
     * @param mainStyles for saving the styles
     * @param embeddedSaver for saving embedded documents
     * @param changes for saving the tracked changes
     */
    KoTextShapeSavingContext(KoXmlWriter &xmlWriter, KoGenStyles& mainStyles,
                         KoEmbeddedDocumentSaver& embeddedSaver, KoGenChanges& changes);
    virtual ~KoTextShapeSavingContext();

    /**
     * @brief Get the changes (tracked)
     *
     * @return changes (tracked)
     */
    KoGenChanges & changes();


private:
    KoGenChanges& m_changes;
};

#endif // KOTEXTSHAPESAVINGCONTEXT_H
