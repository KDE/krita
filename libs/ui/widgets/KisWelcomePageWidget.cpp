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


KisWelcomePageWidget::KisWelcomePageWidget(QWidget *parent)
    : QWidget(parent)
{
   setupUi(this);

   recentDocumentsListView->setDragEnabled(false);
   recentDocumentsListView->viewport()->setAutoFillBackground(false);
   recentDocumentsListView->setSpacing(2);
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


        populateRecentDocuments();
        connect(recentDocumentsListView, SIGNAL(clicked(QModelIndex)), this, SLOT(recentDocumentClicked(QModelIndex)));


        // we need the view manager to actually call actions, so don't create the connections
        // until after the view manager is set
        connect(newFileLink, SIGNAL(clicked(bool)), this, SLOT(slotNewFileClicked()));
        connect(openFileLink, SIGNAL(clicked(bool)), this, SLOT(slotOpenFileClicked()));

        // URL link connections
        connect(manualLink, SIGNAL(clicked(bool)), this, SLOT(slotGoToManual()));

        connect(gettingStartedLink, SIGNAL(clicked(bool)), this, SLOT(slotGettingStarted()));
        connect(supportKritaLink, SIGNAL(clicked(bool)), this, SLOT(slotSupportKrita()));
        connect(userCommunityLink, SIGNAL(clicked(bool)), this, SLOT(slotUserCommunity()));
        connect(kritaWebsiteLink, SIGNAL(clicked(bool)), this, SLOT(slotKritaWebsite()));
        connect(sourceCodeLink, SIGNAL(clicked(bool)), this, SLOT(slotSourceCode()));
        connect(clearRecentFilesLink, SIGNAL(clicked(bool)), this, SLOT(slotClearRecentFiles()));
        connect(poweredByKDELink, SIGNAL(clicked(bool)), this, SLOT(slotKDESiteLink()));


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

    // needed for updating icon color for files that don't have a preview
    if (mainWindow) {
        populateRecentDocuments();
    }
}

void KisWelcomePageWidget::populateRecentDocuments()
{
    // grab recent files data
    recentFilesModel = new QStandardItemModel();
    int recentDocumentsIterator = mainWindow->recentFilesUrls().length() > 5 ? 5 : mainWindow->recentFilesUrls().length(); // grab at most 5

    for (int i = 0; i < recentDocumentsIterator; i++ ) {

       QStandardItem *recentItem = new QStandardItem(1,2); // 1 row, 1 column
       QString recentFileUrlPath = mainWindow->recentFilesUrls().at(i).toString();
       QString fileName = recentFileUrlPath.split("/").last();


       // get thumbnail -- almost all Krita-supported formats save a thumbnail
       // this was mostly copied from the KisAutoSaveRecovery file
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
           else {
               recentItem->setIcon(KisIconUtils::loadIcon("document-export"));
           }

       }
       else {
           recentItem->setIcon(KisIconUtils::loadIcon("document-export"));
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

void KisWelcomePageWidget::slotGoToManual()
{
    QDesktopServices::openUrl(QUrl("https://docs.krita.org"));
}

void KisWelcomePageWidget::slotGettingStarted()
{
    QDesktopServices::openUrl(QUrl("https://docs.krita.org/en/user_manual/getting_started.html"));
}

void KisWelcomePageWidget::slotSupportKrita()
{
    QDesktopServices::openUrl(QUrl("https://krita.org/en/support-us/donations/"));
}

void KisWelcomePageWidget::slotUserCommunity()
{
    QDesktopServices::openUrl(QUrl("https://forum.kde.org/viewforum.php?f=136"));
}

void KisWelcomePageWidget::slotKritaWebsite()
{
    QDesktopServices::openUrl(QUrl("https://www.krita.org"));
}

void KisWelcomePageWidget::slotSourceCode()
{
    QDesktopServices::openUrl(QUrl("https://phabricator.kde.org/source/krita/"));
}

void KisWelcomePageWidget::slotKDESiteLink()
{
    QDesktopServices::openUrl(QUrl("https://userbase.kde.org/What_is_KDE"));
}
