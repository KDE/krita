/*
 *  SPDX-FileCopyrightText: 2021 Felipe Lema <felipelema@mortemale.org>
 *  SPDX-FileCopyrightText: 2022 Alvin Wong <alvin@alvinhc.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "KisRecentFileIconCache.h"

#include <QtConcurrent>
#include <QApplication>
#include <QGlobalStatic>

#include "KisFileIconCreator.h"
#include "KisRecentDocumentsModelWrapper.h"


namespace {
    /*
    * Parameters for fetching a file icon
    */
    struct GetFileIconParameters
    {
        QUrl m_documentUrl;
        QSize m_iconSize;
        qreal m_devicePixelRatioF;
    };

    /**
     * Info for an icon that was fetched (or, at least, we tried to fetch)
     */
    struct IconFetchResult
    {
        bool m_iconWasFetchedOk = false;
        /**
         * Url of the document (file) we're generating an icon for
         */
        QUrl m_documentUrl;
        /**
         * Expensive member to generate
         */
        QIcon m_icon;
    };


    IconFetchResult getFileIcon(GetFileIconParameters gfip)
    {
        KisFileIconCreator iconCreator;
        IconFetchResult iconFetched;
        iconFetched.m_documentUrl = gfip.m_documentUrl;
        iconFetched.m_iconWasFetchedOk = iconCreator.createFileIcon(gfip.m_documentUrl.toLocalFile(),
                                                                    iconFetched.m_icon,
                                                                    gfip.m_devicePixelRatioF,
                                                                    gfip.m_iconSize);
        return iconFetched;
    }
} /* namespace */


struct KisRecentFileIconCache::CacheItem
{
    QUrl url;
    QFuture<IconFetchResult> fetchingFuture;
    QIcon cachedIcon;
};

KisRecentFileIconCache::KisRecentFileIconCache()
{
    // Limit the number of threads used for icon fetching to prevent it from
    // impacting normal usage too much, and to prevent it from consuming too
    // much memory by loading too many large files.
    if (QThread::idealThreadCount() > 2) {
        m_iconFetchThreadPool.setMaxThreadCount(2);
    }
    connect(qApp, SIGNAL(aboutToQuit()), SLOT(cleanupOnQuit()));
}

KisRecentFileIconCache::~KisRecentFileIconCache() {}

Q_GLOBAL_STATIC(KisRecentFileIconCache, s_instance)

KisRecentFileIconCache *KisRecentFileIconCache::instance()
{
    if (QThread::currentThread() != qApp->thread()) {
        qWarning() << "KisRecentFileIconCache::instance() called from non-GUI thread!";
        return nullptr;
    }
    return s_instance;
}

QIcon KisRecentFileIconCache::getOrQueueFileIcon(const QUrl &url)
{
    const QMap<QUrl, CacheItem>::const_iterator findItem = m_iconCacheMap.constFind(url);
    if (findItem != m_iconCacheMap.constEnd()) {
        // If the icon is still being fetched, this returns `QIcon()`.
        return findItem.value().cachedIcon;
    } else {
        if (!url.isLocalFile()) {
            // We don't fetch thumbnails for non-local files.
            return QIcon();
        }
        constexpr QSize iconSize(KisRecentDocumentsModelWrapper::ICON_SIZE_LENGTH, KisRecentDocumentsModelWrapper::ICON_SIZE_LENGTH);
        const GetFileIconParameters param = {
            url, // m_documentUrl
            iconSize, // m_iconSize
            1.0, // m_devicePixelRatioF
        };
        QFuture<IconFetchResult> future = QtConcurrent::run(&m_iconFetchThreadPool, getFileIcon, param);
        auto *watcher = new QFutureWatcher<IconFetchResult>(this);
        watcher->setFuture(future);
        connect(watcher, SIGNAL(finished()), SLOT(iconFetched()));
        connect(watcher, SIGNAL(canceled()), SLOT(futureCanceled()));
        const CacheItem cacheItem = { url, future, QIcon() };
        m_iconCacheMap.insert(url, cacheItem);
        return QIcon();
    }
}

void KisRecentFileIconCache::invalidateFileIcon(const QUrl &url)
{
    QMap<QUrl, CacheItem>::iterator findItem = m_iconCacheMap.find(url);
    if (findItem == m_iconCacheMap.end()) {
        return;
    }
    // Note: Futures returned by `QtConcurrent::run` does not support
    // cancellation, but we try anyway.
    if (!findItem.value().fetchingFuture.isCanceled()) {
        findItem.value().fetchingFuture.cancel();
    }
    m_iconCacheMap.erase(findItem);
}

void KisRecentFileIconCache::reloadFileIcon(const QUrl &url)
{
    invalidateFileIcon(url);
    getOrQueueFileIcon(url);
}

void KisRecentFileIconCache::cleanupOnQuit()
{
    // We need to wait for the icon fetching to finish before letting qApp
    // be deleted, because the icon generation relies on qApp.
    m_iconFetchThreadPool.clear();
    m_iconFetchThreadPool.waitForDone();
}

void KisRecentFileIconCache::iconFetched()
{
    auto *watcher = dynamic_cast<QFutureWatcher<IconFetchResult> *>(QObject::sender());
    if (!watcher) {
        qWarning() << "KisRecentFileIconCache::iconFetched() called but sender is not a QFutureWatcher";
        return;
    }
    QFuture<IconFetchResult> future = watcher->future();
    watcher->deleteLater();
    IconFetchResult result = future.result();
    auto findItem = m_iconCacheMap.find(result.m_documentUrl);
    if (findItem == m_iconCacheMap.end()) {
        qWarning() << "KisRecentFileIconCache item not found!";
        return;
    }
    if (findItem.value().fetchingFuture != future) {
        qWarning() << "KisRecentFileIconCache item has a different QFuture";
        return;
    }
    findItem.value().fetchingFuture = QFuture<IconFetchResult>();
    if (result.m_iconWasFetchedOk) {
        findItem.value().cachedIcon = result.m_icon;
        emit fileIconChanged(result.m_documentUrl, result.m_icon);
    }
}

void KisRecentFileIconCache::futureCanceled()
{
    auto *watcher = dynamic_cast<QFutureWatcher<IconFetchResult> *>(QObject::sender());
    if (!watcher) {
        qWarning() << "KisRecentFileIconCache::futureCanceled() called but sender is not a QFutureWatcher";
        return;
    }
    watcher->deleteLater();
}
