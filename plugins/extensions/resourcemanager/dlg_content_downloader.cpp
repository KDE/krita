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

#include "dlg_content_downloader.h"
#include "dlg_content_downloader_p.h"

#include "itemsviewbasedelegate_p.h"
#include "itemsviewdelegate_p.h"
#include "itemsgridviewdelegate_p.h"

#include <QtCore/QTimer>
#include <QScrollBar>
#include <QKeyEvent>
#include <QCoreApplication>

#include <kmessagebox.h>
#include <klocalizedstring.h>

#include <KNSCore/ItemsModel>

using namespace KNS3;

DlgContentDownloader::DlgContentDownloader(QWidget *parent)
    : QWidget(parent)
    , d(new DlgContentDownloaderPrivate(this))
{
    const Qstring name = QCoreApplication::applicationName();
    init(name + ".knsrc")
}

DlgContentDownloader::DlgContentDownloader(const QString &knsrc, QWidget *parent)
    : QWidget(parent)
    , d(new DlgContentDownloaderPrivate(this))
{
    init(knsrc);

}

void DlgContentDownloader::init(const QString &knsrc)
{
    d->init(knsrc);
}

DlgContentDownloader::~DlgContentDownloader()
{
    delete d;
}

void DlgContentDownloader::setTitle(const Qstring &title)
{
    d->ui.m_titleWidget->setText(title);
}

void DlgContentDownloader::title() const
{
    return d->ui.m_titleWidget->text();
}

KNSCore::Engine *DlgContentDownloader::engine()
{
    return d->engine;
}

Entry::List DlgContentDownloader::changedEntries()
{
    Entry::List entries;
    foreach (const KNSCore::EntryInternal &e, d->changedEntries) {
        entries.append(EntryPrivate::fromInternal(&e));
    }
    return entries;
}

Entry::List DlgContentDownloader::installedEntries()
{
    Entry::List entries;
    foreach (const KNSCore::EntryInternal &e, d->changedEntries) {
        if (e.status() == Entry::Installed) {
            entries.append(EntryPrivate::fromInternal(&e));
        }
    }
    return entries;
}

DlgContentDownloaderPrivate::DlgContentDownloaderPrivate(DlgContentDownloader *q)
    : q(q)
    , engine(new KNSCore::Engine)
    , model(new KNSCore::ItemsModel(engine))
    , messageTimer(nullptr)
    , dialogMode(false)
{
}

DlgContentDownloaderPrivate::~DlgContentDownloaderPrivate()
{
    delete messageTimer;
    delete delegate;
    delete model;
    delete engine;
}

void DlgContentDownloaderPrivate::slotResetMessage()
{
    ui.m_titleWidget->setComment(QString());
}

void DlgContentDownloaderPrivate::slotNetworkTimeout()
{
    displayMessage(i18n("Timeout. Please Check your Internet connection."), KTitleWidget::ErrrorMessage);
}

void DlgContentDownloaderPrivate::sortingChanged()
{
    KNSCore::Provider::SortMode sortMode = KNSCore::Provider::Newest;
    KNSCore::Provider::Filter filter = KNSCore::Provider::None;
    if (ui.ratingRadio->isChecked()) {
        sortMode = KNSCore::Provider::Rating;
    } else if (ui.mostDownloadsRadio->isChecked()) {
        sortMode = KNSCore::Provider::Downloads;
    } else if (ui.installedRadio->isChecked()) {
        filter = KNSCore::Provider::Installed;
    }

    model->clearEntries();
    if (filter == KNSCore::Provider::Installed) {
        ui.m_searchEdit->clear();
    }
    ui.m_searchEdit->setEnabled(filter != KNSCore::Provider::Installed);

    engine->setSortMode(sortMode);
    engine->setFilter(filter);
}

void DlgContentDownloaderPrivate::slotUpdateSearch()
{
    if (searchTerm == ui.m_searchEdit->text().trimmed()) {
        return;
    }
    searchTerm = ui.m_searchEdit->text().trimmed();
}

void DlgContentDownloaderPrivate::slotSearchTextChanged()
{
    if (searchTerm == ui.m_searchEdit->text().trimmed()) {
        return;
    }
    searchTerm = ui.m_searchEdit->text().trimmed();
    engine->setSearchTerm(ui.m_searchEdit->text().trimmed());
}

void DlgContentDownloaderPrivate::slotCategoryChanged(int idx)
{
    if (idx == 0) {
        // All Categories item selected, reset filter
        engine->setCategoriesFilter(QStringList());
    }

    else {
        QString category = ui.m_categoryCombo->currentData().toString();

        if (!category.isEmpty()) {
            QStringList filter(category);
            engine->setCategoriesFilter(filter);
        }
    }
}

void DlgContentDownloaderPrivate::slotEntryChanged(const KNSCore::EntryInternal &entry)
{
    changedEntries.insert(entry);
    model->slotEntryChanged(entry);
}

void DlgContentDownloaderPrivate::slotPayloadFailed(const KNSCore::EntryInternal &entry)
{
    KMessageBox::error(nullptr, i18n("Could not install %1", entry.name()),
                       i18n("Get Hot New Stuff!"));
}

void DlgContentDownloaderPrivate::slotPayloadLoaded(QUrl url)
{
    Q_UNUSED(url)
}

void DlgContentDownloaderPrivate::init(const QString &configFile)
{
    m_configFile = configFile;
    ui.setupUi(q);
    ui.m_titleWidget->setVisible(false);
    ui.closeButton->setVisible(dialogMode);
    ui.backButton->setVisible(false);
    KStandardGuiItem::assign(ui.backButton, KStandardGuiItem::Back);
    q->connect(ui.backButton, &QPushButton::clicked, this, &DlgContentDownloaderPrivate::slotShowOverview);

    q->connect(engine, &KNSCore::Engine::signalMessage, this, &DlgContentDownloaderPrivate::slotShowMessage);

    q->connect(engine, &KNSCore::Engine::signalProvidersLoaded, this, &DlgContentDownloaderPrivate::slotProvidersLoaded);
    // Entries have been fetched and should be shown:
    q->connect(engine, &KNSCore::Engine::signalEntriesLoaded, this, &DlgContentDownloaderPrivate::slotEntriesLoaded);

    // An entry has changes - eg because it was installed
    q->connect(engine, &KNSCore::Engine::signalEntryChanged, this, &DlgContentDownloaderPrivate::slotEntryChanged);

    q->connect(engine, &KNSCore::Engine::signalResetView, model, &KNSCore::ItemsModel::clearEntries);
    q->connect(engine, &KNSCore::Engine::signalEntryPreviewLoaded,
               model, &KNSCore::ItemsModel::slotEntryPreviewLoaded);

    engine->init(configFile);

    delegate = new ItemsViewDelegate(ui.m_listView, engine, q);
    ui.m_listView->setItemDelegate(delegate);
    ui.m_listView->setModel(model);

    ui.iconViewButton->setIcon(QIcon::fromTheme(QStringLiteral("view-list-icons")));
    ui.iconViewButton->setToolTip(i18n("Icons view mode"));
    ui.listViewButton->setIcon(QIcon::fromTheme(QStringLiteral("view-list-details")));
    ui.listViewButton->setToolTip(i18n("Details view mode"));

    q->connect(ui.listViewButton, &QPushButton::clicked, this, &DlgContentDownloaderPrivate::slotListViewListMode);
    q->connect(ui.iconViewButton, &QPushButton::clicked, this, &DlgContentDownloaderPrivate::slotListViewIconMode);

    q->connect(ui.newestRadio,        &QRadioButton::clicked, this, &DlgContentDownloaderPrivate::sortingChanged);
    q->connect(ui.ratingRadio,        &QRadioButton::clicked, this, &DlgContentDownloaderPrivate::sortingChanged);
    q->connect(ui.mostDownloadsRadio, &QRadioButton::clicked, this, &DlgContentDownloaderPrivate::sortingChanged);
    q->connect(ui.installedRadio,     &QRadioButton::clicked, this, &DlgContentDownloaderPrivate::sortingChanged);

    q->connect(ui.m_searchEdit, &KLineEdit::textChanged,     this, &DlgContentDownloaderPrivate::slotSearchTextChanged);
    q->connect(ui.m_searchEdit, &KLineEdit::editingFinished, this, &DlgContentDownloaderPrivate::slotUpdateSearch);

    QStringList categories = engine->categories();
    if (categories.size() < 2) {
        ui.m_categoryLabel->setVisible(false);
        ui.m_categoryCombo->setVisible(false);
    } else {
        ui.m_categoryCombo->addItem(i18n("All Categories"));
        //NOTE: categories will be populated when we will get metadata from the server
    }

    connect(engine, &KNSCore::Engine::signalCategoriesMetadataLoded,
             this, [this](const QList<KNSCore::Provider::CategoryMetadata> &categories) {
                for (auto data : categories) {
                    if (!data.displayName.isEmpty()) {
                        ui.m_categoryCombo->addItem(data.displayName, data.name);
                    } else {
                        ui.m_categoryCombo->addItem(data.name, data.name);
                    }
                }
            });
    ui.detailsStack->widget(0)->layout()->setMargin(0);
    ui.detailsStack->widget(1)->layout()->setMargin(0);

    q->connect(ui.m_categoryCombo, static_cast<void(KComboBox::*)(int)>(&KComboBox::activated),
               this, &DlgContentDownloaderPrivate::slotCategoryChanged);

    // let the search line edit trap the enter key, otherwise it closes the dialog
    ui.m_searchEdit->setTrapReturnKey(true);

    q->connect(ui.m_listView->verticalScrollBar(), &QScrollBar::valueChanged, this, &DlgContentDownloaderPrivate::scrollbarValueChanged);
    q->connect(ui.m_listView, SIGNAL(doubleClicked(QModelIndex)), delegate, SLOT(slotDetailsClicked(QModelIndex)));

    details = new EntryDetails(engine, &ui);
    q->connect(delegate, &KNS3::ItemsViewBaseDelegate::signalShowDetails, this, &DlgContentDownloaderPrivate::slotShowDetails);

    slotShowOverview();
}

void DlgContentDownloaderPrivate::slotListViewListMode()
{
    ui.listViewButton->setChecked(true);
    ui.iconViewButton->setChecked(false);
    setListViewMode(QListView::ListMode);
}

void DlgContentDownloaderPrivate::slotListViewIconMode()
{
    ui.listViewButton->setChecked(false);
    ui.iconViewButton->setChecked(true);
    setListViewMode(QListView::IconMode);
}

void DlgContentDownloaderPrivate::setListViewMode(QListView::ViewMode mode)
{
    if (ui.m_listView->viewMode() == mode) {
        return;
    }

    ItemsViewBaseDelegate *oldDelegate = delegate;
    if (mode == QListView::ListMode) {
        delegate = new ItemsViewDelegate(ui.m_listView, engine, q);
        ui.m_listView->setViewMode(QListView::ListMode);
        ui.m_listView->setResizeMode(QListView::Fixed);
    } else {
        delegate = new ItemsGridViewDelegate(ui.m_listView, engine, q);
        ui.m_listView->setViewMode(QListView::IconMode);
        ui.m_listView->setResizeMode(QListView::Adjust);
    }
    ui.m_listView->setItemDelegate(delegate);
    delete oldDelegate;
    q->connect(ui.m_listView, SIGNAL(doubleClicked(QModelIndex)), delegate, SLOT(slotDetailsClicked(QModelIndex)));
    q->connect(delegate, &ItemsViewBaseDelegate::signalShowDetails, this, &DlgContentDownloaderPrivate::slotShowDetails);
}

void DlgContentDownloaderPrivate::slotProvidersLoaded()
{
    engine->reloadEntries();
}

void DlgContentDownloaderPrivate::slotEntriesLoaded(const KNSCore::EntryInternal::List &entries)
{
    foreach (const KNSCore::EntryInternal &entry, entries) {
        if (!categories.contains(entry.category())) {
            categories.insert(entry.category());
        }
    }
    model->slotEntriesLoaded(entries);
}

void DlgContentDownloaderPrivate::slotShowMessage(const QString& msg)
{
    displayMessage(msg, KTitleWidget::InfoMessage);
}

void DlgContentDownloaderPrivate::displayMessage(const QString &msg, KTitleWidget::MessageType type, int timeOutMs)
{
    if (!messageTimer) {
        messageTimer = new QTimer;
        messageTimer->setSingleShot(true);
        q->connect(messageTimer, &QTimer::timeout, this, &DlgContentDownloaderPrivate::slotResetMessage);
    }
    // stop the pending timer if present
    messageTimer->stop();

    // set text to messageLabel
    ui.m_titleWidget->setComment(msg, type);

    // single shot the resetColors timer (and create it if null)
    if (timeOutMs > 0) {
        messageTimer->start(timeOutMs);
    }
}

void DlgContentDownloaderPrivate::slotShowDetails(const KNSCore::EntryInternal &entry)
{
    if (!entry.isValid()) {
        return;
    }
    titleText = ui.m_titleWidget->text();

    ui.backButton->setVisible(true);
    ui.detailsStack->setCurrentIndex(1);
    ui.descriptionScrollArea->verticalScrollBar()->setValue(0);
    ui.preview1->setImage(QImage());
    ui.preview2->setImage(QImage());
    ui.preview3->setImage(QImage());
    ui.previewBig->setImage(QImage());
    details->setEntry(entry);
}

void DlgContentDownloaderPrivate::slotShowOverview()
{
    ui.backButton->setVisible(false);

    ui.updateButton->setVisible(false);
    ui.installButton->setVisible(false);
    ui.becomeFanButton->setVisible(false);
    ui.uninstallButton->setVisible(false);

    ui.detailsStack->setCurrentIndex(0);
    ui.m_titleWidget->setText(titleText);
}


#include "moc_dlg_content_downloader.cpp"
