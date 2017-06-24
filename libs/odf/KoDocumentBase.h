/* This file is part of the KDE project

   Copyright (C) 1998, 1999 Torben Weis <weis@kde.org>
   Copyright (C) 2000-2005 David Faure <faure@kde.org>
   Copyright (C) 2007 Thorsten Zachmann <zachmann@kde.org>
   Copyright (C) 2009 Boudewijn Rempt <boud@valdyas.org>

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
  Boston, MA 02110-1301, USA.
*/
#ifndef KODOCUMENTBASE_H
#define KODOCUMENTBASE_H

class KoStore;
class KoOdfReadStore;
class KoOdfWriteStore;
class KoEmbeddedDocumentSaver;

class QUrl;
class QByteArray;
class QString;

#include "kritaodf_export.h"

/**
 * Base class for documents that can load and save ODF. Most of the
 * implementation is still in KoDocument, though that should probably
 * change.
 */
class KRITAODF_EXPORT KoDocumentBase
{
public:

    // context passed on saving to saveOdf
    struct SavingContext {
        SavingContext(KoOdfWriteStore &odfStore, KoEmbeddedDocumentSaver &embeddedSaver)
                : odfStore(odfStore)
                , embeddedSaver(embeddedSaver) {}

        KoOdfWriteStore &odfStore;
        KoEmbeddedDocumentSaver &embeddedSaver;
    };

    /**
     * create a new KoDocumentBase
     */
    KoDocumentBase();

    /**
     * delete this document
     */
    virtual ~KoDocumentBase();

    /**
     * @return the current URL
     */
    virtual QUrl url() const = 0;

    virtual void setUrl(const QUrl &url) = 0;

    /**
     * Checks whether the document is currently in the process of autosaving
     */
    virtual bool isAutosaving() const = 0;

    /**
     * Returns true if this document or any of its internal child documents are modified.
     */
    virtual bool isModified() const = 0;

    /**
     * Returns the actual mimetype of the document
     */
    virtual QByteArray mimeType() const = 0;

    /**
     * @brief Sets the mime type for the document.
     *
     * When choosing "save as" this is also the mime type
     * selected by default.
     */
    virtual void setMimeType(const QByteArray & mimeType) = 0;

    virtual QString localFilePath() const = 0;

private:
    class Private;
    Private *const d;
};


#endif
