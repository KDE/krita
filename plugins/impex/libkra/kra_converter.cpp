/*
 * SPDX-FileCopyrightText: 2016 Boudewijn Rempt <boud@valdyas.org>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
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
#include <kis_clone_layer.h>

static const char CURRENT_DTD_VERSION[] = "2.0";

KraConverter::KraConverter(KisDocument *doc)
    : m_doc(doc)
    , m_image(doc->savingImage())
{
}

KraConverter::KraConverter(KisDocument *doc, QPointer<KoUpdater> updater)
    : m_doc(doc)
    ,  m_image(doc->savingImage())
    ,  m_updater(updater)
{
}

KraConverter::~KraConverter()
{
    delete m_store;
    delete m_kraSaver;
    delete m_kraLoader;
}

void fixCloneLayers(KisImageSP image, KisNodeSP root)
{
    KisNodeSP first = root->firstChild();
    KisNodeSP node = first;
    while (!node.isNull()) {
        if (node->inherits("KisCloneLayer")) {
            KisCloneLayer* layer = dynamic_cast<KisCloneLayer*>(node.data());
            if (layer && layer->copyFrom().isNull()) {
                KisLayerSP reincarnation = layer->reincarnateAsPaintLayer();
                image->addNode(reincarnation, node->parent(), node->prevSibling());
                image->removeNode(node);
                node = reincarnation;
            }
        } else if (node->childCount() > 0) {
            fixCloneLayers(image, node);
        }
        node = node->nextSibling();
    }
}

KisImportExportErrorCode KraConverter::buildImage(QIODevice *io)
{
    m_store = KoStore::createStore(io, KoStore::Read, "", KoStore::Zip);

    if (m_store->bad()) {
        m_doc->setErrorMessage(i18n("Not a valid Krita file"));
        return ImportExportCodes::FileFormatIncorrect;
    }

    bool success = false;
    {
        if (m_store->hasFile("root") || m_store->hasFile("maindoc.xml")) {   // Fallback to "old" file format (maindoc.xml)
            QDomDocument doc;

            KisImportExportErrorCode res = oldLoadAndParse(m_store, "root", doc);
            if (res.isOk())
                res = loadXML(doc, m_store);
            if (!res.isOk()) {
                return res;
            }

        } else {
            errUI << "ERROR: No maindoc.xml" << endl;
            m_doc->setErrorMessage(i18n("Invalid document: no file 'maindoc.xml'."));
            return ImportExportCodes::FileFormatIncorrect;
        }

        if (m_store->hasFile("documentinfo.xml")) {
            QDomDocument doc;
            KisImportExportErrorCode resultHere = oldLoadAndParse(m_store, "documentinfo.xml", doc);
            if (resultHere.isOk()) {
                m_doc->documentInfo()->load(doc);
            }
        }
        success = completeLoading(m_store);
    }

    fixCloneLayers(m_image, m_image->root());

    return success ? ImportExportCodes::OK : ImportExportCodes::Failure;
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

StoryboardItemList KraConverter::storyboardItemList()
{
    return m_storyboardItemList;
}

QVector<StoryboardComment> KraConverter::storyboardCommentList()
{
    return m_storyboardCommentList;
}

KisImportExportErrorCode KraConverter::buildFile(QIODevice *io, const QString &filename, bool addMergedImage)
{
    if (m_image->size().isEmpty()) {
        return ImportExportCodes::Failure;
    }
    
    setProgress(5);
    m_store = KoStore::createStore(io, KoStore::Write, m_doc->nativeFormatMimeType(), KoStore::Zip);

    if (m_store->bad()) {
        m_doc->setErrorMessage(i18n("Could not create the file for saving"));
        return ImportExportCodes::CannotCreateFile;
    }

    setProgress(20);

    m_kraSaver = new KisKraSaver(m_doc, filename, addMergedImage);

    KisImportExportErrorCode resultCode = saveRootDocuments(m_store);

    if (!resultCode.isOk()) {
        return resultCode;
    }

    setProgress(40);
    bool result;

    result = m_kraSaver->saveKeyframes(m_store, m_doc->path(), true);
    if (!result) {
        qWarning() << "saving key frames failed";
    }
    setProgress(60);
    result = m_kraSaver->saveBinaryData(m_store, m_image, m_doc->path(), true, addMergedImage);
    if (!result) {
        qWarning() << "saving binary data failed";
    }
    setProgress(70);
    result = m_kraSaver->savePalettes(m_store, m_image, m_doc->path());
    if (!result) {
        qWarning() << "saving palettes data failed";
    }

    result = m_kraSaver->saveStoryboard(m_store, m_image, m_doc->path());
    if (!result) {
        qWarning() << "Saving storyboard data failed";
    }

    result = m_kraSaver->saveAnimationMetadata(m_store, m_image, m_doc->path());
    if (!result) {
        qWarning() << "Saving animation metadata failed";
    }

    setProgress(80);
    if (!m_store->finalize()) {
        return ImportExportCodes::Failure;
    }

    if (!m_kraSaver->errorMessages().isEmpty()) {
        m_doc->setErrorMessage(m_kraSaver->errorMessages().join(".\n"));
        return ImportExportCodes::Failure;
    }
    setProgress(90);
    return ImportExportCodes::OK;
}

KisImportExportErrorCode KraConverter::saveRootDocuments(KoStore *store)
{
    dbgFile << "Saving root";
    if (store->open("root")) {
        KoStoreDevice dev(store);
        if (!saveToStream(&dev) || !store->close()) {
            dbgUI << "saveToStream failed";
            return ImportExportCodes::NoAccessToWrite;
        }
    } else {
        m_doc->setErrorMessage(i18n("Not able to write '%1'. Partition full?", QString("maindoc.xml")));
        return ImportExportCodes::ErrorWhileWriting;
    }

    if (store->open("documentinfo.xml")) {
        QDomDocument doc = KisDocument::createDomDocument("document-info"
                                                          /*DTD name*/, "document-info" /*tag name*/, "1.1");
        doc = m_doc->documentInfo()->save(doc);
        KoStoreDevice dev(store);
        QByteArray s = doc.toByteArray(); // this is already Utf8!
        bool success = dev.write(s.data(), s.size());
        if (!success) {
            return ImportExportCodes::ErrorWhileWriting;
        }
        store->close();
    } else {
        return ImportExportCodes::Failure;
    }

    if (store->open("preview.png")) {
        // ### TODO: missing error checking (The partition could be full!)
        KisImportExportErrorCode result = savePreview(store);
        (void)store->close();
        if (!result.isOk()) {
            return result;
        }
    } else {
        return ImportExportCodes::Failure;
    }

    dbgUI << "Saving done of url:" << m_doc->path();
    return ImportExportCodes::OK;
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
    root.setAttribute("syntaxVersion", CURRENT_DTD_VERSION);
    root.setAttribute("kritaVersion", KritaVersionWrapper::versionString(false));

    root.appendChild(m_kraSaver->saveXML(doc, m_image));

    if (!m_kraSaver->errorMessages().isEmpty()) {
        m_doc->setErrorMessage(m_kraSaver->errorMessages().join(".\n"));
    }
    return doc;
}

KisImportExportErrorCode KraConverter::savePreview(KoStore *store)
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
        return ImportExportCodes::NoAccessToWrite;
    }
    bool ret = preview.save(&io, "PNG");
    io.close();
    return ret ? ImportExportCodes::OK : ImportExportCodes::ErrorWhileWriting;
}


KisImportExportErrorCode KraConverter::oldLoadAndParse(KoStore *store, const QString &filename, QDomDocument &xmldoc)
{
    //dbgUI <<"Trying to open" << filename;

    if (!store->open(filename)) {
        warnUI << "Entry " << filename << " not found!";
        m_doc->setErrorMessage(i18n("Could not find %1", filename));
        return ImportExportCodes::FileNotExist;
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
        m_doc->setErrorMessage(i18n("Parsing error in %1 at line %2, column %3\nError message: %4",
                                    filename, errorLine, errorColumn,
                                    QCoreApplication::translate("QXml", errorMsg.toUtf8(), 0)));
        return ImportExportCodes::FileFormatIncorrect;
    }
    dbgUI << "File" << filename << " loaded and parsed";
    return ImportExportCodes::OK;
}

KisImportExportErrorCode KraConverter::loadXML(const QDomDocument &doc, KoStore *store)
{
    Q_UNUSED(store);

    QDomElement root;
    QDomNode node;

    if (doc.doctype().name() != "DOC") {
       errUI << "The format is not supported or the file is corrupted";
       m_doc->setErrorMessage(i18n("The format is not supported or the file is corrupted"));
       return ImportExportCodes::FileFormatIncorrect;
    }
    root = doc.documentElement();
    int syntaxVersion = root.attribute("syntaxVersion", "3").toInt();
    if (syntaxVersion > 2) {
        errUI << "The file is too new for this version of Krita:" << syntaxVersion;
        m_doc->setErrorMessage(i18n("The file is too new for this version of Krita (%1).", syntaxVersion));
        return ImportExportCodes::FormatFeaturesUnsupported;
    }

    if (!root.hasChildNodes()) {
        errUI << "The file has no layers.";
        m_doc->setErrorMessage(i18n("The file has no layers."));
        return ImportExportCodes::FileFormatIncorrect;
    }

    m_kraLoader = new KisKraLoader(m_doc, syntaxVersion);

    // reset the old image before loading the next one
    m_doc->setCurrentImage(0, false);

    for (node = root.firstChild(); !node.isNull(); node = node.nextSibling()) {
        if (node.isElement()) {
            if (node.nodeName() == "IMAGE") {
                QDomElement elem = node.toElement();
                if (!(m_image = m_kraLoader->loadXML(elem))) {

                    if (m_kraLoader->errorMessages().isEmpty()) {
                        errUI << "Unknown error while opening the .kra file.";
                        m_doc->setErrorMessage(i18n("Unknown error."));
                    }
                    else {
                        m_doc->setErrorMessage(m_kraLoader->errorMessages().join("\n"));
                        errUI << m_kraLoader->errorMessages().join("\n");
                    }
                    return ImportExportCodes::Failure;
                }

                // HACK ALERT!
                m_doc->hackPreliminarySetImage(m_image);

                return ImportExportCodes::OK;
            }
            else {
                if (m_kraLoader->errorMessages().isEmpty()) {
                    m_doc->setErrorMessage(i18n("The file does not contain an image."));
                }
                return ImportExportCodes::FileFormatIncorrect;
            }
        }
    }
    return ImportExportCodes::Failure;
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

    m_image->disableDirtyRequests();

    QString layerPathName = m_kraLoader->imageName();
    if (!m_store->hasDirectory(layerPathName)) {
        // We might be hitting an encoding problem. Get the only folder in the toplevel
        Q_FOREACH (const QString &entry, m_store->directoryList()) {
            if (entry.contains("/layers/")) {
                layerPathName = entry.split("/layers/").first();
                m_store->setSubstitution(m_kraLoader->imageName(), layerPathName);
                break;
            }
        }
    }

    m_kraLoader->loadBinaryData(store, m_image, m_doc->localFilePath(), true);
    m_kraLoader->loadPalettes(store, m_doc);
    m_kraLoader->loadStoryboards(store, m_doc);
    m_kraLoader->loadAnimationMetadata(store, m_image);

    if (!m_kraLoader->errorMessages().isEmpty()) {
        m_doc->setErrorMessage(m_kraLoader->errorMessages().join("\n"));
        return false;
    }

    m_image->enableDirtyRequests();

    if (!m_kraLoader->warningMessages().isEmpty()) {
        // warnings do not interrupt loading process, so we do not return here
        m_doc->setWarningMessage(m_kraLoader->warningMessages().join("\n"));
    }

    m_activeNodes = m_kraLoader->selectedNodes();
    m_assistants = m_kraLoader->assistants();
    m_storyboardItemList = m_kraLoader->storyboardItemList();
    m_storyboardCommentList = m_kraLoader->storyboardCommentList();

    return true;
}

void KraConverter::cancel()
{
    m_stop = true;
}

void KraConverter::setProgress(int progress)
{
    if (m_updater) {
        m_updater->setProgress(progress);
    }
}

