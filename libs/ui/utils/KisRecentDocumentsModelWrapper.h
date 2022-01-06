/*
 *  SPDX-FileCopyrightText: 2021 Felipe Lema <felipelema@mortemale.org>
 *  SPDX-FileCopyrightText: 2022 Alvin Wong <alvin@alvinhc.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KISRECENTDOCUMENTSMODELWRAPPER_H
#define KISRECENTDOCUMENTSMODELWRAPPER_H

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
    static constexpr const int ICON_SIZE_LENGTH = 48;

    KisRecentDocumentsModelWrapper();
    ~KisRecentDocumentsModelWrapper();

    /**
     * Update m_filesAndThumbnailsModel and launch worker thread to fetch icons in background
     */
    void setFiles(const QList<QUrl> &urls, qreal devicePixelRatioF);

    /**
     * Get underlying model
     *
     * No need for any extra setup. This will hold the provided files
     */
    QStandardItemModel &model();

private:
    QStandardItemModel m_filesAndThumbnailsModel;

private Q_SLOTS:
    void slotFileIconChanged(const QUrl &url, const QIcon &icon);

Q_SIGNALS:
    /**
     * Report when the model has been updated with the latest recent file list.
     */
    void sigModelIsUpToDate();
}; // class KisRecentDocumentsModelWrapper

#endif // KISRECENTDOCUMENTSMODELWRAPPER_H
