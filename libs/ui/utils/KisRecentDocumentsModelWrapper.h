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
 * This singleton class provides a `QStandardItemModel` representing the
 * recent files list and also supports lazy-loading of the file thumbnail
 * icons. Each recent file entry is represented as a QStandardItem. When
 * `QStandardItem::icon()` is called for the first time, it fetches the icon
 * via `KisRecentFileIconCache`, which triggers loading the file icon in
 * background if it hasn't already been loaded and cached.
 *
 * See also `KisRecentFilesManager`.
 */
class KisRecentDocumentsModelWrapper : public QObject
{
    Q_OBJECT
public:
    static constexpr const int ICON_SIZE_LENGTH = 48;

    static KisRecentDocumentsModelWrapper *instance();

private:
    KisRecentDocumentsModelWrapper();
    ~KisRecentDocumentsModelWrapper();

    /**
     * Update m_filesAndThumbnailsModel and launch worker thread to fetch icons in background
     */
    void setFiles(const QList<QUrl> &urls);

public:
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
    void fileAdded(const QUrl &url);
    void fileRemoved(const QUrl &url);
    void listRenewed();

Q_SIGNALS:
    /**
     * Report when the model has been updated with the latest recent file list.
     */
    void sigModelIsUpToDate();
}; // class KisRecentDocumentsModelWrapper

#endif // KISRECENTDOCUMENTSMODELWRAPPER_H
