/*
 *  Copyright (C) 2005 by Enrico Ros <eros.kde@email.it>
 *  Copyright (C) 2005 - 2007 Josef Spillner <spillner@kde.org>
 *  Copyright (C) 2007 Dirk Mueller <mueller@kde.org>
 *  Copyright (C) 2007-2009 Jeremy Whiting <jpwhiting@kde.org>
 *  Copyright (C) 2009-2010 Frederik Gladhorn <gladhorn@kde.org>
 *  Copyright (c) 2017 Aniketh Girish <anikethgireesh@gmail.com>
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

#include "content_dowloader_dialog.h"

#include <QCoreApplication>
#include <QtCore/QTimer>
#include <QSortFilterProxyModel>
#include <QScrollBar>
#include <QKeyEvent>

#include <ksharedconfig.h>
#include <ktitlewidget.h>
#include <kwindowconfig.h>
#include <kstandardguiitem.h>
#include <klocalizedstring.h>
#include <kauthorized.h>
#include <kmessagebox.h>

#include "dlg_content_downloader.h"
#include "dlg_content_downloader_p.h"
#include "widgetquestionlistener.h"

using namespace KNSCore;

class ContentDownloaderDialogPrivate
{
public:

    DlgContentDownloader *downloadWidget;

    ~ContentDownloaderDialogPrivate()
    {
        delete downloadWidget;
    }
};

const char ConfigGroup[] = "Content Downloader Dialog Settings";

ContentDownloaderDialog::ContentDownloaderDialog(QWidget *parent)
    : QDialog(parent)
    , d(new ContentDownloaderDialogPrivate)
{
    const QString name = QCoreApplication::applicationName();
    init(name + ".knsrc");
}

ContentDownloaderDialog::ContentDownloaderDialog(const QString &configFile, QWidget *parent)
    : QDialog(parent)
    , d(new ContentDownloaderDialogPrivate)
{
    init(configFile);
}

void ContentDownloaderDialog::init(const QString &configFile)
{
    // load the last size from config
    KConfigGroup group(KSharedConfig::openConfig(), ConfigGroup);
    KWindowConfig::restoreWindowSize(windowHandle(), group);
    setMinimumSize(700, 400);

    setWindowTitle(i18n("Krita's content downloader"));

    QVBoxLayout *layout = new QVBoxLayout;
    d->downloadWidget = new DlgContentDownloader(configFile, this);
    layout->addWidget(d->downloadWidget);
    setLayout(layout);

    if (group.hasKey("Name")) {
        d->downloadWidget->setTitle(group.readEntry("Name"));
    } else {
        QString displayName = QGuiApplication::applicationDisplayName();
        if (displayName.isEmpty()) {
            displayName = QCoreApplication::applicationName();
        }
        d->downloadWidget->setTitle(i18nc("Program name followed by 'Add On Installer'",
            "%1 Add-On Installer", displayName));
    }
    //d->downloadWidget->d->ui.m_titleWidget->setPixmap(QIcon::fromTheme(KGlobal::activeComponent().aboutData()->programIconName()));
    d->downloadWidget->d->ui.m_titleWidget->setVisible(true);
    d->downloadWidget->d->ui.closeButton->setVisible(true);
    KStandardGuiItem::assign(d->downloadWidget->d->ui.closeButton, KStandardGuiItem::Close);
    d->downloadWidget->d->dialogMode = true;
    connect(d->downloadWidget->d->ui.closeButton, &QAbstractButton::clicked, this, &QDialog::accept);
    WidgetQuestionListener::instance();
}

ContentDownloaderDialog::~ContentDownloaderDialog()
{
    KConfigGroup group(KSharedConfig::openConfig(), ConfigGroup);
    KWindowConfig::saveWindowSize(windowHandle(), group, KConfigBase::Persistent);
    delete d;
}

int ContentDownloaderDialog::exec()
{
    if (!KAuthorized::authorize("KF5NewStuffCore")) {
        KMessageBox::information(this, "Get Hot New Stuff is disabled by the administrator", "Get Hot New Stuff disabled");
        return QDialog::Rejected;
    }
    return QDialog::exec();
}

void ContentDownloaderDialog::open()
{
    if (!KAuthorized::authorize("KF5NewStuffCore")) {
        KMessageBox::information(this, "Get Hot New Stuff is disabled by the administrator", "Get Hot New Stuff disabled");
        return;
    }
    QDialog::open();
}

void ContentDownloaderDialog::showEvent(QShowEvent *event)
{
    if (!KAuthorized::authorize("KF5NewStuffCore")) {
        KMessageBox::information(this, "Get Hot New Stuff is disabled by the administrator", "Get Hot New Stuff disabled");
        return;
    }
    QWidget::showEvent(event);
}

void ContentDownloaderDialog::setTitle(const QString &title)
{
    d->downloadWidget->setTitle(title);
}

QString ContentDownloaderDialog::title() const
{
    return d->downloadWidget->title();
}

KNSCore::Engine *ContentDownloaderDialog::engine()
{
    return d->downloadWidget->engine();
}

EntryInternal::List ContentDownloaderDialog::changedEntries()
{
    return d->downloadWidget->changedEntries();
}

EntryInternal::List ContentDownloaderDialog::installedEntries()
{
    return d->downloadWidget->installedEntries();
}
