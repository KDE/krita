/* This file is part of the KDE project
   Copyright (C) 2004-2006 David Faure <faure@kde.org>
   Copyright (C) 2007 Thorsten Zachmann <zachmann@kde.org>
   Copyright (C) 2010 Thomas Zander <zander@kde.org>

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

#include "KoEmbeddedDocumentSaver.h"

#include <kdebug.h>

#include <KoStore.h>
#include <KoXmlWriter.h>
#include <KoOdfWriteStore.h>

#include "KoOdfDocument.h"

#include <QList>

#define INTERNAL_PROTOCOL "intern"

class KoEmbeddedDocumentSaver::Private
{
public:
    Private()
        : objectId(0)
    {
    }

    QList<KoOdfDocument*> documents;
    int objectId;
};

KoEmbeddedDocumentSaver::KoEmbeddedDocumentSaver()
        : d(new Private())
{
}

KoEmbeddedDocumentSaver::~KoEmbeddedDocumentSaver()
{
    delete d;
}

void KoEmbeddedDocumentSaver::embedDocument(KoXmlWriter &writer, KoOdfDocument * doc)
{
    Q_ASSERT(doc);
    d->documents.append(doc);

    QString ref;
    if (!doc->isStoredExtern()) {
        const QString name = QString("Object_%1").arg(++d->objectId);
        // set URL in document so that saveEmbeddedDocuments will save
        // the actual embedded object with the right name in the store.
        KUrl u;
        u.setProtocol(INTERNAL_PROTOCOL);
        u.setPath(name);
        kDebug(30003) << u;
        doc->setOdfUrl(u);
        ref = "./" + name;
    } else {
        ref = doc->getOdfUrl().url();
    }

    //<draw:object draw:style-name="standard" draw:id="1" draw:layer="layout" svg:width="14.973cm" svg:height="4.478cm" svg:x="11.641cm" svg:y="14.613cm" xlink:href="#./Object 1" xlink:type="simple" xlink:show="embed" xlink:actuate="onLoad"/>
    writer.addAttribute("xlink:type", "simple");
    writer.addAttribute("xlink:show", "embed");
    writer.addAttribute("xlink:actuate", "onLoad");

    kDebug(30003) << "KoEmbeddedDocumentSaver::addEmbeddedDocument saving reference to embedded document as" << ref;
    writer.addAttribute("xlink:href", /*"#" + */ref);
}

bool KoEmbeddedDocumentSaver::saveEmbeddedDocuments(KoOdfDocument::SavingContext & documentContext)
{
    KoStore * store = documentContext.odfStore.store();
    foreach(KoOdfDocument * doc, d->documents) {
        QString path;
        if (doc->isStoredExtern()) {
            kDebug(30003) << " external (don't save) url:" << doc->getOdfUrl().url();
            path = doc->getOdfUrl().url();
        } else {
            // The name comes from addEmbeddedDocument (which was set while saving the document).
            Q_ASSERT(doc->getOdfUrl().protocol() == INTERNAL_PROTOCOL);
            const QString name = doc->getOdfUrl().path();
            kDebug(30003) << "saving" << name;

            if (doc->nativeOasisMimeType().isEmpty()) {
                // Embedded object doesn't support OpenDocument, save in the old format.
                kDebug(30003) << "Embedded object doesn't support OpenDocument, save in the old format.";

                if (!doc->saveToStore(store, name)) {
                    return false;
                }
            } else {
                // To make the children happy cd to the correct directory
                store->pushDirectory();
                store->enterDirectory(name);

                if (!doc->saveOdf(documentContext)) {
                    kWarning(30003) << "KoEmbeddedDocumentSaver::saveEmbeddedDocuments failed";
                    return false;
                }
                // Now that we're done leave the directory again
                store->popDirectory();
            }

            Q_ASSERT(doc->getOdfUrl().protocol() == INTERNAL_PROTOCOL);
            path = store->currentDirectory();
            if (!path.isEmpty()) {
                path += '/';
            }
            path += doc->getOdfUrl().path();
            if (path.startsWith('/')) {
                path = path.mid(1);   // remove leading '/', no wanted in manifest
            }
        }

        // OOo uses a trailing slash for the path to embedded objects (== directories)
        if (!path.endsWith('/')) {
            path += '/';
        }
        QByteArray mimetype = doc->nativeOasisMimeType();
        documentContext.odfStore.manifestWriter()->addManifestEntry(path, mimetype);
    }

    return true;
}
