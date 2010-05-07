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
#ifndef KOODFDOCUMENT_H
#define KOODFDOCUMENT_H

#include <kurl.h>

class KoStore;
class KoOdfReadStore;
class KoOdfWriteStore;
class KoEmbeddedDocumentSaver;

#include "koodf_export.h"

/**
 * Base class for documents that can load and save ODF. Most of the
 * implementation is still in KoDocument, though that should probably
 * change.
 */
class KOODF_EXPORT KoOdfDocument
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
     * create a new KoOdfDocument
     */
    KoOdfDocument();

    /**
     * delete this document
     */
    virtual ~KoOdfDocument();

    /**
     *  Saves all internal children (only!), to the store, using the OASIS format.
     *  This is called automatically during saveNativeFormat.
     *  @see saveExternalChildren if you have external children.
     *  Returns true on success.
     */
    bool saveChildrenOdf(SavingContext &documentContext);

    /**
     * Return true if url() is a real filename, false if url() is
     * an internal url in the store, like "tar:/..."
     */
    virtual bool isStoredExtern() const = 0;

    /**
     * @return the current URL
     */
    virtual KUrl odfUrl() const = 0;

    virtual void setOdfUrl(const KUrl &url) = 0;

    /**
     * Returns the OASIS OpenDocument mimetype of the document, if supported
     * This comes from the X-KDE-NativeOasisMimeType key in the .desktop file
     */
    virtual QByteArray nativeOasisMimeType() const = 0;

    /**
     *  @brief Saves a document to a store.
     */
    virtual bool saveToStore(KoStore *store, const QString &path) = 0;

    /**
     *  Reimplement this method to load the odf document. Take care to
     *  make sure styles are loaded before body text is loaded by the
     *  text shape.
     */
    virtual bool loadOdf(KoOdfReadStore &odfStore) = 0;

    /**
     *  Reimplement this method to save the contents of your %KOffice document,
     *  using the ODF format.
     */
    virtual bool saveOdf(SavingContext &documentContext) = 0;

private:
    class Private;
    Private *const d;
};


#endif
