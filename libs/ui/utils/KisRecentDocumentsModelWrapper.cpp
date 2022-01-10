/*
 *  SPDX-FileCopyrightText: 2021 Felipe Lema <felipelema@mortemale.org>
 *  SPDX-FileCopyrightText: 2022 Alvin Wong <alvin@alvinhc.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "KisRecentDocumentsModelWrapper.h"

#include <QTimer>
#include <QUrl>

#include "kis_icon_utils.h"
#include "KisRecentFileIconCache.h"
#include "KisRecentFilesManager.h"


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
}

KisRecentDocumentsModelWrapper::~KisRecentDocumentsModelWrapper() {}

void KisRecentDocumentsModelWrapper::setFiles(const QList<QUrl> &urls, qreal devicePixelRatioF)
{
    // TODO: Use devicePixelRatioF arg?

    // update model
    QList<QStandardItem *> items;
    const QIcon stubIcon = KisIconUtils::loadIcon("media-floppy");
    {
        Q_FOREACH(const QUrl &recentFileUrl, urls){
            const QString recentFileUrlPath = recentFileUrl.toLocalFile();
            QStandardItem *recentItem = new QStandardItem(stubIcon, recentFileUrl.fileName());
            recentItem->setData(recentFileUrl);
            recentItem->setToolTip(recentFileUrlPath);
            items.append(recentItem);
        }
    }
    m_filesAndThumbnailsModel.clear(); // clear existing data before it gets re-populated
    Q_FOREACH(QStandardItem *item, items) {
        m_filesAndThumbnailsModel.appendRow(item);
    }

    int row=0;
    Q_FOREACH(const QUrl &recentFileUrl, urls){
        const QIcon icon = KisRecentFileIconCache::instance()->getOrQueueFileIcon(recentFileUrl);
        if (!icon.isNull()) {
            m_filesAndThumbnailsModel.item(row)->setIcon(icon);
        }
        row++;
    }

    emit sigModelIsUpToDate();
}

void KisRecentDocumentsModelWrapper::slotFileIconChanged(const QUrl &url, const QIcon &icon)
{
    const int count = m_filesAndThumbnailsModel.rowCount();
    for (int i = 0; i < count; i++) {
        QStandardItem *item = m_filesAndThumbnailsModel.item(i);
        if (item->data() == url) {
            item->setIcon(icon);
            return;
        }
    }
}

void KisRecentDocumentsModelWrapper::fileAdded(const QUrl &url)
{
    // TODO: Only insert one row into the model
    listRenewed();
}

void KisRecentDocumentsModelWrapper::fileRemoved(const QUrl &url)
{
    // TODO: Only remove one row from the model
    listRenewed();
}

void KisRecentDocumentsModelWrapper::listRenewed()
{
    // HACK: We need to delay this to the next tick. KRecentFilesAction now
    //       relies on KisRecentFilesManager to be notified about changes
    //       using the same signals that would call this slot, but it also
    //       relies on KisRecentDocumentsModelWrapper::model() change events
    //       (connected by KisMainWindow) to update the file icons. Because
    //       of this, we need to call setFiles *after* KRecentFilesAction has
    //       updated the menu actions in order for the file icons to be
    //       applied.
    QTimer::singleShot(0, this, [this]() {
        setFiles(KisRecentFilesManager::instance()->recentUrlsLatestFirst(), 1.0);
    });
}

QStandardItemModel &KisRecentDocumentsModelWrapper::model()
{
    return m_filesAndThumbnailsModel;
}
