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
#include "KisImportExportManager_p.h"
#include "KisDocument.h"
#include "KisDocumentEntry.h"
#include "KoProgressUpdater.h"
#include "KoJsonTrader.h"

#include <QFile>
#include <QLabel>
#include <QVBoxLayout>
#include <QList>
#include <QApplication>
#include <QByteArray>
#include <QPluginLoader>
#include <QFileInfo>

#include <klocalizedstring.h>
#include <QMessageBox>
#include <ksqueezedtextlabel.h>

#include <kis_debug.h>

#include <queue>

#include <unistd.h>

// static cache for import and export mimetypes
QStringList KisImportExportManager::m_importMimeTypes;
QStringList KisImportExportManager::m_exportMimeTypes;

KisImportExportManager::KisImportExportManager(KisDocument* document)
    : m_document(document)
    , m_graph("")
    , d(new Private(0))
{
    d->batch = false;
}


KisImportExportManager::KisImportExportManager(const QString& location)
    : m_document(0)
    , m_importUrl(locationToUrl(location))
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
    QUrl u = locationToUrl(location);

    // Find the mime type for the file to be imported.
    QString  typeName(documentMimeType);
    QMimeType t;
    if (documentMimeType.isEmpty()) {
        QMimeDatabase db;
        db.mimeTypeForFile(u.path(), QMimeDatabase::MatchExtension);
        if (t.isValid()) {
            typeName = t.name();
        }
        else {
            // Find the right mimetype by the extension
            KoJsonTrader trader;
            QStringList mimes = trader.instance()->mimeTypes(QFileInfo(location).suffix());
            typeName = mimes.first();
        }
    }
    m_graph.setSourceMimeType(typeName.toLatin1()); // .latin1() is okay here (Werner)

    if (!m_graph.isValid()) {
        bool userCancelled = false;

        warnFile << "Can't open " << typeName << ", trying filter chooser";
        if (m_document) {
            if (!m_document->isAutoErrorHandlingEnabled()) {
                status = KisImportExportFilter::BadConversionGraph;
                return QString();
            }
            QByteArray nativeFormat = m_document->nativeFormatMimeType();

            QApplication::setOverrideCursor(Qt::ArrowCursor);
            KisFilterChooser chooser(0,
                                     KisImportExportManager::mimeFilter(nativeFormat, KisImportExportManager::Import,
                                                                        m_document->extraNativeMimeTypes()), nativeFormat, u);
            if (chooser.exec()) {
                QByteArray f = chooser.filterSelected().toLatin1();
                if (f == nativeFormat) {
                    status = KisImportExportFilter::OK;
                    QApplication::restoreOverrideCursor();
                    return u.toString();
                }

                m_graph.setSourceMimeType(f);
            } else
                userCancelled = true;
            QApplication::restoreOverrideCursor();
        }

        if (!m_graph.isValid()) {
            errFile << "Couldn't create a valid graph for this source mimetype: "
                    << typeName;
            importErrorHelper(typeName, userCancelled);
            status = KisImportExportFilter::BadConversionGraph;
            return QString();
        }
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
    } else if (!d->importMimeType.isEmpty()) {
        chain = m_graph.chain(this, d->importMimeType);
    } else {
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
    m_importUrl = u;
    m_exportUrl.clear();
    status = chain->invokeChain();

    m_importUrl.clear();  // Reset the import URL

    if (status == KisImportExportFilter::OK)
        return chain->chainOutput();
    return QString();
}

KisImportExportFilter::ConversionStatus KisImportExportManager::exportDocument(const QString& location, QByteArray& mimeType)
{
    bool userCancelled = false;

    // The import url should already be set correctly (null if we have a KisDocument
    // file manager and to the correct URL if we have an embedded manager)
    m_direction = Export; // vital information!
    m_exportUrl = locationToUrl(location);

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
        QMimeDatabase db;
        QMimeType t = db.mimeTypeForUrl(m_importUrl);
        if (!t.isValid() || t.isDefault()) {
            errFile << "No mimetype found for" << m_importUrl.toDisplayString();
            return KisImportExportFilter::BadMimeType;
        }
        m_graph.setSourceMimeType(t.name().toLatin1());

        if (!m_graph.isValid()) {
            warnFile << "Can't open" << t.name() << ", trying filter chooser";

            QApplication::setOverrideCursor(Qt::ArrowCursor);
            KisFilterChooser chooser(0, KisImportExportManager::mimeFilter("", KisImportExportManager::Export), QString(), m_importUrl);
            if (chooser.exec())
                m_graph.setSourceMimeType(chooser.filterSelected().toLatin1());
            else
                userCancelled = true;

            QApplication::restoreOverrideCursor();
        }
    }

    if (!m_graph.isValid()) {
        errFile << "Couldn't create a valid graph for this source mimetype.";
        if (!d->batch && !userCancelled) {
            QMessageBox::critical(0, i18nc("@title:window", "Krita"), i18n("Could not export file: the export filter is missing."));
        }
        return KisImportExportFilter::BadConversionGraph;
    }

    if (!chain)   // already set when coming from the m_document case
        chain = m_graph.chain(this, mimeType);

    if (!chain) {
        errFile << "Couldn't create a valid filter chain to " << mimeType << " !" << endl;
        if (!d->batch) {
            QMessageBox::critical(0, i18nc("@title:window", "Krita"), i18n("Could not export file: the export filter is missing."));
        }
        return KisImportExportFilter::BadConversionGraph;
    }

    return chain->invokeChain();
}

// The static method to figure out to which parts of the
// graph this mimetype has a connection to.
QStringList KisImportExportManager::mimeFilter(const QByteArray &mimetype, Direction direction, const QStringList &extraNativeMimeTypes)
{
    Q_UNUSED(mimetype);
    Q_UNUSED(extraNativeMimeTypes);

    // Find the right mimetype by the extension
    QSet<QString> mimeTypes;

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
            m_exportMimeTypes = mimeTypes.toList();
        }
        qDebug() << m_exportMimeTypes;
        return m_exportMimeTypes;
    }
    return QStringList();
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

QUrl KisImportExportManager::locationToUrl(QString location) const
{
    QUrl u = QUrl::fromLocalFile(location);
    return (u.isEmpty()) ? QUrl(location) : u;
}

#include <QMimeDatabase>
#include <QMimeType>
