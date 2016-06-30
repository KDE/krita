/* This file is part of the KDE project
   Copyright (C) 1998, 1999 Torben Weis <weis@kde.org>
                 2000, 2001 Werner Trobin <trobin@kde.org>
   Copyright (C) 2004 Nicolas Goutte <goutte@kde.org>
   Copyright (C) 2009 Thomas Zander <zander@kde.org>

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

#include "KisImportExportManager.h"

#include <QFile>
#include <QLabel>
#include <QVBoxLayout>
#include <QList>
#include <QApplication>
#include <QByteArray>
#include <QPluginLoader>
#include <QFileInfo>
#include <QMessageBox>
#include <QJsonObject>

#include <klocalizedstring.h>
#include <ksqueezedtextlabel.h>
#include <kpluginfactory.h>

#include "KoProgressUpdater.h"
#include "KoJsonTrader.h"

#include <kis_debug.h>
#include <KisMimeDatabase.h>

#include "KisImportExportFilter.h"
#include "KisDocument.h"

#include <queue>
#include <unistd.h>

// static cache for import and export mimetypes
QStringList KisImportExportManager::m_importMimeTypes;
QStringList KisImportExportManager::m_exportMimeTypes;

class Q_DECL_HIDDEN KisImportExportManager::Private
{
public:
    bool batch;
    QByteArray importMimeType;
    QWeakPointer<KoProgressUpdater> progressUpdater;

    Private(KoProgressUpdater *progressUpdater_ = 0)
        : progressUpdater(progressUpdater_)
    {
    }

};

KisImportExportManager::KisImportExportManager(KisDocument* document)
    : m_document(document)
    , m_graph("")
    , d(new Private(0))
{
    d->batch = false;
}


KisImportExportManager::KisImportExportManager(const QString& location)
    : m_document(0)
    , m_importFileName(location)
    , m_graph("")
    , d(new Private)
{
    d->batch = false;
}

KisImportExportManager::KisImportExportManager(const QByteArray& mimeType)
    : m_document(0)
    , m_graph("")
    , d(new Private)
{
    d->batch = false;
    d->importMimeType = mimeType;
}

KisImportExportManager::~KisImportExportManager()
{
    delete d;
}

QString KisImportExportManager::importDocument(const QString& location,
                                               const QString& documentMimeType,
                                               KisImportExportFilter::ConversionStatus& status)
{
    // Find the mime type for the file to be imported.
    QString  typeName = documentMimeType;
    if (typeName.isEmpty()) {
        typeName = KisMimeDatabase::mimeTypeForFile(location);
    }
    m_graph.setSourceMimeType(typeName.toLatin1()); // .latin1() is okay here (Werner)

    if (!m_graph.isValid()) {
        errFile << "Couldn't create a valid graph for this source mimetype: "
                << typeName;
        status = KisImportExportFilter::BadConversionGraph;
        return QString();
    }

    KisFilterChainSP chain(0);
    // Are we owned by a KisDocument?
    if (m_document) {
        QByteArray mimeType = m_document->nativeFormatMimeType();
        QStringList extraMimes = m_document->extraNativeMimeTypes();
        int i = 0;
        int n = extraMimes.count();
        chain = m_graph.chain(this, mimeType);
        while (i < n) {
            QByteArray extraMime = extraMimes[i].toUtf8();
            // TODO check if its the same target mime then continue
            KisFilterChainSP newChain(0);
            newChain = m_graph.chain(this, extraMime);
            if (!chain || (newChain && newChain->weight() < chain->weight()))
                chain = newChain;
            ++i;
        }
    }
    else if (!d->importMimeType.isEmpty()) {
        chain = m_graph.chain(this, d->importMimeType);
    }
    else {
        errFile << "You aren't supposed to use import() from a filter!" << endl;
        status = KisImportExportFilter::UsageError;
        return QString();
    }

    if (!chain) {
        errFile << "Couldn't create a valid filter chain!" << endl;
        importErrorHelper(typeName);
        status = KisImportExportFilter::BadConversionGraph;
        return QString();
    }

    // Okay, let's invoke the filters one after the other
    m_direction = Import; // vital information!
    m_importFileName = location;
    m_exportFileName.clear();
    status = chain->invokeChain();

    m_importFileName.clear();  // Reset the import URL

    if (status == KisImportExportFilter::OK)
        return chain->chainOutput();
    return QString();
}

KisImportExportFilter::ConversionStatus KisImportExportManager::exportDocument(const QString& location, QByteArray& mimeType, KisPropertiesConfigurationSP exportConfiguration)
{
    bool userCancelled = false;

    // The import url should already be set correctly (null if we have a KisDocument
    // file manager and to the correct URL if we have an embedded manager)
    m_direction = Export; // vital information!
    m_exportFileName = location;
    m_exportConfiguration = exportConfiguration;

    KisFilterChainSP chain;

    if (m_document) {
        // We have to pick the right native mimetype as source.
        QStringList nativeMimeTypes;
        nativeMimeTypes.append(m_document->nativeFormatMimeType());
        nativeMimeTypes += m_document->extraNativeMimeTypes();
        QStringList::ConstIterator it = nativeMimeTypes.constBegin();
        const QStringList::ConstIterator end = nativeMimeTypes.constEnd();
        for (; !chain && it != end; ++it) {
            m_graph.setSourceMimeType((*it).toLatin1());
            if (m_graph.isValid())
                chain = m_graph.chain(this, mimeType);
        }
    }
    else {
        QString t = KisMimeDatabase::mimeTypeForFile(m_importFileName);
        m_graph.setSourceMimeType(t.toLatin1());
    }

    if (!m_graph.isValid()) {
        errFile << "Couldn't create a valid graph for this source mimetype.";
        QApplication::restoreOverrideCursor();
        if (!d->batch && !userCancelled) {
            QMessageBox::critical(0, i18nc("@title:window", "Krita"), i18n("Could not export file: the export filter is missing."));
        }
        return KisImportExportFilter::BadConversionGraph;
    }

    if (!chain)  {  // already set when coming from the m_document case
        chain = m_graph.chain(this, mimeType);
    }

    if (!chain) {
        errFile << "Couldn't create a valid filter chain to " << mimeType << " !" << endl;
        if (!d->batch) {
            QMessageBox::critical(0, i18nc("@title:window", "Krita"), i18n("Could not export file: the export filter is missing."));
        }
        QApplication::restoreOverrideCursor();
        return KisImportExportFilter::BadConversionGraph;
    }

    return chain->invokeChain();
}

// The static method to figure out to which parts of the
// graph this mimetype has a connection to.
QStringList KisImportExportManager::mimeFilter(Direction direction)
{
    // Find the right mimetype by the extension
    QSet<QString> mimeTypes;
//    mimeTypes << KisDocument::nativeFormatMimeType() << "application/x-krita-paintoppreset" << "image/openraster";

    if (direction == KisImportExportManager::Import) {
        if (m_importMimeTypes.isEmpty()) {
            KoJsonTrader trader;
            QList<QPluginLoader *>list = trader.query("Krita/FileFilter", "");
            Q_FOREACH(QPluginLoader *loader, list) {
                QJsonObject json = loader->metaData().value("MetaData").toObject();
                Q_FOREACH(const QString &mimetype, json.value("X-KDE-Import").toString().split(",")) {
                    mimeTypes << mimetype;
                }
            }
            qDeleteAll(list);
            m_importMimeTypes = mimeTypes.toList();
        }
        return m_importMimeTypes;
    }
    else if (direction == KisImportExportManager::Export) {
        if (m_exportMimeTypes.isEmpty()) {
            KoJsonTrader trader;
            QList<QPluginLoader *>list = trader.query("Krita/FileFilter", "");
            Q_FOREACH(QPluginLoader *loader, list) {
                QJsonObject json = loader->metaData().value("MetaData").toObject();
                Q_FOREACH(const QString &mimetype, json.value("X-KDE-Export").toString().split(",")) {
                    mimeTypes << mimetype;
                }
            }
            qDeleteAll(list);
            m_exportMimeTypes = mimeTypes.toList();
        }
        qDebug() << m_exportMimeTypes;
        return m_exportMimeTypes;
    }
    return QStringList();
}

KisImportExportFilter *KisImportExportManager::filterForMimeType(const QString &mimetype, KisImportExportManager::Direction direction)
{
    int weight = -1;
    KisImportExportFilter *filter = 0;
    KoJsonTrader trader;
    QList<QPluginLoader *>list = trader.query("Krita/FileFilter", "");
    Q_FOREACH(QPluginLoader *loader, list) {
        QJsonObject json = loader->metaData().value("MetaData").toObject();
        QString directionKey = direction == Export ? "X-KDE-Export" : "X-KDE-Import";
        if (json.value(directionKey).toString().split(",").contains(mimetype)) {
            KLibFactory *factory = qobject_cast<KLibFactory *>(loader->instance());

            if (!factory) {
                warnUI << loader->errorString();
                continue;
            }

            QObject* obj = factory->create<KisImportExportFilter>(0);
            if (!obj || !obj->inherits("KisImportExportFilter")) {
                delete obj;
                continue;
            }

            KisImportExportFilter *f = static_cast<KisImportExportFilter*>(obj);
            if (!f) {
                delete obj;
                continue;
            }

            int w = json.value("X-KDE-Weight").toInt();
            if (w > weight) {
                delete filter;
                filter = f;
                f->setObjectName(loader->fileName());
                weight = w;
            }
        }
    }
    qDeleteAll(list);
    return filter;
}

void KisImportExportManager::importErrorHelper(const QString& mimeType, const bool suppressDialog)
{
    // ###### FIXME: use KLibLoader::lastErrorMessage() here
    if (!suppressDialog) {
        QMessageBox::critical(0, i18nc("@title:window", "Krita"), i18n("Could not import file of type\n%1. The import filter is missing.", mimeType));
    }
}

void KisImportExportManager::setBatchMode(const bool batch)
{
    d->batch = batch;
}

bool KisImportExportManager::getBatchMode(void) const
{
    return d->batch;
}

void KisImportExportManager::setProgresUpdater(KoProgressUpdater *updater)
{
    d->progressUpdater = updater;
}

KoProgressUpdater* KisImportExportManager::progressUpdater() const
{
    if (d->progressUpdater.isNull()) {
        // somebody, probably its parent, deleted our progress updater for us
        return 0;
    }
    return d->progressUpdater.data();
}

#include <KisMimeDatabase.h>
