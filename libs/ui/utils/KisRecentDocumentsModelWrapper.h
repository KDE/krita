/*
 *  SPDX-FileCopyrightText: 2021 Felipe Lema <felipelema@mortemale.org>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KISRECENTDOCUMENTSMODELWRAPPER_H
#define KISRECENTDOCUMENTSMODELWRAPPER_H

#include <QtCore>
#include <QMap>
#include <QFutureWatcher>
#include <QStandardItemModel>

class QString;
class QIcon;

/**
 * Fills model with files, launches thumbnail in background
 * While icons are being fetched in the background, items without such an icon will be
 * marked as "not enabled". When all background fetching is done, non-enabled items will
 * be removed from the model
 */
class KisRecentDocumentsModelWrapper : public QObject
{
    Q_OBJECT
public:
    KisRecentDocumentsModelWrapper();
    ~KisRecentDocumentsModelWrapper();
    using URLs=QList<QUrl>;
    /**
     * Update m_filesAndThumbnailsModel and launch worker thread to fetch icons in background
     */
    void setFiles(const URLs &urls, QSize iconSize, qreal devicePixelRatioF);
    /**
     * Get underlying model
     *
     * No need for any extra setup. This will hold the provided files
     */
    QStandardItemModel &model();
    /**
     * Info for an icon that was fetched (or, at least, we tried to fetch)
     * We map a document url (and other info) int
     * \see GetFileIconParameters
     */
    struct IconFetchResult
    {
        int m_workerId = 0;
        int m_row = 0;
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

private:
    /**
     * Cache that holds an icon per filepath
     */
    QMap<QString, QIcon> m_filePathToIconCache;
    QStandardItemModel m_filesAndThumbnailsModel;
    /**
     * Id to match an incoming icon with "current" file paths
     *
     * Since thumbnail icon is processed in a separate thread, it could happen that we receive
     * an icon thot corresponded to the previous batch of file-paths (previous thread was interrupted)
     */
    int m_currentWorkerId = 0;
    using FutureIconWatcher=QFutureWatcher<IconFetchResult>;
    FutureIconWatcher m_iconWorkerWatcher;
    void cancelAndWaitIfRunning();

private Q_SLOTS:
    /**
     * Receive an icon from working thread and update m_filesAndThumbnailsModel
     * If not getting an actual icon, set row to "invalid"
     */
    void slotIconReady(int row);
    /**
     * Cleanup after separate threads finished fetching
     */
    void slotIconFetchingFinished();
Q_SIGNALS:
    /**
     * Report an document we couldn't generate an icon for
     */
    void sigInvalidDocumentForIcon(QUrl documentUrl);
    /**
     * Report when the background icon thread is done and no more changes will apply for the model
     */
    void sigModelIsUpToDate();
}; // class KisRecentDocumentsModelWrapper

#endif // KISRECENTDOCUMENTSMODELWRAPPER_H
