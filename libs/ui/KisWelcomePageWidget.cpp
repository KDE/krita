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
#include <QPixmap>
#include <QImage>
#include <QThread>

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
#include "KisDocument.h"
#include <kis_image.h>
#include <kis_paint_device.h>
#include <KisPart.h>

#include <kritaversion.h>
#include <QSysInfo>
#include <kis_config.h>
#include <kis_image_config.h>
#include "opengl/kis_opengl.h"




KisWelcomePageWidget::KisWelcomePageWidget(QWidget *parent)
    : QWidget(parent)
{
    setupUi(this);

#ifdef KRITA_BETA
    lblSurveyLink->setVisible(true);
    lblSurveyLink->setTextInteractionFlags(Qt::TextBrowserInteraction);

    QString surveyId = "965214";

    QString arguments;
    arguments += "&KritaVersion=" + QStringLiteral(KRITA_VERSION_STRING);
    arguments += "&BuildAbi=" + QSysInfo::buildAbi();
    arguments += "&BuildCPUArchitecture=" + QSysInfo::buildCpuArchitecture();
    arguments += "&CurrentCPUArchitecture=" + QSysInfo::currentCpuArchitecture();
    arguments += "&KernelVersion=" + QSysInfo::kernelVersion();
    arguments += "&ProductType=" + QSysInfo::productType();
    arguments += "&ProductVersion=" + QSysInfo::productVersion();
    arguments += "&InstantPreviewEnabled=" + QString(KisConfig(true).levelOfDetailEnabled( )? "true" : "false");
    arguments += "&TotalMemorySize=" + QString::number(KisImageConfig(true).totalRAM());
    arguments += "&SwapLocation=" + KisImageConfig(true).swapDir();
    arguments += "&NumberOfCores=" + QString::number(QThread::idealThreadCount());
    arguments += "&HiDPI=" + QString(QCoreApplication::testAttribute(Qt::AA_EnableHighDpiScaling) ? "true" : "false");
    arguments += "&OpenGLRenderer=" + KisOpenGL::convertOpenGLRendererToConfig(KisOpenGL::getCurrentOpenGLRenderer());

    QString openGLInfo = KisOpenGL::getDebugText();
    QStringList infolines = openGLInfo.split("\n");
    Q_FOREACH(const QString line, infolines) {
        if (line.contains("Vendor")) {
            arguments += "&GPUVendor=" + line.split(": ")[1].replace("\n", "");
        }
        if (line.contains("Version")) {
            arguments += "&OpenGLVersion=" + line.split(": ")[1].replace("\n", "");
        }
        if (line.contains("Current Format")) {
            arguments += "&OpenGLFormat=" + line.split(": ")[1].replace("\n", "");
        }
    }
    QUrl url (arguments);

    QString source = QString("<html><head/><body><p align=\"center\"><a href=\"https://survey.kde.org/index.php/%1?lang=en?%2\">"
                             "<span style=\"font-size:18pt; font-weight:600; text-decoration: underline; color:#2980b9;\">"
                             "Help Krita, do the survey!"
                              "</span></a></p></body></html>").arg(surveyId).arg(QString::fromUtf8(url.toEncoded()));

    lblSurveyLink->setText(source);
#else
    lblSurveyLink->setVisible(false);
#endif

    recentDocumentsListView->setDragEnabled(false);
    recentDocumentsListView->viewport()->setAutoFillBackground(false);
    recentDocumentsListView->setSpacing(2);

    // set up URLs that go to web browser
    manualLink->setTextFormat(Qt::RichText);
    manualLink->setTextInteractionFlags(Qt::TextBrowserInteraction);
    manualLink->setOpenExternalLinks(true);

    gettingStartedLink->setTextFormat(Qt::RichText);
    gettingStartedLink->setTextInteractionFlags(Qt::TextBrowserInteraction);
    gettingStartedLink->setOpenExternalLinks(true);

    supportKritaLink->setTextFormat(Qt::RichText);
    supportKritaLink->setTextInteractionFlags(Qt::TextBrowserInteraction);
    supportKritaLink->setOpenExternalLinks(true);

    userCommunityLink->setTextFormat(Qt::RichText);
    userCommunityLink->setTextInteractionFlags(Qt::TextBrowserInteraction);
    userCommunityLink->setOpenExternalLinks(true);


    kritaWebsiteLink->setTextFormat(Qt::RichText);
    kritaWebsiteLink->setTextInteractionFlags(Qt::TextBrowserInteraction);
    kritaWebsiteLink->setOpenExternalLinks(true);


    sourceCodeLink->setTextFormat(Qt::RichText);
    sourceCodeLink->setTextInteractionFlags(Qt::TextBrowserInteraction);
    sourceCodeLink->setOpenExternalLinks(true);

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

        // allows RSS news items to apply analytics tracking.
        newsWidget->setAnalyticsTracking("?" + analyticsString);

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
    newFileLinkShortcut->setStyleSheet(blendedStyle);
    openFileShortcut->setStyleSheet(blendedStyle);
    clearRecentFilesLink->setStyleSheet(blendedStyle);
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


    kdeIcon->setIcon(KisIconUtils::loadIcon(QStringLiteral("kde")).pixmap(20));

    // HTML links seem to be a bit more stubborn with theme changes... setting inline styles to help with color change
    userCommunityLink->setText(QString("<a style=\"color: " + blendedColor.name() + " \" href=\"https://forum.kde.org/viewforum.php?f=136&" + analyticsString + "user-community" + "\">")
                               .append(i18n("User Community")).append("</a>"));

    gettingStartedLink->setText(QString("<a style=\"color: " + blendedColor.name() + " \" href=\"https://docs.krita.org/en/user_manual/getting_started.html?" + analyticsString + "getting-started" + "\">")
                                .append(i18n("Getting Started")).append("</a>"));

    manualLink->setText(QString("<a style=\"color: " + blendedColor.name() + " \" href=\"https://docs.krita.org?" + analyticsString + "documentation-site" + "\">")
                        .append(i18n("User Manual")).append("</a>"));

    supportKritaLink->setText(QString("<a style=\"color: " + blendedColor.name() + " \" href=\"https://krita.org/en/support-us/donations?" + analyticsString + "donations" + "\">")
                              .append(i18n("Support Krita")).append("</a>"));

    kritaWebsiteLink->setText(QString("<a style=\"color: " + blendedColor.name() + " \" href=\"https://www.krita.org?" + analyticsString + "marketing-site" + "\">")
                              .append(i18n("Krita Website")).append("</a>"));

    sourceCodeLink->setText(QString("<a style=\"color: " + blendedColor.name() + " \" href=\"https://invent.kde.org/kde/krita?" + analyticsString + "source-code" + "\">")
                            .append(i18n("Source Code")).append("</a>"));

    poweredByKDELink->setText(QString("<a style=\"color: " + blendedColor.name() + " \" href=\"https://userbase.kde.org/What_is_KDE?" + analyticsString + "what-is-kde" + "\">")
                              .append(i18n("Powered by KDE")).append("</a>"));
    // re-populate recent files since they might have themed icons
    populateRecentDocuments();

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

        QList<QUrl> brokenUrls;

        if (m_thumbnailMap.contains(recentFileUrlPath)) {
            recentItem->setIcon(m_thumbnailMap[recentFileUrlPath]);
        }
        else {
            QFileInfo fi(recentFileUrlPath);

            if (fi.exists()) {
                if (fi.suffix() == "ora" || fi.suffix() == "kra") {

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
                    else {
                        brokenUrls << m_mainWindow->recentFilesUrls().at(i);
                    }
                }
                else if (fi.suffix() == "tiff" || fi.suffix() == "tif") {
                    // Workaround for a bug in Qt tiff QImageIO plugin
                    QScopedPointer<KisDocument> doc;
                    doc.reset(KisPart::instance()->createDocument());
                    doc->setFileBatchMode(true);
                    bool r = doc->openUrl(QUrl::fromLocalFile(recentFileUrlPath), KisDocument::DontAddToRecent);
                    if (r) {
                        KisPaintDeviceSP projection = doc->image()->projection();
                        recentItem->setIcon(QIcon(QPixmap::fromImage(projection->createThumbnail(48, 48, projection->exactBounds()))));
                    }
                    else {
                        brokenUrls << m_mainWindow->recentFilesUrls().at(i);
                    }
                }
                else {
                    QImage img;
                    img.setDevicePixelRatio(devicePixelRatioF());
                    img.load(recentFileUrlPath);
                    if (!img.isNull()) {
                        recentItem->setIcon(QIcon(QPixmap::fromImage(img.scaledToWidth(48))));
                    }
                    else {
                        brokenUrls << m_mainWindow->recentFilesUrls().at(i);
                    }
                }
                if (brokenUrls.size() > 0 && brokenUrls.last().toLocalFile() != recentFileUrlPath) {
                    m_thumbnailMap[recentFileUrlPath] = recentItem->icon();
                }
            }
        }
        Q_FOREACH(const QUrl &url, brokenUrls) {
            m_mainWindow->removeRecentUrl(url);
        }
        // set the recent object with the data
        if (brokenUrls.isEmpty() || brokenUrls.last().toLocalFile() != recentFileUrlPath) {
            recentItem->setText(fileName); // what to display for the item
            recentItem->setToolTip(recentFileUrlPath);
            m_recentFilesModel.appendRow(recentItem);
        }
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
