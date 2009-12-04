/* This file is part of the KDE project
 * Copyright (C) 2007-2008 Thorsten Zachmann <zachmann@kde.org>
 * Copyright (C) 2009 Thomas Zander <zander@kde.org>
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

#include "KoDrag.h"
#include "KoDragOdfSaveHelper.h"

#include <QApplication>
#include <QBuffer>
#include <QByteArray>
#include <QClipboard>
#include <QMimeData>
#include <QString>

#include <kdebug.h>

#include <KoStore.h>
#include <KoGenStyles.h>
#include <KoOdfWriteStore.h>
#include <KoXmlWriter.h>
#include <KoOdfDocument.h>
#include <KoEmbeddedDocumentSaver.h>
#include "KoShapeSavingContext.h"

class KoDragPrivate {
public:
    KoDragPrivate() : mimeData(0) { }
    ~KoDragPrivate() { delete mimeData; }
    QMimeData *mimeData;
};

KoDrag::KoDrag()
    : d(new KoDragPrivate())
{
}

KoDrag::~KoDrag()
{
    delete d;
}

bool KoDrag::setOdf(const char *mimeType, KoDragOdfSaveHelper &helper)
{
    struct Finally {
        Finally(KoStore *s) : store(s) { }
        ~Finally() {
            delete store;
        }
        KoStore *store;
    };

    QBuffer buffer;
    KoStore *store = KoStore::createStore(&buffer, KoStore::Write, mimeType);
    Finally finally(store); // delete store when we exit this scope
    Q_ASSERT(store);
    Q_ASSERT(!store->bad());

    KoOdfWriteStore odfStore(store);
    KoEmbeddedDocumentSaver embeddedSaver;

    KoXmlWriter *manifestWriter = odfStore.manifestWriter(mimeType);
    KoXmlWriter *contentWriter = odfStore.contentWriter();

    if (!contentWriter) {
        return false;
    }

    KoGenStyles mainStyles;
    KoXmlWriter *bodyWriter = odfStore.bodyWriter();
    KoShapeSavingContext *context = helper.context(bodyWriter, mainStyles, embeddedSaver);

    if (!helper.writeBody()) {
        return false;
    }

    mainStyles.saveOdfAutomaticStyles(contentWriter, false);

    odfStore.closeContentWriter();

    //add manifest line for content.xml
    manifestWriter->addManifestEntry("content.xml", "text/xml");


    if (!mainStyles.saveOdfStylesDotXml(store, manifestWriter)) {
        return false;
    }

    if (!context->saveDataCenter(store, manifestWriter)) {
        kDebug(30006) << "save data centers failed";
        return false;
    }

    // Save embedded objects
    KoOdfDocument::SavingContext documentContext(odfStore, embeddedSaver);
    if (!embeddedSaver.saveEmbeddedDocuments(documentContext)) {
        kDebug(30006) << "save embedded documents failed";
        return false;
    }

    // Write out manifest file
    if (!odfStore.closeManifestWriter()) {
        return false;
    }

    delete store; // make sure the buffer if fully flushed.
    finally.store = 0;
    setData(mimeType, buffer.buffer());

    return true;
}

void KoDrag::setData(const QString &mimeType, const QByteArray &data)
{
    if (d->mimeData == 0) {
        d->mimeData = new QMimeData();
    }
    d->mimeData->setData(mimeType, data);
}

void KoDrag::addToClipboard()
{
    if (d->mimeData) {
        QApplication::clipboard()->setMimeData(d->mimeData);
        d->mimeData = 0;
    }
}

QMimeData * KoDrag::mimeData()
{
    QMimeData *mimeData = d->mimeData;
    d->mimeData = 0;
    return mimeData;
}
