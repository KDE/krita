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
#include <KoXmlWriter.h>
#include <KoXmlReader.h>

#include <KritaVersionWrapper.h>
#include <kis_group_layer.h>
#include <kis_image.h>
#include <kis_paint_layer.h>
#include <kis_png_converter.h>
#include <KisDocument.h>

static const char CURRENT_DTD_VERSION[] = "2.0";

KraConverter::KraConverter(KisDocument *doc)
    : m_doc(doc)
    , m_image(doc->savingImage())
{
}

KraConverter::~KraConverter()
{
    delete m_store;
    delete m_kraSaver;
    delete m_kraLoader;
}

KisImageBuilder_Result KraConverter::buildImage(QIODevice *io)
{
    m_store = KoStore::createStore(io, KoStore::Read, "", KoStore::Zip);

    if (m_store->bad()) {
        m_doc->setErrorMessage(i18n("Not a valid Krita file"));
        return KisImageBuilder_RESULT_FAILURE;
    }

    bool success;
    {
        if (m_store->hasFile("root") || m_store->hasFile("maindoc.xml")) {   // Fallback to "old" file format (maindoc.xml)
            KoXmlDocument doc;

            bool ok = oldLoadAndParse(m_store, "root", doc);
            if (ok)
                ok = loadXML(doc, m_store);
            if (!ok) {
                return KisImageBuilder_RESULT_FAILURE;
            }

        } else {
            errUI << "ERROR: No maindoc.xml" << endl;
            m_doc->setErrorMessage(i18n("Invalid document: no file 'maindoc.xml'."));
            return KisImageBuilder_RESULT_FAILURE;
        }

        if (m_store->hasFile("documentinfo.xml")) {
            KoXmlDocument doc;
            if (oldLoadAndParse(m_store, "documentinfo.xml", doc)) {
                m_doc->documentInfo()->load(doc);
            }
        }
        success = completeLoading(m_store);
    }

    return success ? KisImageBuilder_RESULT_OK : KisImageBuilder_RESULT_FAILURE;
}

KisImageSP KraConverter::image()
{
    return m_image;
}

vKisNodeSP KraConverter::activeNodes()
{
    return m_activeNodes;
}

QList<KisPaintingAssistantSP> KraConverter::assistants()
{
    return m_assistants;
}

KisImageBuilder_Result KraConverter::buildFile(QIODevice *io, const QString &filename)
{
    m_store = KoStore::createStore(io, KoStore::Write, m_doc->nativeFormatMimeType(), KoStore::Zip);

    if (m_store->bad()) {
        m_doc->setErrorMessage(i18n("Could not create the file for saving"));
        return KisImageBuilder_RESULT_FAILURE;
    }

    bool result = false;

    m_kraSaver = new KisKraSaver(m_doc, filename);

    result = saveRootDocuments(m_store);

    if (!result) {
        return KisImageBuilder_RESULT_FAILURE;
    }

    result = m_kraSaver->saveKeyframes(m_store, m_doc->url().toLocalFile(), true);
    if (!result) {
        qWarning() << "saving key frames failed";
    }
    result = m_kraSaver->saveBinaryData(m_store, m_image, m_doc->url().toLocalFile(), true, m_doc->isAutosaving());
    if (!result) {
        qWarning() << "saving binary data failed";
    }
    result = m_kraSaver->savePalettes(m_store, m_image, m_doc->url().toLocalFile());
    if (!result) {
        qWarning() << "saving palettes data failed";
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
    dbgFile << "Saving root";
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

    if (store->open("preview.png")) {
        // ### TODO: missing error checking (The partition could be full!)
        savePreview(store);
        (void)store->close();
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
    root.setAttribute("kritaVersion", KritaVersionWrapper::versionString(false));

    root.appendChild(m_kraSaver->saveXML(doc, m_image));

    if (!m_kraSaver->errorMessages().isEmpty()) {
        m_doc->setErrorMessage(m_kraSaver->errorMessages().join(".\n"));
    }
    return doc;
}

bool KraConverter::savePreview(KoStore *store)
{
    QPixmap pix = m_doc->generatePreview(QSize(256, 256));
    QImage preview(pix.toImage().convertToFormat(QImage::Format_ARGB32, Qt::ColorOnly));
    if (preview.size() == QSize(0,0)) {
        QSize newSize = m_doc->savingImage()->bounds().size();
        newSize.scale(QSize(256, 256), Qt::KeepAspectRatio);
        preview = QImage(newSize, QImage::Format_ARGB32);
        preview.fill(QColor(0, 0, 0, 0));
    }

    KoStoreDevice io(store);
    if (!io.open(QIODevice::WriteOnly)) {
        return false;
    }
    bool ret = preview.save(&io, "PNG");
    io.close();
    return ret;
}


bool KraConverter::oldLoadAndParse(KoStore *store, const QString &filename, KoXmlDocument &xmldoc)
{
    //dbgUI <<"Trying to open" << filename;

    if (!store->open(filename)) {
        warnUI << "Entry " << filename << " not found!";
        m_doc->setErrorMessage(i18n("Could not find %1", filename));
        return false;
    }
    // Error variables for QDomDocument::setContent
    QString errorMsg;
    int errorLine, errorColumn;
    bool ok = xmldoc.setContent(store->device(), &errorMsg, &errorLine, &errorColumn);
    store->close();
    if (!ok) {
        errUI << "Parsing error in " << filename << "! Aborting!" << endl
              << " In line: " << errorLine << ", column: " << errorColumn << endl
              << " Error message: " << errorMsg << endl;
        m_doc->setErrorMessage(i18n("Parsing error in %1 at line %2, column %3\nError message: %4"
                                   , filename  , errorLine, errorColumn ,
                                   QCoreApplication::translate("QXml", errorMsg.toUtf8(), 0)));
        return false;
    }
    dbgUI << "File" << filename << " loaded and parsed";
    return true;
}

bool KraConverter::loadXML(const KoXmlDocument &doc, KoStore *store)
{
    Q_UNUSED(store);

    KoXmlElement root;
    KoXmlNode node;

    if (doc.doctype().name() != "DOC") {
       m_doc->setErrorMessage(i18n("The format is not supported or the file is corrupted"));
        return false;
    }
    root = doc.documentElement();
    int syntaxVersion = root.attribute("syntaxVersion", "3").toInt();
    if (syntaxVersion > 2) {
       m_doc->setErrorMessage(i18n("The file is too new for this version of Krita (%1).", syntaxVersion));
        return false;
    }

    if (!root.hasChildNodes()) {
       m_doc->setErrorMessage(i18n("The file has no layers."));
        return false;
    }

    m_kraLoader = new KisKraLoader(m_doc, syntaxVersion);

    // reset the old image before loading the next one
    m_doc->setCurrentImage(0, false);

    for (node = root.firstChild(); !node.isNull(); node = node.nextSibling()) {
        if (node.isElement()) {
            if (node.nodeName() == "IMAGE") {
                KoXmlElement elem = node.toElement();
                if (!(m_image = m_kraLoader->loadXML(elem))) {
                    if (m_kraLoader->errorMessages().isEmpty()) {
                        m_doc->setErrorMessage(i18n("Unknown error."));
                    }
                    else {
                        m_doc->setErrorMessage(m_kraLoader->errorMessages().join("\n"));
                    }
                    return false;
                }

                // HACK ALERT!
                m_doc->hackPreliminarySetImage(m_image);

                return true;
            }
            else {
                if (m_kraLoader->errorMessages().isEmpty()) {
                    m_doc->setErrorMessage(i18n("The file does not contain an image."));
                }
                return false;
            }
        }
    }
    return false;
}

bool KraConverter::completeLoading(KoStore* store)
{
    if (!m_image) {
        if (m_kraLoader->errorMessages().isEmpty()) {
           m_doc->setErrorMessage(i18n("Unknown error."));
        }
        else {
           m_doc->setErrorMessage(m_kraLoader->errorMessages().join("\n"));
        }
        return false;
    }

    m_image->blockUpdates();

    QString layerPathName = m_kraLoader->imageName();
    if (!m_store->hasFile(layerPathName)) {
        // We might be hitting an encoding problem. Get the only folder in the toplevel
        Q_FOREACH (const QString &entry, m_store->directoryList()) {
            if (entry.contains("layer")) {
                layerPathName = entry.split('/').first();
                m_store->setSubstitution(m_kraLoader->imageName(), layerPathName);
                break;
            }
        }
    }

    m_kraLoader->loadBinaryData(store, m_image, m_doc->localFilePath(), true);
    m_kraLoader->loadPalettes(store, m_doc);

    m_image->unblockUpdates();

    if (!m_kraLoader->warningMessages().isEmpty()) {
        // warnings do not interrupt loading process, so we do not return here
        m_doc->setWarningMessage(m_kraLoader->warningMessages().join("\n"));
    }

    m_activeNodes = m_kraLoader->selectedNodes();
    m_assistants = m_kraLoader->assistants();

    return true;
}

void KraConverter::cancel()
{
    m_stop = true;
}


