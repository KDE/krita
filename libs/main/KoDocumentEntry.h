/* This file is part of the KDE project
   Copyright (C) 1998, 1999 Torben Weis <weis@kde.org>
   Copyright     2007       David Faure <faure@kde.org>

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

#ifndef __ko_document_entry_h__
#define __ko_document_entry_h__

#include <kservice.h>
#include <ksharedptr.h>
#include <QList>
#include "komain_export.h"

class QStringList;
class KoDocument;
class KoFilter;
class KoPart;

/**
 *  Represents an available Calligra component
 *  that supports the document interface.
 */
class KOMAIN_EXPORT KoDocumentEntry
{

public:
    /**
     * Represents an invalid entry (as returned by queryByMimeType for instance)
     */
    explicit KoDocumentEntry();
    /**
     * Represents a valid entry
     */
    explicit KoDocumentEntry(const KService::Ptr& service);
    ~KoDocumentEntry();

    KService::Ptr service() const;

    /**
     * @return TRUE if the service pointer is null
     */
    bool isEmpty() const;

    /**
     * @return name of the associated service
     */
    QString name() const;

    /**
     *  Mimetypes (and other service types) which this document can handle.
     */
    QStringList mimeTypes() const;

    /**
     *  @return TRUE if the document can handle the requested mimetype.
     */
    bool supportsMimeType(const QString & _mimetype) const;

    /**
     *  Uses the factory of the component to create
     *  a part. If that is not possible, 0 is returned.
     */
    KoPart *createKoPart(QString* errorMsg = 0) const;

    /**
     *  This function will query ksycoca to find all available components.
     *  The result will only contain parts, which are embeddable into a document
     *
     *  @param _constr is a constraint expression as used by KTrader.
     *                 You can use it to set additional restrictions on the available
     *                 components.
     */
    static QList<KoDocumentEntry> query(const QString &  _constr = QString());

    /**
     *  This is a convenience function.
     *
     *  @return a document entry for the Calligra component that supports
     *          the requested mimetype and fits the user best.
     */
    static KoDocumentEntry queryByMimeType(const QString & mimetype);

private:
    KService::Ptr m_service;
};

#endif
