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
#include <QFileInfo>
#include <QMimeData>

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

    setAcceptDrops(true);

}

KisWelcomePageWidget::~KisWelcomePageWidget()
{
}

void KisWelcomePageWidget::setMainWindow(KisMainWindow* mainWin)
{
    if (mainWin) {
        m_mainWindow = mainWin;

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
    m_recentFilesModel.clear(); // clear existing data before it gets re-populated

    // grab recent files data
    int numRecentFiles = m_mainWindow->recentFilesUrls().length() > 5 ? 5 : m_mainWindow->recentFilesUrls().length(); // grab at most 5

    for (int i = 0; i < numRecentFiles; i++ ) {

        QStandardItem *recentItem = new QStandardItem(1,2); // 1 row, 1 column
        recentItem->setIcon(KisIconUtils::loadIcon("document-export"));

        QString recentFileUrlPath = m_mainWindow->recentFilesUrls().at(i).toLocalFile();
        QString fileName = recentFileUrlPath.split("/").last();

        if (m_thumbnailMap.contains(recentFileUrlPath)) {
            recentItem->setIcon(m_thumbnailMap[recentFileUrlPath]);
        }
        else {
            if (QFileInfo(recentFileUrlPath).exists()) {
                if (recentFileUrlPath.toLower().endsWith("ora") || recentFileUrlPath.toLower().endsWith("kra")) {

                    QScopedPointer<KoStore> store(KoStore::createStore(recentFileUrlPath, KoStore::Read));
                    if (store) {
                        QString thumbnailpath;
                        if (store->hasFile(QString("Thumbnails/thumbnail.png"))){
                            thumbnailpath = QString("Thumbnails/thumbnail.png");
                        } else if (store->hasFile(QString("preview.png"))) {
                            thumbnailpath = QString("preview.png");
                        }
                        if (!thumbnailpath.isEmpty()) {
                            if (store->open(thumbnailpath)) {

                                QByteArray bytes = store->read(store->size());
                                store->close();
                                QImage img;
                                img.loadFromData(bytes);
                                img.setDevicePixelRatio(devicePixelRatioF());
                                recentItem->setIcon(QIcon(QPixmap::fromImage(img)));
                            }
                        }
                    }
                }
                else {
                    QImage img;
                    img.setDevicePixelRatio(devicePixelRatioF());
                    img.load(recentFileUrlPath);
                    if (!img.isNull()) {
                        recentItem->setIcon(QIcon(QPixmap::fromImage(img.scaledToWidth(48))));
                    }
                }
                m_thumbnailMap[recentFileUrlPath] = recentItem->icon();
            }
        }

        // set the recent object with the data
        recentItem->setText(fileName); // what to display for the item
        recentItem->setToolTip(recentFileUrlPath);
        m_recentFilesModel.appendRow(recentItem);
    }

    // hide clear and Recent files title if there are none
    bool hasRecentFiles = m_mainWindow->recentFilesUrls().length() > 0;

    recentDocumentsLabel->setVisible(hasRecentFiles);
    clearRecentFilesLink->setVisible(hasRecentFiles);

    recentDocumentsListView->setIconSize(QSize(48, 48));
    recentDocumentsListView->setModel(&m_recentFilesModel);
}

void KisWelcomePageWidget::dragEnterEvent(QDragEnterEvent *event)
{
    //qDebug() << "dragEnterEvent formats" << event->mimeData()->formats() << "urls" << event->mimeData()->urls() << "has images" << event->mimeData()->hasImage();
    showDropAreaIndicator(true);
    if (event->mimeData()->hasUrls() ||
        event->mimeData()->hasFormat("application/x-krita-node") ||
        event->mimeData()->hasFormat("application/x-qt-image")) {

        event->accept();
    }
}

void KisWelcomePageWidget::dropEvent(QDropEvent *event)
{
    //qDebug() << "KisWelcomePageWidget::dropEvent() formats" << event->mimeData()->formats() << "urls" << event->mimeData()->urls() << "has images" << event->mimeData()->hasImage();

    showDropAreaIndicator(false);

    if (event->mimeData()->hasUrls() && event->mimeData()->urls().size() > 0) {
        Q_FOREACH (const QUrl &url, event->mimeData()->urls()) {
            if (url.toLocalFile().endsWith(".bundle")) {
                bool r = m_mainWindow->installBundle(url.toLocalFile());
                if (!r) {
                    qWarning() << "Could not install bundle" << url.toLocalFile();
                }
            }
            else {
                m_mainWindow->openDocument(url, KisMainWindow::None);
            }
        }
    }
}

void KisWelcomePageWidget::dragMoveEvent(QDragMoveEvent *event)
{
    //qDebug() << "dragMoveEvent";
    m_mainWindow->dragMoveEvent(event);
    if (event->mimeData()->hasUrls() ||
        event->mimeData()->hasFormat("application/x-krita-node") ||
        event->mimeData()->hasFormat("application/x-qt-image")) {

        event->accept();
    }

}

void KisWelcomePageWidget::dragLeaveEvent(QDragLeaveEvent */*event*/)
{
    //qDebug() << "dragLeaveEvent";
    showDropAreaIndicator(false);
    m_mainWindow->dragLeave();
}

void KisWelcomePageWidget::recentDocumentClicked(QModelIndex index)
{
    QString fileUrl = index.data(Qt::ToolTipRole).toString();
    m_mainWindow->openDocument(QUrl::fromLocalFile(fileUrl), KisMainWindow::None );
}


void KisWelcomePageWidget::slotNewFileClicked()
{
    m_mainWindow->slotFileNew();
}

void KisWelcomePageWidget::slotOpenFileClicked()
{
    m_mainWindow->slotFileOpen();
}

void KisWelcomePageWidget::slotClearRecentFiles()
{
    m_mainWindow->clearRecentFiles();
    populateRecentDocuments();
}
