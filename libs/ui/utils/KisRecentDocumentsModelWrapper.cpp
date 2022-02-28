/*
 *  SPDX-FileCopyrightText: 2021 Felipe Lema <felipelema@mortemale.org>
 *  SPDX-FileCopyrightText: 2022 Alvin Wong <alvin@alvinhc.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "KisRecentDocumentsModelWrapper.h"

#include <QApplication>
#include <QDir>
#include <QUrl>

#include "kis_icon_utils.h"
#include "KisRecentFileIconCache.h"
#include "KisRecentFilesManager.h"


/**
 * This class implements lazy-loading of file icons when used by, for example,
 * a `QListView` via a `QStandardItemModel`. It can be used by the welcome
 * screen to load the thumbnail icons of recent files on demand as the user
 * scrolls through the list, thus avoiding the need to preload all icons
 * when the user may not even look at the list.
 */
class KisRecentDocumentsModelItem : public QStandardItem
{
    QUrl m_url;
    mutable bool m_iconFetched {false};
    mutable QIcon m_fileIcon;
    QString m_tooltip;

public:
    explicit KisRecentDocumentsModelItem(const QUrl &url);
    ~KisRecentDocumentsModelItem() override;

    QVariant data(int role = Qt::UserRole + 1) const override;
    void setData(const QVariant &value, int role = Qt::UserRole + 1) override;
};

static QString urlToTooltip(const QUrl &url)
{
#ifdef Q_OS_WIN
    // Convert forward slashes to backslashes
    if (url.isLocalFile()) {
        return QDir::toNativeSeparators(url.toLocalFile());
    }
#elif defined(Q_OS_ANDROID)
    return url.toLocalFile();
#endif
    return url.toDisplayString(QUrl::PreferLocalFile);
}

KisRecentDocumentsModelItem::KisRecentDocumentsModelItem(const QUrl &url)
    : QStandardItem(url.fileName())
    , m_url(url)
    , m_tooltip(urlToTooltip(url))
{
}

KisRecentDocumentsModelItem::~KisRecentDocumentsModelItem() {}

QVariant KisRecentDocumentsModelItem::data(int role) const
{
    switch (role) {
    case Qt::DecorationRole:
        if (!m_iconFetched) {
            m_iconFetched = true;
            // This lazy-loads the icon if not already cached. Once the real
            // icon has been loaded, `KisRecentDocumentsModelItem` will be
            // notified of the icon change and sets it to this item.
            const QIcon icon = KisRecentFileIconCache::instance()->getOrQueueFileIcon(m_url);
            if (!icon.isNull()) {
                m_fileIcon = icon;
            }
        }
        if (m_fileIcon.isNull()) {
            return KisIconUtils::loadIcon("media-floppy");
        } else {
            return m_fileIcon;
        }
    case Qt::ToolTipRole:
        return m_tooltip;
    case Qt::UserRole + 1:
        return m_url;
    }
    return QStandardItem::data(role);
}

void KisRecentDocumentsModelItem::setData(const QVariant &value, int role)
{
    switch (role) {
    case Qt::DecorationRole:
        if (value.type() == (QVariant::Type)QMetaType::QIcon) {
            // `KisRecentDocumentsModelItem` calls `setIcon` to update the
            // file icon once it has been lazy-loaded or changed.
            m_iconFetched = true;
            m_fileIcon = value.value<QIcon>();
            emitDataChanged();
        }
        return;
    case Qt::ToolTipRole:
        qWarning() << "KisRecentDocumentsModelItem::setTooltip ignored";
        return;
    case Qt::UserRole + 1:
        qWarning() << "KisRecentDocumentsModelItem::setData ignored";
        return;
    }
    QStandardItem::setData(value, role);
}


KisRecentDocumentsModelWrapper::KisRecentDocumentsModelWrapper()
{
    connect(KisRecentFileIconCache::instance(),
            SIGNAL(fileIconChanged(const QUrl &, const QIcon &)),
            SLOT(slotFileIconChanged(const QUrl &, const QIcon &)));
    connect(KisRecentFilesManager::instance(),
            SIGNAL(fileAdded(const QUrl &)),
            SLOT(fileAdded(const QUrl &)));
    connect(KisRecentFilesManager::instance(),
            SIGNAL(fileRemoved(const QUrl &)),
            SLOT(fileRemoved(const QUrl &)));
    connect(KisRecentFilesManager::instance(),
            SIGNAL(listRenewed()),
            SLOT(listRenewed()));

    // Load the initial recent files list
    listRenewed();
}

KisRecentDocumentsModelWrapper::~KisRecentDocumentsModelWrapper() {}

KisRecentDocumentsModelWrapper *KisRecentDocumentsModelWrapper::instance()
{
    if (QThread::currentThread() != qApp->thread()) {
        qWarning() << "KisRecentDocumentsModelWrapper::instance() called from non-GUI thread!";
        return nullptr;
    }
    static KisRecentDocumentsModelWrapper s_instance;
    return &s_instance;
}

void KisRecentDocumentsModelWrapper::setFiles(const QList<QUrl> &urls)
{
    // Replace all items in the model. The existing items are deleted by
    // `QStandardItemModel`.
    m_filesAndThumbnailsModel.setRowCount(urls.count());
    for (int i = 0; i < urls.count(); i++) {
        QStandardItem *item = new KisRecentDocumentsModelItem(urls[i]);
        m_filesAndThumbnailsModel.setItem(i, item);
    }

    emit sigModelIsUpToDate();
}

void KisRecentDocumentsModelWrapper::slotFileIconChanged(const QUrl &url, const QIcon &icon)
{
    const int count = m_filesAndThumbnailsModel.rowCount();
    for (int i = 0; i < count; i++) {
        QStandardItem *item = m_filesAndThumbnailsModel.item(i);
        if (item && item->data() == url) {
            item->setIcon(icon);
            return;
        }
    }
}

void KisRecentDocumentsModelWrapper::fileAdded(const QUrl &url)
{
    m_filesAndThumbnailsModel.insertRow(0, new KisRecentDocumentsModelItem(url));
    emit sigModelIsUpToDate();
}

void KisRecentDocumentsModelWrapper::fileRemoved(const QUrl &url)
{
    const int count = m_filesAndThumbnailsModel.rowCount();
    for (int i = 0; i < count; i++) {
        QStandardItem *item = m_filesAndThumbnailsModel.item(i);
        if (item && item->data() == url) {
            m_filesAndThumbnailsModel.removeRow(i);
            emit sigModelIsUpToDate();
            return;
        }
    }
}

void KisRecentDocumentsModelWrapper::listRenewed()
{
    setFiles(KisRecentFilesManager::instance()->recentUrlsLatestFirst());
}

QStandardItemModel &KisRecentDocumentsModelWrapper::model()
{
    return m_filesAndThumbnailsModel;
}
