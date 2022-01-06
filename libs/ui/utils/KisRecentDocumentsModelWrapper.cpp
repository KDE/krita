/*
 *  SPDX-FileCopyrightText: 2021 Felipe Lema <felipelema@mortemale.org>
 *  SPDX-FileCopyrightText: 2022 Alvin Wong <alvin@alvinhc.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "KisRecentDocumentsModelWrapper.h"

#include <QUrl>

#include "kis_icon_utils.h"
#include "KisRecentFileIconCache.h"


KisRecentDocumentsModelWrapper::KisRecentDocumentsModelWrapper()
{
    connect(KisRecentFileIconCache::instance(),
            SIGNAL(fileIconChanged(const QUrl &, const QIcon &)),
            SLOT(slotFileIconChanged(const QUrl &, const QIcon &)));
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

QStandardItemModel &KisRecentDocumentsModelWrapper::model()
{
    return m_filesAndThumbnailsModel;
}
