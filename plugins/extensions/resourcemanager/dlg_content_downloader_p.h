/*
 *  Copyright (c) 2017 Aniketh Girish anikethgireesh@gmail.com
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#ifndef DLG_CONTENT_DOWNLOADER_P_H
#define DLG_CONTENT_DOWNLOADER_P_H

#include <QSortFilterProxyModel>
#include <QtCore/QStringList>
#include <QtCore/QTimer>
#include <QScrollBar>
#include <QListView>

#include <KNSCore/ItemsModel>
#include <KNSCore/EntryInternal>

#include "entrydetailsdialog_p.h"
#include "itemsviewbasedelegate_p.h"

#include "ui_wdgdlgcontentdownloader.h"

namespace Ui {
class WdgDlgContentDownloader;
}

class DlgContentDownloader;

class DlgContentDownloaderPrivate : public QObject
{
    Q_OBJECT
public:
    DlgContentDownloader *q;
    EntryDetails *details;

    // The engine that does all the work
    KNSCore::Engine *engine;
    Ui::WdgDlgContentDownloader ui;
    // Model to show the entries
    KNSCore::ItemsModel *model;
    // Timeout for messge display
    QTimer *messageTimer;

    ItemsViewBaseDelegate *delegate;

    QString searchTerm;
    QSet<KNSCore::EntryInternal> changedEntries;

    QSet<QString> categories;
    QSet<QString> providers;

    QString titleText;
    QString m_knsrc;
    bool dialogMode;

    explicit DlgContentDownloaderPrivate(DlgContentDownloader *q);
    ~DlgContentDownloaderPrivate();

    void init(const QString &knsrc);
    void slotShowMessage(const QString& msg);
    void displayMessage(const QString &msg, KTitleWidget::MessageType type, int timeOutMs = 0);

    void slotProvidersLoaded();
    void slotEntriesLoaded(const KNSCore::EntryInternal::List &entries);
    void slotEntryChanged(const KNSCore::EntryInternal &entry);

    void slotShowDetails(const KNSCore::EntryInternal &entry);
    void slotShowOverview();

    void slotPayloadFailed(const KNSCore::EntryInternal &entry);
    void slotPayloadLoaded(QUrl url);

    void slotResetMessage();
    void slotNetworkTimeout();
    void sortingChanged();
    void slotSearchTextChanged();
    void slotUpdateSearch();
    void slotCategoryChanged(int);

    void slotListViewListMode();
    void slotListViewIconMode();
    void setListViewMode(QListView::ViewMode mode);

};

class EntryPrivate : public QSharedData
{
public:
    KNSCore::EntryInternal e;
    static KNS3::Entry fromInternal(const KNSCore::EntryInternal* internal)
    {
        KNS3::Entry e;
        e.d->e = *internal;
        return e;
    }
};

#endif // DLG_CONTENT_DOWNLOADER_P_H
