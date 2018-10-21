/* This file is part of the KDE project
 * Copyright (C) 2018 Scott Petrovic <scottpetrovic@gmail.com>
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

#include "KisWelcomePageWidget.h"
#include <QDebug>
#include <QDesktopServices>
#include "kis_action_manager.h"
#include "kactioncollection.h"
#include "kis_action.h"

#include "KConfigGroup"
#include "KSharedConfig"

#include <QListWidget>
#include <QListWidgetItem>
#include "kis_icon_utils.h"
#include "krita_utils.h"
#include "KoStore.h"
#include "kis_config.h"

KisWelcomePageWidget::KisWelcomePageWidget(QWidget *parent)
    : QWidget(parent)
{
    setupUi(this);

    recentDocumentsListView->setDragEnabled(false);
    recentDocumentsListView->viewport()->setAutoFillBackground(false);
    recentDocumentsListView->setSpacing(2);

    // set up URLs that go to web browser
    manualLink->setText(QString("<a href=\"https://docs.krita.org/\">").append(i18n("User Manual")).append("</a>"));
    manualLink->setTextFormat(Qt::RichText);
    manualLink->setTextInteractionFlags(Qt::TextBrowserInteraction);
    manualLink->setOpenExternalLinks(true);

    gettingStartedLink->setText(QString("<a href=\"https://docs.krita.org/en/user_manual/getting_started.html\">").append(i18n("Getting Started")).append("</a>"));
    gettingStartedLink->setTextFormat(Qt::RichText);
    gettingStartedLink->setTextInteractionFlags(Qt::TextBrowserInteraction);
    gettingStartedLink->setOpenExternalLinks(true);

    supportKritaLink->setText(QString("<a href=\"https://krita.org/en/support-us/donations/\">").append(i18n("Support Krita")).append("</a>"));
    supportKritaLink->setTextFormat(Qt::RichText);
    supportKritaLink->setTextInteractionFlags(Qt::TextBrowserInteraction);
    supportKritaLink->setOpenExternalLinks(true);

    userCommunityLink->setText(QString("<a href=\"https://forum.kde.org/viewforum.php?f=136\">").append(i18n("User Community")).append("</a>"));
    userCommunityLink->setTextFormat(Qt::RichText);
    userCommunityLink->setTextInteractionFlags(Qt::TextBrowserInteraction);
    userCommunityLink->setOpenExternalLinks(true);

    kritaWebsiteLink->setText(QString("<a href=\"https://www.krita.org\">").append(i18n("Krita Website")).append("</a>"));
    kritaWebsiteLink->setTextFormat(Qt::RichText);
    kritaWebsiteLink->setTextInteractionFlags(Qt::TextBrowserInteraction);
    kritaWebsiteLink->setOpenExternalLinks(true);

    sourceCodeLink->setText(QString("<a href=\"https://phabricator.kde.org/source/krita/\">").append(i18n("Source Code")).append("</a>"));
    sourceCodeLink->setTextFormat(Qt::RichText);
    sourceCodeLink->setTextInteractionFlags(Qt::TextBrowserInteraction);
    sourceCodeLink->setOpenExternalLinks(true);

    poweredByKDELink->setText(QString("<a href=\"https://userbase.kde.org/What_is_KDE\">").append(i18n("Powered by KDE")).append("</a>"));
    poweredByKDELink->setTextFormat(Qt::RichText);
    poweredByKDELink->setTextInteractionFlags(Qt::TextBrowserInteraction);
    poweredByKDELink->setOpenExternalLinks(true);
    kdeIcon->setIconSize(QSize(20, 20));
    kdeIcon->setIcon(KisIconUtils::loadIcon(QStringLiteral("kde")).pixmap(20));


    connect(chkShowNews, SIGNAL(toggled(bool)), newsWidget, SLOT(toggleNews(bool)));

    // configure the News area
    KisConfig cfg(true);
    bool m_getNews = cfg.readEntry<bool>("FetchNews", false);
    chkShowNews->setChecked(m_getNews);

}

KisWelcomePageWidget::~KisWelcomePageWidget()
{
}

void KisWelcomePageWidget::setMainWindow(KisMainWindow* mainWin)
{
    if (mainWin) {
        mainWindow = mainWin;

        // set the shortcut links from actions (only if a shortcut exists)
        if ( mainWin->viewManager()->actionManager()->actionByName("file_new")->shortcut().toString() != "") {
            newFileLinkShortcut->setText(QString("(") + mainWin->viewManager()->actionManager()->actionByName("file_new")->shortcut().toString() + QString(")"));
        }

        if (mainWin->viewManager()->actionManager()->actionByName("file_open")->shortcut().toString()  != "") {
            openFileShortcut->setText(QString("(") + mainWin->viewManager()->actionManager()->actionByName("file_open")->shortcut().toString() + QString(")"));
        }
        connect(recentDocumentsListView, SIGNAL(clicked(QModelIndex)), this, SLOT(recentDocumentClicked(QModelIndex)));
        // we need the view manager to actually call actions, so don't create the connections
        // until after the view manager is set
        connect(newFileLink, SIGNAL(clicked(bool)), this, SLOT(slotNewFileClicked()));
        connect(openFileLink, SIGNAL(clicked(bool)), this, SLOT(slotOpenFileClicked()));
        connect(clearRecentFilesLink, SIGNAL(clicked(bool)), this, SLOT(slotClearRecentFiles()));

        slotUpdateThemeColors();
    }
}

void KisWelcomePageWidget::showDropAreaIndicator(bool show)
{
    if (!show) {
        QString dropFrameStyle = "QFrame#dropAreaIndicator { border: 0px }";
        dropFrameBorder->setStyleSheet(dropFrameStyle);
    } else {
        QColor textColor = qApp->palette().color(QPalette::Text);
        QColor backgroundColor = qApp->palette().color(QPalette::Background);
        QColor blendedColor = KritaUtils::blendColors(textColor, backgroundColor, 0.8);

        // QColor.name() turns it into a hex/web format
        QString dropFrameStyle = QString("QFrame#dropAreaIndicator { border: 2px dotted ").append(blendedColor.name()).append(" }") ;
        dropFrameBorder->setStyleSheet(dropFrameStyle);
    }
}

void KisWelcomePageWidget::slotUpdateThemeColors()
{

    QColor textColor = qApp->palette().color(QPalette::Text);
    QColor backgroundColor = qApp->palette().color(QPalette::Background);

    // make the welcome screen labels a subtle color so it doesn't clash with the main UI elements
    QColor blendedColor = KritaUtils::blendColors(textColor, backgroundColor, 0.8);
    QString blendedStyle = QString("color: ").append(blendedColor.name());


    // what labels to change the color...
    startTitleLabel->setStyleSheet(blendedStyle);
    recentDocumentsLabel->setStyleSheet(blendedStyle);
    helpTitleLabel->setStyleSheet(blendedStyle);
    manualLink->setStyleSheet(blendedStyle);
    gettingStartedLink->setStyleSheet(blendedStyle);
    supportKritaLink->setStyleSheet(blendedStyle);
    userCommunityLink->setStyleSheet(blendedStyle);
    kritaWebsiteLink->setStyleSheet(blendedStyle);
    sourceCodeLink->setStyleSheet(blendedStyle);
    newFileLinkShortcut->setStyleSheet(blendedStyle);
    openFileShortcut->setStyleSheet(blendedStyle);
    clearRecentFilesLink->setStyleSheet(blendedStyle);
    poweredByKDELink->setStyleSheet(blendedStyle);
    recentDocumentsListView->setStyleSheet(blendedStyle);

    newFileLink->setStyleSheet(blendedStyle);
    openFileLink->setStyleSheet(blendedStyle);


    // giving the drag area messaging a dotted border
    QString dottedBorderStyle = QString("border: 2px dotted ").append(blendedColor.name()).append("; color:").append(blendedColor.name()).append( ";");
    dragImageHereLabel->setStyleSheet(dottedBorderStyle);


    // make drop area QFrame have a dotted line
    dropFrameBorder->setObjectName("dropAreaIndicator");
    QString dropFrameStyle = QString("QFrame#dropAreaIndicator { border: 4px dotted ").append(blendedColor.name()).append("}");
    dropFrameBorder->setStyleSheet(dropFrameStyle);

    // only show drop area when we have a document over the empty area
    showDropAreaIndicator(false);

    // add icons for new and open settings to make them stand out a bit more
    openFileLink->setIconSize(QSize(30, 30));
    newFileLink->setIconSize(QSize(30, 30));
    openFileLink->setIcon(KisIconUtils::loadIcon("document-open"));
    newFileLink->setIcon(KisIconUtils::loadIcon("document-new"));
}

void KisWelcomePageWidget::populateRecentDocuments()
{
    // grab recent files data
    recentFilesModel = new QStandardItemModel();
    int numRecentFiles = mainWindow->recentFilesUrls().length() > 5 ? 5 : mainWindow->recentFilesUrls().length(); // grab at most 5

    for (int i = 0; i < numRecentFiles; i++ ) {

        QStandardItem *recentItem = new QStandardItem(1,2); // 1 row, 1 column
        recentItem->setIcon(KisIconUtils::loadIcon("document-export"));

        QString recentFileUrlPath = mainWindow->recentFilesUrls().at(i).toString();
        QString fileName = recentFileUrlPath.split("/").last();

        // get thumbnail -- almost all Krita-supported formats save a thumbnail
        // this was mostly copied from the KisAutoSaveRecovery file
        if (recentFileUrlPath.endsWith("ora") || recentFileUrlPath.endsWith("kra")) {
            QScopedPointer<KoStore> store(KoStore::createStore(QUrl(recentFileUrlPath), KoStore::Read));
            if (store) {
                if (store->open(QString("Thumbnails/thumbnail.png"))
                        || store->open(QString("preview.png"))) {

                    QByteArray bytes = store->read(store->size());
                    store->close();
                    QImage img;
                    img.loadFromData(bytes);
                    recentItem->setIcon(QIcon(QPixmap::fromImage(img)));

                }
            }
        }

        else {
            QImage img(QUrl(recentFileUrlPath).toLocalFile());
            if (!img.isNull()) {
                recentItem->setIcon(QIcon(QPixmap::fromImage(img.scaledToWidth(256))));
            }
        }

        // set the recent object with the data
        recentItem->setText(fileName); // what to display for the item
        recentItem->setToolTip(recentFileUrlPath);
        recentFilesModel->appendRow(recentItem);
    }


    // hide clear and Recent files title if there are none
    bool hasRecentFiles = mainWindow->recentFilesUrls().length() > 0;
    recentDocumentsLabel->setVisible(hasRecentFiles);
    clearRecentFilesLink->setVisible(hasRecentFiles);


    recentDocumentsListView->setIconSize(QSize(40, 40));
    recentDocumentsListView->setModel(recentFilesModel);
}


void KisWelcomePageWidget::recentDocumentClicked(QModelIndex index)
{
    QString fileUrl = index.data(Qt::ToolTipRole).toString();
    mainWindow->openDocument(QUrl(fileUrl), KisMainWindow::None );
}


void KisWelcomePageWidget::slotNewFileClicked()
{
    mainWindow->slotFileNew();
}

void KisWelcomePageWidget::slotOpenFileClicked()
{
    mainWindow->slotFileOpen();
}

void KisWelcomePageWidget::slotClearRecentFiles()
{
    mainWindow->clearRecentFiles();
    mainWindow->reloadRecentFileList();
    populateRecentDocuments();
}
