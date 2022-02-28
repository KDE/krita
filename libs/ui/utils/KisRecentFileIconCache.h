/*
 *  SPDX-FileCopyrightText: 2022 Alvin Wong <alvin@alvinhc.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KIS_RECENT_FILE_ICON_CACHE_H
#define KIS_RECENT_FILE_ICON_CACHE_H

#include <QFuture>
#include <QIcon>
#include <QMap>
#include <QThreadPool>
#include <QUrl>

class KisRecentFileIconCache : public QObject
{
    Q_OBJECT

    struct CacheItem;

    QMap<QUrl, CacheItem> m_iconCacheMap;
    QThreadPool m_iconFetchThreadPool;

public:
    /**
     * DO NOT USE! Use `instance()` instead. This constructor is public only
     * because it is needed by Q_GLOBAL_STATIC.
     */
    KisRecentFileIconCache();
    ~KisRecentFileIconCache();

public:
    static KisRecentFileIconCache *instance();

    /**
     * Get a cached icon or queue fetching of the icon.
     *
     * If the icon is cached and available, the cached icon is returned.
     * Otherwise, a null default-constructed `QIcon` will be returned, and
     * the fetching of the icon *may* be queued in a background thread.
     */
    QIcon getOrQueueFileIcon(const QUrl &url);

    /**
     * Invalidate (remove) a cached file icon. If the file icon is still in
     * the process of being loaded, its result will be discarded.
     */
    void invalidateFileIcon(const QUrl &url);

    /**
     * Invalidate a cached file icon and trigger a reload of it.
     */
    void reloadFileIcon(const QUrl &url);

private Q_SLOTS:
    void cleanupOnQuit();
    void iconFetched();
    void futureCanceled();

Q_SIGNALS:
    void fileIconChanged(const QUrl &url, const QIcon &icon);
};

#endif /* KIS_RECENT_FILE_ICON_CACHE_H */
