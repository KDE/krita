/* This file is part of the Calligra libraries
   Copyright (C) 2001 Werner Trobin <trobin@kde.org>

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

#include "KisFilterChain.h"

#include "KisImportExportManager.h"  // KisImportExportManager::filterAvailable, private API
#include "KisDocumentEntry.h"
#include "KisDocument.h"
#include "KisPart.h"

#include "PriorityQueue_p.h"
#include "KisFilterGraph.h"
#include "KisFilterEdge.h"
#include "KisFilterChainLink.h"
#include "KisFilterVertex.h"

#include <QMetaMethod>
#include <QTemporaryFile>

#include <kis_debug.h>

#include <limits.h> // UINT_MAX
#include <QMimeDatabase>
#include <QMimeType>

// Those "defines" are needed in the setupConnections method below.
// Please always keep the strings and the length in sync!
using namespace CalligraFilter;


KisFilterChain::KisFilterChain(const KisImportExportManager* manager)
    : KisShared()
    , m_manager(manager)
    , m_state(Beginning)
    , m_inputStorage(0),
      m_inputStorageDevice(0)
    , m_outputStorage(0)
    , m_outputStorageDevice(0)
    , m_inputDocument(0)
    , m_outputDocument(0)
    , m_inputTempFile(0),
      m_outputTempFile(0)
    , m_inputQueried(Nil)
    , m_outputQueried(Nil)
    , d(0)
{
}


KisFilterChain::~KisFilterChain()
{
    m_chainLinks.deleteAll();

    if (filterManagerParentChain() && filterManagerParentChain()->m_outputStorage)
        filterManagerParentChain()->m_outputStorage->leaveDirectory();
    manageIO(); // Called for the 2nd time in a row -> clean up
}

KisImportExportFilter::ConversionStatus KisFilterChain::invokeChain()
{
    KisImportExportFilter::ConversionStatus status = KisImportExportFilter::OK;

    m_state = Beginning;
    int count = m_chainLinks.count();

    // This is needed due to nasty Microsoft design
    const ChainLink* parentChainLink = 0;
    if (filterManagerParentChain())
        parentChainLink = filterManagerParentChain()->m_chainLinks.current();

    // No iterator here, as we need m_chainLinks.current() in outputDocument()
    m_chainLinks.first();
    for (; count > 1 && m_chainLinks.current() && status == KisImportExportFilter::OK;
         m_chainLinks.next(), --count) {
        status = m_chainLinks.current()->invokeFilter(parentChainLink);
        m_state = Middle;
        manageIO();
    }

    if (!m_chainLinks.current()) {
        warnFile << "Huh?? Found a null pointer in the chain";
        return KisImportExportFilter::StupidError;
    }

    if (status == KisImportExportFilter::OK) {
        if (m_state & Beginning)
            m_state |= End;
        else
            m_state = End;
        status = m_chainLinks.current()->invokeFilter(parentChainLink);
        manageIO();
    }

    m_state = Done;
    if (status == KisImportExportFilter::OK)
        finalizeIO();
    return status;
}

QString KisFilterChain::chainOutput() const
{
    if (m_state == Done)
        return m_inputFile; // as we already called manageIO()
    return QString();
}

QString KisFilterChain::inputFile()
{
    if (m_inputQueried == File)
        return m_inputFile;
    else if (m_inputQueried != Nil) {
        warnFile << "You already asked for some different source.";
        return QString();
    }
    m_inputQueried = File;

    if (m_state & Beginning) {
        if (static_cast<KisImportExportManager::Direction>(filterManagerDirection()) ==
                KisImportExportManager::Import)
            m_inputFile = filterManagerImportFile();
        else
            inputFileHelper(filterManagerKisDocument(), filterManagerImportFile());
    } else
        if (m_inputFile.isEmpty())
            inputFileHelper(m_inputDocument, QString());

    return m_inputFile;
}

QString KisFilterChain::outputFile()
{
    if (m_outputQueried == File)
        return m_outputFile;
    else if (m_outputQueried != Nil) {
        warnFile << "You already asked for some different destination.";
        return QString();
    }
    m_outputQueried = File;

    if (m_state & End) {
        if (static_cast<KisImportExportManager::Direction>(filterManagerDirection()) ==
                KisImportExportManager::Import)
            outputFileHelper(false);    // This (last) one gets deleted by the caller
        else
            m_outputFile = filterManagerExportFile();
    } else
        outputFileHelper(true);

    return m_outputFile;
}


KisDocument* KisFilterChain::inputDocument()
{
    if (m_inputQueried == Document)
        return m_inputDocument;
    else if (m_inputQueried != Nil) {
        warnFile << "You already asked for some different source.";
        return 0;
    }

    if ((m_state & Beginning) &&
            static_cast<KisImportExportManager::Direction>(filterManagerDirection()) == KisImportExportManager::Export &&
            filterManagerKisDocument())
        m_inputDocument = filterManagerKisDocument();
    else if (!m_inputDocument)
        m_inputDocument = createDocument(inputFile());

    m_inputQueried = Document;
    return m_inputDocument;
}

KisDocument* KisFilterChain::outputDocument()
{
    if (m_outputQueried == Document)
        return m_outputDocument;
    else if (m_outputQueried != Nil) {
        warnFile << "You already asked for some different destination.";
        return 0;
    }

    if ((m_state & End) &&
            static_cast<KisImportExportManager::Direction>(filterManagerDirection()) == KisImportExportManager::Import &&
            filterManagerKisDocument())
        m_outputDocument = filterManagerKisDocument();
    else
        m_outputDocument = createDocument(m_chainLinks.current()->to());

    m_outputQueried = Document;
    return m_outputDocument;
}

void KisFilterChain::appendChainLink(KisFilterEntrySP filterEntry, const QByteArray& from, const QByteArray& to)
{
    m_chainLinks.append(new ChainLink(this, filterEntry, from, to));
}

void KisFilterChain::prependChainLink(KisFilterEntrySP filterEntry, const QByteArray& from, const QByteArray& to)
{
    m_chainLinks.prepend(new ChainLink(this, filterEntry, from, to));
}

QString KisFilterChain::filterManagerImportFile() const
{
    return m_manager->importFile();
}

QString KisFilterChain::filterManagerExportFile() const
{
    return m_manager->exportFile();
}

KisDocument* KisFilterChain::filterManagerKisDocument() const
{
    return m_manager->document();
}

int KisFilterChain::filterManagerDirection() const
{
    return m_manager->direction();
}

KisFilterChain* KisFilterChain::filterManagerParentChain() const
{
    return m_manager->parentChain();
}

void KisFilterChain::manageIO()
{
    m_inputQueried = Nil;
    m_outputQueried = Nil;

    delete m_inputStorageDevice;
    m_inputStorageDevice = 0;
    if (m_inputStorage) {
        m_inputStorage->close();
        delete m_inputStorage;
        m_inputStorage = 0;
    }
    delete m_inputTempFile;  // autodelete
    m_inputTempFile = 0;
    m_inputFile.clear();

    if (!m_outputFile.isEmpty()) {
        if (m_outputTempFile == 0) {
            m_inputTempFile = new QTemporaryFile;
            m_inputTempFile->setAutoRemove(true);
            m_inputTempFile->setFileName(m_outputFile);
        }
        else {
            m_inputTempFile = m_outputTempFile;
            m_outputTempFile = 0;
        }
        m_inputFile = m_outputFile;
        m_outputFile.clear();
        m_inputTempFile = m_outputTempFile;
        m_outputTempFile = 0;

        delete m_outputStorageDevice;
        m_outputStorageDevice = 0;
        if (m_outputStorage) {
            m_outputStorage->close();
            // Don't delete the storage if we're just pointing to the
            // storage of the parent filter chain
            if (!filterManagerParentChain() || m_outputStorage->mode() != KoStore::Write)
                delete m_outputStorage;
            m_outputStorage = 0;
        }
    }

    if (m_inputDocument != filterManagerKisDocument())
        delete m_inputDocument;
    m_inputDocument = m_outputDocument;
    m_outputDocument = 0;
}

void KisFilterChain::finalizeIO()
{
    // In case we export (to a file, of course) and the last
    // filter chose to output a KisDocument we have to save it.
    // Should be very rare, but well...
    // Note: m_*input*Document as we already called manageIO()
    if (m_inputDocument &&
            static_cast<KisImportExportManager::Direction>(filterManagerDirection()) == KisImportExportManager::Export) {
        dbgFile << "Saving the output document to the export file " << m_chainLinks.current()->to();
        m_inputDocument->setOutputMimeType(m_chainLinks.current()->to());
        m_inputDocument->saveNativeFormat(filterManagerExportFile());
        m_inputFile = filterManagerExportFile();
    }
}

bool KisFilterChain::createTempFile(QTemporaryFile** tempFile, bool autoDelete)
{
    if (*tempFile) {
        errFile << "Ooops, why is there already a temp file???" << endl;
        return false;
    }
    *tempFile = new QTemporaryFile();
    (*tempFile)->setAutoRemove(autoDelete);
    return (*tempFile)->open();
}

/*  Note about Windows & usage of QTemporaryFile

    The QTemporaryFile objects m_inputTempFile and m_outputTempFile are just used
    to reserve a temporary file with a unique name which then can be used to store
    an intermediate format. The filters themselves do not get access to these objects,
    but can query KisFilterChain only for the filename and then have to open the files
    themselves with their own file handlers (TODO: change this).
    On Windows this seems to be a problem and results in content not sync'ed to disk etc.

    So unless someone finds out which flags might be needed on opening the files on
    Windows to prevent this behaviour (unless these details are hidden away by the
    Qt abstraction and cannot be influenced), a workaround is to destruct the
    QTemporaryFile objects right after creation again and just take the name,
    to avoid having two file handlers on the same file.

    A better fix might be to use the QTemporaryFile objects also by the filters,
    instead of having them open the same file on their own again, but that needs more work
    and is left for... you :)
*/

void KisFilterChain::inputFileHelper(KisDocument* document, const QString& alternativeFile)
{
    if (document) {
        if (!createTempFile(&m_inputTempFile)) {
            delete m_inputTempFile;
            m_inputTempFile = 0;
            m_inputFile.clear();
            return;
        }
        m_inputFile = m_inputTempFile->fileName();
        // See "Note about Windows & usage of QTemporaryFile" above
#ifdef Q_OS_WIN
        m_inputTempFile->close();
        m_inputTempFile->setAutoRemove(true);
        delete m_inputTempFile;
        m_inputTempFile = 0;
#endif
        document->setOutputMimeType(m_chainLinks.current()->from());
        if (!document->saveNativeFormat(m_inputFile)) {
            delete m_inputTempFile;
            m_inputTempFile = 0;
            m_inputFile.clear();
            return;
        }
    } else
        m_inputFile = alternativeFile;
}

void KisFilterChain::outputFileHelper(bool autoDelete)
{
    if (!createTempFile(&m_outputTempFile, autoDelete)) {
        delete m_outputTempFile;
        m_outputTempFile = 0;
        m_outputFile.clear();
    } else {
        m_outputFile = m_outputTempFile->fileName();

        // See "Note about Windows & usage of QTemporaryFile" above
#ifdef Q_OS_WIN
        m_outputTempFile->close();
        m_outputTempFile->setAutoRemove(true);
        delete m_outputTempFile;
        m_outputTempFile = 0;
#endif
    }
}

KoStoreDevice* KisFilterChain::storageNewStreamHelper(KoStore** storage, KoStoreDevice** device,
                                                      const QString& name)
{
    delete *device;
    *device = 0;
    if ((*storage)->isOpen())
        (*storage)->close();
    if ((*storage)->bad())
        return storageCleanupHelper(storage);
    if (!(*storage)->open(name))
        return 0;

    *device = new KoStoreDevice(*storage);
    return *device;
}

KoStoreDevice* KisFilterChain::storageHelper(const QString& file, const QString& streamName,
                                             KoStore::Mode mode, KoStore** storage,
                                             KoStoreDevice** device)
{
    if (file.isEmpty())
        return 0;
    if (*storage) {
        dbgFile << "Uh-oh, we forgot to clean up...";
        return 0;
    }

    storageInit(file, mode, storage);

    if ((*storage)->bad())
        return storageCleanupHelper(storage);

    // Seems that we got a valid storage, at least. Even if we can't open
    // the stream the "user" asked us to open, we nonetheless change the
    // IOState from File to Storage, as it might be possible to open other streams
    if (mode == KoStore::Read)
        m_inputQueried = Storage;
    else // KoStore::Write
        m_outputQueried = Storage;

    return storageCreateFirstStream(streamName, storage, device);
}

void KisFilterChain::storageInit(const QString& file, KoStore::Mode mode, KoStore** storage)
{
    QByteArray appIdentification("");
    if (mode == KoStore::Write) {
        // To create valid storages we also have to add the mimetype
        // magic "applicationIndentifier" to the storage.
        // As only filters with a Calligra destination should query
        // for a storage to write to, we don't check the content of
        // the mimetype here. It doesn't do a lot of harm if someome
        // "abuses" this method.
        appIdentification = m_chainLinks.current()->to();
    }
    *storage = KoStore::createStore(file, mode, appIdentification);
}

KoStoreDevice* KisFilterChain::storageCreateFirstStream(const QString& streamName, KoStore** storage,
                                                        KoStoreDevice** device)
{
    if (!(*storage)->open(streamName))
        return 0;

    if (*device) {
        dbgFile << "Uh-oh, we forgot to clean up the storage device!";
        (*storage)->close();
        return storageCleanupHelper(storage);
    }
    *device = new KoStoreDevice(*storage);
    return *device;
}

KoStoreDevice* KisFilterChain::storageCleanupHelper(KoStore** storage)
{
    // Take care not to delete the storage of the parent chain
    if (*storage != m_outputStorage || !filterManagerParentChain() ||
            (*storage)->mode() != KoStore::Write)
        delete *storage;
    *storage = 0;
    return 0;
}

KisDocument* KisFilterChain::createDocument(const QString& file)
{
    QUrl url = QUrl::fromLocalFile(file);
    QMimeDatabase db;
    QMimeType t = db.mimeTypeForFile(url.path(), QMimeDatabase::MatchExtension);
    if (t.isDefault()) {
        errFile << "No mimetype found for " << file << endl;
        return 0;
    }

    KisDocument *doc = createDocument(t.name().toLatin1());

    if (!doc || !doc->loadNativeFormat(file)) {
        errFile << "Couldn't load from the file" << endl;
        delete doc;
        return 0;
    }
    return doc;
}

KisDocument* KisFilterChain::createDocument(const QByteArray& mimeType)
{
    Q_UNUSED(mimeType);
    return KisPart::instance()->createDocument();
}

int KisFilterChain::weight() const
{
    return m_chainLinks.count();
}
