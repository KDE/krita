/*
 * Copyright (C) 2016 Boudewijn Rempt <boud@valdyas.org>
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

#include "kra_converter.h"

#include <QApplication>
#include <QFileInfo>
#include <QScopedPointer>
#include <QUrl>

#include <KoStore.h>
#include <KoStoreDevice.h>
#include <KoColorSpaceRegistry.h>
#include <KoDocumentInfo.h>

#include <kis_group_layer.h>
#include <kis_image.h>
#include <kis_paint_layer.h>
#include <kis_png_converter.h>
#include <KisDocument.h>

static const char CURRENT_DTD_VERSION[] = "2.0";

KraConverter::KraConverter(KisDocument *doc)
    : m_doc(doc)
    , m_image(doc->image())
{
}

KraConverter::~KraConverter()
{
    delete m_store;
    delete m_kraSaver;
}

KisImageBuilder_Result KraConverter::buildImage(QIODevice *io)
{
    return KisImageBuilder_RESULT_OK;
}

KisImageWSP KraConverter::image()
{
    return m_image;
}

vKisNodeSP KraConverter::activeNodes()
{
    return m_activeNodes;
}

KisImageBuilder_Result KraConverter::buildFile(QIODevice *io, KisImageWSP /*image*/, vKisNodeSP /*activeNodes*/)
{
    m_store = KoStore::createStore(io, KoStore::Write, m_doc->nativeFormatMimeType(), KoStore::Zip);

    if (m_store->bad()) {
        m_doc->setErrorMessage(i18n("Could not create the file for saving"));
        return KisImageBuilder_RESULT_FAILURE;
    }

    bool result = false;

    m_kraSaver = new KisKraSaver(m_doc);

    result = saveRootDocuments(m_store);

    if (!result) {
        return KisImageBuilder_RESULT_FAILURE;
    }

    result = m_kraSaver->saveKeyframes(m_store, m_doc->url().toLocalFile(), m_doc->isStoredExtern());
    if (!result) {
        qWarning() << "saving key frames failed";
    }
    result = m_kraSaver->saveBinaryData(m_store, m_image, m_doc->url().toLocalFile(), m_doc->isStoredExtern(), m_doc->isAutosaving());
    if (!result) {
        qWarning() << "saving binary data failed";
    }

    if (!m_store->finalize()) {
        return KisImageBuilder_RESULT_FAILURE;
    }

    if (!m_kraSaver->errorMessages().isEmpty()) {
        m_doc->setErrorMessage(m_kraSaver->errorMessages().join(".\n"));
        return KisImageBuilder_RESULT_FAILURE;
    }
    return KisImageBuilder_RESULT_OK;
}

bool KraConverter::saveRootDocuments(KoStore *store)
{
    dbgUI << "Saving root";
    if (store->open("root")) {
        KoStoreDevice dev(store);
        if (!saveToStream(&dev) || !store->close()) {
            dbgUI << "saveToStream failed";
            return false;
        }
    } else {
        m_doc->setErrorMessage(i18n("Not able to write '%1'. Partition full?", QString("maindoc.xml")));
        return false;
    }
    bool success = false;
    if (store->open("documentinfo.xml")) {
        QDomDocument doc = KisDocument::createDomDocument("document-info"
                                                          /*DTD name*/, "document-info" /*tag name*/, "1.1");
        doc = m_doc->documentInfo()->save(doc);
        KoStoreDevice dev(store);
        QByteArray s = doc.toByteArray(); // this is already Utf8!
        success = dev.write(s.data(), s.size());
        store->close();
    }

    if (!m_doc->isAutosaving()) {
        if (store->open("preview.png")) {
            // ### TODO: missing error checking (The partition could be full!)
            savePreview(store);
            (void)store->close();
        }
    }

    dbgUI << "Saving done of url:" << m_doc->url().toLocalFile();
    // Success
    return success;
}

bool KraConverter::saveToStream(QIODevice *dev)
{
    QDomDocument doc = createDomDocument();
    // Save to buffer
    QByteArray s = doc.toByteArray(); // utf8 already
    dev->open(QIODevice::WriteOnly);
    int nwritten = dev->write(s.data(), s.size());
    if (nwritten != (int)s.size()) {
        warnUI << "wrote " << nwritten << "- expected" <<  s.size();
    }
    return nwritten == (int)s.size();
}

QDomDocument KraConverter::createDomDocument()
{
    QDomDocument doc = m_doc->createDomDocument("DOC", CURRENT_DTD_VERSION);
    QDomElement root = doc.documentElement();

    root.setAttribute("editor", "Krita");
    root.setAttribute("syntaxVersion", "2");

    root.appendChild(m_kraSaver->saveXML(doc, m_image));

    if (!m_kraSaver->errorMessages().isEmpty()) {
        m_doc->setErrorMessage(m_kraSaver->errorMessages().join(".\n"));
    }
    return doc;
}

bool KraConverter::savePreview(KoStore *store)
{
    QPixmap pix = m_doc->generatePreview(QSize(256, 256));
    const QImage preview(pix.toImage().convertToFormat(QImage::Format_ARGB32, Qt::ColorOnly));
    KoStoreDevice io(store);
    if (!io.open(QIODevice::WriteOnly)) {
        return false;
    }
    bool ret = preview.save(&io, "PNG");
    io.close();
    return ret;
}


void KraConverter::cancel()
{
    m_stop = true;
}


