/* This file is part of the KOffice libraries
   Copyright (C) 2001 Werner Trobin <trobin@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
*/

#include "KoFilterChain.h"

#include "KoFilterManager.h"  // KoFilterManager::filterAvailable, private API
#include "KoDocumentEntry.h"
#include "KoFilterEntry.h"
#include "KoDocument.h"

#include "PriorityQueue_p.h"
#include "KoFilterGraph.h"
#include "KoFilterEdge.h"
#include "KoFilterChainLink.h"
#include "KoFilterVertex.h"

#include <QMetaMethod>
#include <ktemporaryfile.h>
#include <kmimetype.h>
#include <kdebug.h>

#include <limits.h> // UINT_MAX

// Those "defines" are needed in the setupConnections method below.
// Please always keep the strings and the length in sync!
using namespace KOfficeFilter;


KoFilterChain::KoFilterChain(const KoFilterManager* manager) :
        m_manager(manager), m_state(Beginning), m_inputStorage(0),
        m_inputStorageDevice(0), m_outputStorage(0), m_outputStorageDevice(0),
        m_inputDocument(0), m_outputDocument(0), m_inputTempFile(0),
        m_outputTempFile(0), m_inputQueried(Nil), m_outputQueried(Nil), d(0)
{
}


KoFilterChain::~KoFilterChain()
{
    m_chainLinks.deleteAll();

    if (filterManagerParentChain() && filterManagerParentChain()->m_outputStorage)
        filterManagerParentChain()->m_outputStorage->leaveDirectory();
    manageIO(); // Called for the 2nd time in a row -> clean up
}

KoFilter::ConversionStatus KoFilterChain::invokeChain()
{
    KoFilter::ConversionStatus status = KoFilter::OK;

    m_state = Beginning;
    int count = m_chainLinks.count();

    // This is needed due to nasty Microsoft design
    const ChainLink* parentChainLink = 0;
    if (filterManagerParentChain())
        parentChainLink = filterManagerParentChain()->m_chainLinks.current();

    // No iterator here, as we need m_chainLinks.current() in outputDocument()
    m_chainLinks.first();
    for (; count > 1 && m_chainLinks.current() && status == KoFilter::OK;
            m_chainLinks.next(), --count) {
        status = m_chainLinks.current()->invokeFilter(parentChainLink);
        m_state = Middle;
        manageIO();
    }

    if (!m_chainLinks.current()) {
        kWarning(30500) << "Huh?? Found a null pointer in the chain";
        return KoFilter::StupidError;
    }

    if (status == KoFilter::OK) {
        if (m_state & Beginning)
            m_state |= End;
        else
            m_state = End;
        status = m_chainLinks.current()->invokeFilter(parentChainLink);
        manageIO();
    }

    m_state = Done;
    if (status == KoFilter::OK)
        finalizeIO();
    return status;
}

QString KoFilterChain::chainOutput() const
{
    if (m_state == Done)
        return m_inputFile; // as we already called manageIO()
    return QString();
}

QString KoFilterChain::inputFile()
{
    if (m_inputQueried == File)
        return m_inputFile;
    else if (m_inputQueried != Nil) {
        kWarning(30500) << "You already asked for some different source.";
        return QString();
    }
    m_inputQueried = File;

    if (m_state & Beginning) {
        if (static_cast<KoFilterManager::Direction>(filterManagerDirection()) ==
                KoFilterManager::Import)
            m_inputFile = filterManagerImportFile();
        else
            inputFileHelper(filterManagerKoDocument(), filterManagerImportFile());
    } else
        if (m_inputFile.isEmpty())
            inputFileHelper(m_inputDocument, QString());

    return m_inputFile;
}

QString KoFilterChain::outputFile()
{
    // sanity check: No embedded filter should ask for a plain file
    // ###### CHECK: This will break as soon as we support exporting embedding filters
    if (filterManagerParentChain())
        kWarning(30500) << "An embedded filter has to use storageFile()!";

    if (m_outputQueried == File)
        return m_outputFile;
    else if (m_outputQueried != Nil) {
        kWarning(30500) << "You already asked for some different destination.";
        return QString();
    }
    m_outputQueried = File;

    if (m_state & End) {
        if (static_cast<KoFilterManager::Direction>(filterManagerDirection()) ==
                KoFilterManager::Import)
            outputFileHelper(false);    // This (last) one gets deleted by the caller
        else
            m_outputFile = filterManagerExportFile();
    } else
        outputFileHelper(true);

    return m_outputFile;
}

KoStoreDevice* KoFilterChain::storageFile(const QString& name, KoStore::Mode mode)
{
    // ###### CHECK: This works only for import filters. Do we want something like
    // that for export filters too?
    if (m_outputQueried == Nil && mode == KoStore::Write && filterManagerParentChain())
        return storageInitEmbedding(name);

    // Plain normal use case
    if (m_inputQueried == Storage && mode == KoStore::Read &&
            m_inputStorage && m_inputStorage->mode() == KoStore::Read)
        return storageNewStreamHelper(&m_inputStorage, &m_inputStorageDevice, name);
    else if (m_outputQueried == Storage && mode == KoStore::Write &&
             m_outputStorage && m_outputStorage->mode() == KoStore::Write)
        return storageNewStreamHelper(&m_outputStorage, &m_outputStorageDevice, name);
    else if (m_inputQueried == Nil && mode == KoStore::Read)
        return storageHelper(inputFile(), name, KoStore::Read,
                             &m_inputStorage, &m_inputStorageDevice);
    else if (m_outputQueried == Nil && mode == KoStore::Write)
        return storageHelper(outputFile(), name, KoStore::Write,
                             &m_outputStorage, &m_outputStorageDevice);
    else {
        kWarning(30500) << "Oooops, how did we get here? You already asked for a"
        << " different source/destination?" << endl;
        return 0;
    }
}

KoDocument* KoFilterChain::inputDocument()
{
    if (m_inputQueried == Document)
        return m_inputDocument;
    else if (m_inputQueried != Nil) {
        kWarning(30500) << "You already asked for some different source.";
        return 0;
    }

    if ((m_state & Beginning) &&
            static_cast<KoFilterManager::Direction>(filterManagerDirection()) == KoFilterManager::Export &&
            filterManagerKoDocument())
        m_inputDocument = filterManagerKoDocument();
    else if (!m_inputDocument)
        m_inputDocument = createDocument(inputFile());

    m_inputQueried = Document;
    return m_inputDocument;
}

KoDocument* KoFilterChain::outputDocument()
{
    // sanity check: No embedded filter should ask for a document
    // ###### CHECK: This will break as soon as we support exporting embedding filters
    if (filterManagerParentChain()) {
        kWarning(30500) << "An embedded filter has to use storageFile()!";
        return 0;
    }

    if (m_outputQueried == Document)
        return m_outputDocument;
    else if (m_outputQueried != Nil) {
        kWarning(30500) << "You already asked for some different destination.";
        return 0;
    }

    if ((m_state & End) &&
            static_cast<KoFilterManager::Direction>(filterManagerDirection()) == KoFilterManager::Import &&
            filterManagerKoDocument())
        m_outputDocument = filterManagerKoDocument();
    else
        m_outputDocument = createDocument(m_chainLinks.current()->to());

    m_outputQueried = Document;
    return m_outputDocument;
}

void KoFilterChain::dump()
{
    kDebug(30500) << "########## KoFilterChain with" << m_chainLinks.count() << " members:";
    ChainLink* link = m_chainLinks.first();
    while (link) {
        link->dump();
        link = m_chainLinks.next();
    }
    kDebug(30500) << "########## KoFilterChain (done) ##########";
}

void KoFilterChain::appendChainLink(KoFilterEntry::Ptr filterEntry, const QByteArray& from, const QByteArray& to)
{
    m_chainLinks.append(new ChainLink(this, filterEntry, from, to));
}

void KoFilterChain::prependChainLink(KoFilterEntry::Ptr filterEntry, const QByteArray& from, const QByteArray& to)
{
    m_chainLinks.prepend(new ChainLink(this, filterEntry, from, to));
}

void KoFilterChain::enterDirectory(const QString& directory)
{
    // Only a little bit of checking as we (have to :} ) trust KoEmbeddingFilter
    // If the output storage isn't initialized yet, we perform that step(s) on init.
    if (m_outputStorage)
        m_outputStorage->enterDirectory(directory);
    m_internalEmbeddingDirectories.append(directory);
}

void KoFilterChain::leaveDirectory()
{
    if (m_outputStorage)
        m_outputStorage->leaveDirectory();
    if (!m_internalEmbeddingDirectories.isEmpty())
        m_internalEmbeddingDirectories.pop_back();
}

QString KoFilterChain::filterManagerImportFile() const
{
    return m_manager->importFile();
}

QString KoFilterChain::filterManagerExportFile() const
{
    return m_manager->exportFile();
}

KoDocument* KoFilterChain::filterManagerKoDocument() const
{
    return m_manager->document();
}

int KoFilterChain::filterManagerDirection() const
{
    return m_manager->direction();
}

KoFilterChain* KoFilterChain::filterManagerParentChain() const
{
    return m_manager->parentChain();
}

void KoFilterChain::manageIO()
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
    if (m_inputTempFile) {
        delete m_inputTempFile;  // autodelete
        m_inputTempFile = 0;
    }
    m_inputFile.clear();

    if (!m_outputFile.isEmpty()) {
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

    if (m_inputDocument != filterManagerKoDocument())
        delete m_inputDocument;
    m_inputDocument = m_outputDocument;
    m_outputDocument = 0;
}

void KoFilterChain::finalizeIO()
{
    // In case we export (to a file, of course) and the last
    // filter chose to output a KoDocument we have to save it.
    // Should be very rare, but well...
    // Note: m_*input*Document as we already called manageIO()
    if (m_inputDocument &&
            static_cast<KoFilterManager::Direction>(filterManagerDirection()) == KoFilterManager::Export) {
        kDebug(30500) << "Saving the output document to the export file";
        m_inputDocument->saveNativeFormat(filterManagerExportFile());
        m_inputFile = filterManagerExportFile();
    }
}

bool KoFilterChain::createTempFile(KTemporaryFile** tempFile, bool autoDelete)
{
    if (*tempFile) {
        kError(30500) << "Ooops, why is there already a temp file???" << endl;
        return false;
    }
    *tempFile = new KTemporaryFile();
    (*tempFile)->setAutoRemove(autoDelete);
    return (*tempFile)->open();
}

void KoFilterChain::inputFileHelper(KoDocument* document, const QString& alternativeFile)
{
    if (document) {
        if (!createTempFile(&m_inputTempFile)) {
            delete m_inputTempFile;
            m_inputTempFile = 0;
            m_inputFile.clear();
            return;
        }
        if (!document->saveNativeFormat(m_inputTempFile->fileName())) {
            delete m_inputTempFile;
            m_inputTempFile = 0;
            m_inputFile.clear();
            return;
        }
        m_inputFile = m_inputTempFile->fileName();
    } else
        m_inputFile = alternativeFile;
}

void KoFilterChain::outputFileHelper(bool autoDelete)
{
    if (!createTempFile(&m_outputTempFile, autoDelete)) {
        delete m_outputTempFile;
        m_outputTempFile = 0;
        m_outputFile.clear();
    } else
        m_outputFile = m_outputTempFile->fileName();
}

KoStoreDevice* KoFilterChain::storageNewStreamHelper(KoStore** storage, KoStoreDevice** device,
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

KoStoreDevice* KoFilterChain::storageHelper(const QString& file, const QString& streamName,
        KoStore::Mode mode, KoStore** storage,
        KoStoreDevice** device)
{
    if (file.isEmpty())
        return 0;
    if (*storage) {
        kDebug(30500) << "Uh-oh, we forgot to clean up...";
        return 0;
    }

    storageInit(file, mode, storage);

    if ((*storage)->bad())
        return storageCleanupHelper(storage);

    // Seems that we got a valid storage, at least. Even if we can't open
    // the stream the "user" asked us to open, we nontheless change the
    // IOState from File to Storage, as it might be possible to open other streams
    if (mode == KoStore::Read)
        m_inputQueried = Storage;
    else // KoStore::Write
        m_outputQueried = Storage;

    return storageCreateFirstStream(streamName, storage, device);
}

void KoFilterChain::storageInit(const QString& file, KoStore::Mode mode, KoStore** storage)
{
    QByteArray appIdentification("");
    if (mode == KoStore::Write) {
        // To create valid storages we also have to add the mimetype
        // magic "applicationIndentifier" to the storage.
        // As only filters with a KOffice destination should query
        // for a storage to write to, we don't check the content of
        // the mimetype here. It doesn't do a lot of harm if someome
        // "abuses" this method.
        appIdentification = m_chainLinks.current()->to();
    }
    *storage = KoStore::createStore(file, mode, appIdentification);
}

KoStoreDevice* KoFilterChain::storageInitEmbedding(const QString& name)
{
    if (m_outputStorage) {
        kWarning(30500) << "Ooops! Something's really screwed here.";
        return 0;
    }

    m_outputStorage = filterManagerParentChain()->m_outputStorage;

    if (!m_outputStorage) {
        // If the storage of the parent hasn't been initialized yet,
        // we have to do that here. Quite nasty...
        storageInit(filterManagerParentChain()->outputFile(), KoStore::Write, &m_outputStorage);

        // transfer the ownership
        filterManagerParentChain()->m_outputStorage = m_outputStorage;
        filterManagerParentChain()->m_outputQueried = Storage;
    }

    if (m_outputStorage->isOpen())
        m_outputStorage->close();  // to be on the safe side, should never happen
    if (m_outputStorage->bad())
        return storageCleanupHelper(&m_outputStorage);

    m_outputQueried = Storage;

    // Now that we have a storage we have to change the directory
    // and remember it for later!
    const int lruPartIndex = filterManagerParentChain()->m_chainLinks.current()->lruPartIndex();
    if (lruPartIndex == -1) {
        kError(30500) << "Huh! You want to use embedding features w/o inheriting KoEmbeddingFilter?" << endl;
        return storageCleanupHelper(&m_outputStorage);
    }

    if (!m_outputStorage->enterDirectory(QString("part%1").arg(lruPartIndex)))
        return storageCleanupHelper(&m_outputStorage);

    return storageCreateFirstStream(name, &m_outputStorage, &m_outputStorageDevice);
}

KoStoreDevice* KoFilterChain::storageCreateFirstStream(const QString& streamName, KoStore** storage,
        KoStoreDevice** device)
{
    // Before we go and create the first stream in this storage we
    // have to perform a little hack in case we're used by any ole-style
    // filter which utilizes internal embedding. Ugly, but well...
    if (!m_internalEmbeddingDirectories.isEmpty()) {
        QStringList::ConstIterator it = m_internalEmbeddingDirectories.constBegin();
        QStringList::ConstIterator end = m_internalEmbeddingDirectories.constEnd();
        while (it != end && (*storage)->enterDirectory(*it))
            ++it;
    }

    if (!(*storage)->open(streamName))
        return 0;

    if (*device) {
        kDebug(30500) << "Uh-oh, we forgot to clean up the storage device!";
        (*storage)->close();
        return storageCleanupHelper(storage);
    }
    *device = new KoStoreDevice(*storage);
    return *device;
}

KoStoreDevice* KoFilterChain::storageCleanupHelper(KoStore** storage)
{
    // Take care not to delete the storage of the parent chain
    if (*storage != m_outputStorage || !filterManagerParentChain() ||
            (*storage)->mode() != KoStore::Write)
        delete *storage;
    *storage = 0;
    return 0;
}

KoDocument* KoFilterChain::createDocument(const QString& file)
{
    KUrl url;
    url.setPath(file);
    KMimeType::Ptr t = KMimeType::findByUrl(url, 0, true);
    if (t->name() == KMimeType::defaultMimeType()) {
        kError(30500) << "No mimetype found for " << file << endl;
        return 0;
    }

    KoDocument *doc = createDocument(QByteArray(t->name().toLatin1()));

    if (!doc || !doc->loadNativeFormat(file)) {
        kError(30500) << "Couldn't load from the file" << endl;
        delete doc;
        return 0;
    }
    return doc;
}

KoDocument* KoFilterChain::createDocument(const QByteArray& mimeType)
{
    KoDocumentEntry entry = KoDocumentEntry::queryByMimeType(mimeType);

    if (entry.isEmpty()) {
        kError(30500) << "Couldn't find a part that can handle mimetype " << mimeType << endl;
    }

    QString errorMsg;
    KoDocument* doc = entry.createDoc(&errorMsg);   /*entries.first().createDoc();*/
    if (!doc) {
        kError(30500) << "Couldn't create the document: " << errorMsg << endl;
        return 0;
    }
    return doc;
}

int KoFilterChain::weight() const
{
    return m_chainLinks.count();
}
