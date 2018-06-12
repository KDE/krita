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
#include "KoStore.h"


KisWelcomePageWidget::KisWelcomePageWidget(QWidget *parent)
{
   Q_UNUSED(parent);
   setupUi(this);

   recentDocumentsListView->viewport()->setAutoFillBackground(false);
   recentDocumentsListView->setSpacing(5);
}

KisWelcomePageWidget::~KisWelcomePageWidget()
{
}

void KisWelcomePageWidget::setMainWindow(KisMainWindow* mainWin)
{
    if (mainWin) {
        mainWindow = mainWin;


        // set the shortcut links from actions
        newFileLinkShortcut->setText(QString("(") + mainWin->viewManager()->actionManager()->actionByName("file_new")->shortcut().toString() + QString(")"));
        openFileShortcut->setText(QString("(") + mainWin->viewManager()->actionManager()->actionByName("file_open")->shortcut().toString() + QString(")"));


        // grab recent files data
        recentFilesModel = new QStandardItemModel();

        for (int i = 0; i < mainWindow->recentFilesUrls().length(); i++ ) {

           QStandardItem *recentItem = new QStandardItem(1,2); // 1 row, 1 column
           QString recentFileUrlPath = mainWindow->recentFilesUrls().at(i).toString();
           QString fileName = recentFileUrlPath.split("/").last();


           // get thumbnail -- almost all Krita-supported formats save a thumbnail
           // this was mostly copied from the KisAutoSaveRecorvery file
           KoStore* store = KoStore::createStore(QUrl(recentFileUrlPath), KoStore::Read);
           if (store) {
               if(store->open(QString("Thumbnails/thumbnail.png"))
                  || store->open(QString("preview.png"))) {

                   QByteArray bytes = store->read(store->size());
                   store->close();
                   QImage img;
                   img.loadFromData(bytes);
                   recentItem->setIcon(QIcon(QPixmap::fromImage(img)));
               }else {
                   recentItem->setIcon(KisIconUtils::loadIcon("document-export"));
               }

               delete store;
           } else {
               recentItem->setIcon(KisIconUtils::loadIcon("document-export"));
           }


           // set the recent object with the data
           recentItem->setText(fileName); // what to display for the item
           recentItem->setToolTip(recentFileUrlPath);

           recentFilesModel->appendRow(recentItem);
        }

        recentDocumentsListView->setIconSize(QSize(48, 48));
        recentDocumentsListView->setModel(recentFilesModel);
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

    }
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
