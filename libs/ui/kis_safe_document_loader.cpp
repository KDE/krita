/*
 *  SPDX-FileCopyrightText: 2013 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_safe_document_loader.h"

#include <QTimer>
#include <QFileSystemWatcher>
#include <QApplication>
#include <QFileInfo>
#include <QDir>
#include <QUrl>

#include <KoStore.h>

#include <kis_paint_layer.h>
#include <kis_group_layer.h>
#include "KisDocument.h"
#include <kis_image.h>
#include "kis_signal_compressor.h"
#include "KisPart.h"
#include "KisUsageLogger.h"
#include "KisFileSystemWatcherWrapper.h"

Q_GLOBAL_STATIC(KisFileSystemWatcherWrapper, s_fileSystemWatcher)


struct KisSafeDocumentLoader::Private
{
    Private()
        : fileChangedSignalCompressor(500 /* ms */, KisSignalCompressor::POSTPONE)
    {
    }

    QScopedPointer<KisDocument> doc;
    KisSignalCompressor fileChangedSignalCompressor;
    bool isLoading = false;
    bool fileChangedFlag = false;
    QString path;
    QString temporaryPath;

    qint64 initialFileSize {0};
    QDateTime initialFileTimeStamp;

    int failureCount {0};
};

KisSafeDocumentLoader::KisSafeDocumentLoader(const QString &path, QObject *parent)
    : QObject(parent),
      m_d(new Private())
{
    connect(s_fileSystemWatcher, SIGNAL(fileChanged(QString)),
            SLOT(fileChanged(QString)));

    connect(&m_d->fileChangedSignalCompressor, SIGNAL(timeout()),
            SLOT(fileChangedCompressed()));

    setPath(path);
}

KisSafeDocumentLoader::~KisSafeDocumentLoader()
{
    if (!m_d->path.isEmpty()) {
        s_fileSystemWatcher->removePath(m_d->path);
    }

    delete m_d;
}

void KisSafeDocumentLoader::setPath(const QString &path)
{
    if (path.isEmpty()) return;

    if (!m_d->path.isEmpty()) {
        s_fileSystemWatcher->removePath(m_d->path);
    }

    m_d->path = path;
    s_fileSystemWatcher->addPath(m_d->path);
}

void KisSafeDocumentLoader::reloadImage()
{
    fileChangedCompressed(true);
}

void KisSafeDocumentLoader::fileChanged(QString path)
{
    if (KisFileSystemWatcherWrapper::unifyFilePath(m_d->path) == path) {
        m_d->fileChangedFlag = true;
        m_d->fileChangedSignalCompressor.start();
    }
}

void KisSafeDocumentLoader::fileChangedCompressed(bool sync)
{
    if (m_d->isLoading) return;

    QFileInfo initialFileInfo(m_d->path);
    m_d->initialFileSize = initialFileInfo.size();
    m_d->initialFileTimeStamp = initialFileInfo.lastModified();

    // it may happen when the file is flushed by
    // so other application
    if (!m_d->initialFileSize) return;

    m_d->isLoading = true;
    m_d->fileChangedFlag = false;

    m_d->temporaryPath =
            QDir::tempPath() + '/' +
            QString("krita_file_layer_copy_%1_%2.%3")
            .arg(QApplication::applicationPid())
            .arg(qrand())
            .arg(initialFileInfo.suffix());

    QFile::copy(m_d->path, m_d->temporaryPath);


    if (!sync) {
        QTimer::singleShot(100, this, SLOT(delayedLoadStart()));
    } else {
        QApplication::processEvents();
        delayedLoadStart();
    }
}

void KisSafeDocumentLoader::delayedLoadStart()
{
    QFileInfo originalInfo(m_d->path);
    QFileInfo tempInfo(m_d->temporaryPath);
    bool successfullyLoaded = false;

    if (!m_d->fileChangedFlag &&
            originalInfo.size() == m_d->initialFileSize &&
            originalInfo.lastModified() == m_d->initialFileTimeStamp &&
            tempInfo.size() == m_d->initialFileSize) {

        m_d->doc.reset(KisPart::instance()->createDocument());

        if (m_d->path.toLower().endsWith("ora") || m_d->path.toLower().endsWith("kra")) {
            QScopedPointer<KoStore> store(KoStore::createStore(m_d->temporaryPath, KoStore::Read));
            if (store && !store->bad()) {
                if (store->open(QString("mergedimage.png"))) {
                    QByteArray bytes = store->read(store->size());
                    store->close();
                    QImage mergedImage;
                    mergedImage.loadFromData(bytes);
                    Q_ASSERT(!mergedImage.isNull());
                    KisImageSP image = new KisImage(0, mergedImage.width(), mergedImage.height(), KoColorSpaceRegistry::instance()->rgb8(), "");
                    KisPaintLayerSP layer = new KisPaintLayer(image, "", OPACITY_OPAQUE_U8);
                    layer->paintDevice()->convertFromQImage(mergedImage, 0);
                    image->addNode(layer, image->rootLayer());
                    image->initialRefreshGraph();
                    m_d->doc->setCurrentImage(image);
                    successfullyLoaded = true;
                }
                else {
                    qWarning() << "delayedLoadStart: Could not open mergedimage.png";
                }
            }
            else {
                qWarning() << "delayedLoadStart: Store was bad";
            }
        }
        else {
            successfullyLoaded = m_d->doc->openPath(m_d->temporaryPath,
                                                   KisDocument::DontAddToRecent);
        }
    } else {
        dbgKrita << "File was modified externally. Restarting.";
        dbgKrita << ppVar(m_d->fileChangedFlag);
        dbgKrita << ppVar(m_d->initialFileSize);
        dbgKrita << ppVar(m_d->initialFileTimeStamp);
        dbgKrita << ppVar(originalInfo.size());
        dbgKrita << ppVar(originalInfo.lastModified());
        dbgKrita << ppVar(tempInfo.size());
    }

    QFile::remove(m_d->temporaryPath);
    m_d->isLoading = false;

    if (!successfullyLoaded) {
        // Restart the attempt
        m_d->failureCount++;
        if (m_d->failureCount >= 3) {
            emit loadingFailed();
        }
        else {
            m_d->fileChangedSignalCompressor.start();
        }
    }
    else {
        KisPaintDeviceSP paintDevice = new KisPaintDevice(m_d->doc->image()->colorSpace());
        KisPaintDeviceSP projection = m_d->doc->image()->projection();
        paintDevice->makeCloneFrom(projection, projection->extent());
        emit loadingFinished(paintDevice,
                             m_d->doc->image()->xRes(),
                             m_d->doc->image()->yRes(),
                             m_d->doc->image()->size());
    }

    m_d->doc.reset();
}

#include "kis_safe_document_loader.moc"
