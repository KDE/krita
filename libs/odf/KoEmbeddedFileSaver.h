/* This file is part of the KDE project
   Copyright (C) 2007 Thorsten Zachmann <zachmann@kde.org>
   Copyright (C) 2011 Inge Wallin <inge@lysator.liu.se>

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

#ifndef KOEMBEDDEDFILESAVER_H
#define KOEMBEDDEDFILESAVER_H

#include "KoOdfDocument.h"
#include "koodf_export.h"

class KoXmlWriter;

/**
 * This class is used to save embedded files in ODF documents.
 *
 * The difference between this and embedded objects / documents is
 * that embedded files are treated like blobs while embedded objects
 * have their own saving functions.  @see KoEmbeddedDocumentSaver.
 *
 */
class KOODF_EXPORT KoEmbeddedFileSaver
{
public:
    KoEmbeddedFileSaver();
    ~KoEmbeddedFileSaver();

    QString getFilename(const QString &prefix);

    /**FIXME
     * Adds the object specific attributes to the tag, but does NOT
     * write the content of the embedded document. Saving of the
     * embedded documents themselves is done in @ref save. This
     * function should be called from within KoOdfDocument::saveOdf.
     */
    void embedFile(KoXmlWriter &writer, const QString &path, const QByteArray &mimeType,
                   QByteArray contents);

    /**
     * Save all embedded files to the store.
     */
    bool saveEmbeddedFiles(KoOdfDocument::SavingContext &documentContext);

private:
    class Private;
    Private *d;
    Q_DISABLE_COPY(KoEmbeddedFileSaver)
};

#endif /* KOEMBEDDEDFILESAVER_H */
