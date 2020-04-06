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
#include <QDesktopServices>
#include <QFileInfo>
#include <QMimeData>
#include <QPixmap>
#include <QImage>
#include <QMessageBox>

#include <KisMimeDatabase.h>
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
#include <utils/KisFileIconCreator.h>

#include <utils/KisUpdaterBase.h>

#include <QCoreApplication>
#include <kis_debug.h>
#include <QDir>

#include "config-updaters.h"

#ifdef ENABLE_UPDATERS
#ifdef Q_OS_LINUX
#include <utils/KisAppimageUpdater.h>
#endif

#include <utils/KisManualUpdater.h>
#endif

#include <klocalizedstring.h>
#include <KritaVersionWrapper.h>

#include <KisUsageLogger.h>

KisWelcomePageWidget::KisWelcomePageWidget(QWidget *parent)
    : QWidget(parent)
{
    setupUi(this);
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


    versionNotificationLabel->setTextFormat(Qt::RichText);
    versionNotificationLabel->setTextInteractionFlags(Qt::TextBrowserInteraction);
    versionNotificationLabel->setOpenExternalLinks(true);

    devBuildIcon->setIcon(KisIconUtils::loadIcon("warning"));

    devBuildLabel->setTextFormat(Qt::RichText);
    devBuildLabel->setTextInteractionFlags(Qt::TextBrowserInteraction);
    devBuildLabel->setOpenExternalLinks(true);
    devBuildLabel->setVisible(false);

    updaterFrame->setVisible(false);
    versionNotificationLabel->setVisible(false);
    bnVersionUpdate->setVisible(false);
    bnErrorDetails->setVisible(false);

    connect(chkShowNews, SIGNAL(toggled(bool)), newsWidget, SLOT(toggleNews(bool)));

#ifdef ENABLE_UPDATERS
    connect(chkShowNews, SIGNAL(toggled(bool)), this, SLOT(slotToggleUpdateChecks(bool)));
#endif

#ifdef Q_OS_ANDROID
    // enabling this widgets crashes the app, so it is better for it to be hidden for now
    newsWidget->hide();
    helpTitleLabel_2->hide();
    chkShowNews->hide();

    donationLink = new QLabel(dropFrameBorder);
    donationLink->setOpenExternalLinks(true);
    donationLink->setTextInteractionFlags(Qt::TextBrowserInteraction);

    QFont f = font();
    f.setPointSize(15);
    donationLink->setFont(f);

    verticalLayout_3->addWidget(donationLink);
    verticalLayout_3->setSpacing(20);

    QLabel *donationBannerImage = new QLabel(dropFrameBorder);
    QString bannerPath = QStandardPaths::locate(QStandardPaths::AppDataLocation, "share/krita/donation/banner.png");
    donationBannerImage->setPixmap(QPixmap(bannerPath));

    verticalLayout_3->addWidget(donationBannerImage);
#endif

    // configure the News area
    KisConfig cfg(true);
    m_checkUpdates = cfg.readEntry<bool>("FetchNews", false);


#ifdef ENABLE_UPDATERS
#ifndef Q_OS_ANDROID
    // Setup version updater, but do not check for them, unless the user explicitly
    // wants to check for updates.
    // * No updater is created for Linux/Steam, Windows/Steam and Windows/Store distributions,
    // as those stores have their own updating mechanism.
    // * STEAMAPPID(Windows)/SteamAppId(Linux) environment variable is set when Krita is run from Steam.
    // The environment variables are not public API.
    // * AppxManifest.xml file in the installation directory indicates MS Store version
#if defined Q_OS_LINUX
    if (!qEnvironmentVariableIsSet("SteamAppId")) { // do not create updater for linux/steam
        if (qEnvironmentVariableIsSet("APPIMAGE")) {
            m_versionUpdater.reset(new KisAppimageUpdater());
        } else {
            m_versionUpdater.reset(new KisManualUpdater());
        }
    }
#elif defined Q_OS_WIN
    QString appxManifestFilePath = QString("%1/../AppxManifest.xml").arg(QCoreApplication::applicationDirPath());
	QFileInfo appxManifestFileInfo(appxManifestFilePath);

    if (!appxManifestFileInfo.exists() && !qEnvironmentVariableIsSet("STEAMAPPID")) {
 		m_versionUpdater.reset(new KisManualUpdater());
        KisUsageLogger::log("Non-store package - creating updater");
    } else {
        KisUsageLogger::log("detected appx or steam package - not creating the updater");
    }

#else
	// always create updater for MacOS
    m_versionUpdater.reset(new KisManualUpdater());
#endif // Q_OS_*

	if (!m_versionUpdater.isNull()) {
		connect(bnVersionUpdate, SIGNAL(clicked()), this, SLOT(slotRunVersionUpdate()));
		connect(bnErrorDetails, SIGNAL(clicked()), this, SLOT(slotShowUpdaterErrorDetails()));
		connect(m_versionUpdater.data(), SIGNAL(sigUpdateCheckStateChange(KisUpdaterStatus)),
				this, SLOT(slotSetUpdateStatus(const KisUpdaterStatus&)));

		if (m_checkUpdates) { // only if the user wants them
			m_versionUpdater->checkForUpdate();
		}
	}
#endif // ifndef Q_OS_ANDROID
#endif // ENABLE_UPDATERS

    chkShowNews->setChecked(m_checkUpdates);

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
            newFileLinkShortcut->setText(
                QString("(") + mainWin->viewManager()->actionManager()->actionByName("file_new")->shortcut().toString(QKeySequence::NativeText) + QString(")"));
        }
        if (mainWin->viewManager()->actionManager()->actionByName("file_open")->shortcut().toString()  != "") {
            openFileShortcut->setText(
                QString("(") + mainWin->viewManager()->actionManager()->actionByName("file_open")->shortcut().toString(QKeySequence::NativeText) + QString(")"));
        }
        connect(recentDocumentsListView, SIGNAL(clicked(QModelIndex)), this, SLOT(recentDocumentClicked(QModelIndex)));
        // we need the view manager to actually call actions, so don't create the connections
        // until after the view manager is set
        connect(newFileLink, SIGNAL(clicked(bool)), this, SLOT(slotNewFileClicked()));
        connect(openFileLink, SIGNAL(clicked(bool)), this, SLOT(slotOpenFileClicked()));
        connect(clearRecentFilesLink, SIGNAL(clicked(bool)), mainWin, SLOT(clearRecentFiles()));

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

    textColor = qApp->palette().color(QPalette::Text);
    backgroundColor = qApp->palette().color(QPalette::Background);

    // make the welcome screen labels a subtle color so it doesn't clash with the main UI elements
    blendedColor = KritaUtils::blendColors(textColor, backgroundColor, 0.8);
    blendedStyle = QString("color: ").append(blendedColor.name());


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
    userCommunityLink->setText(QString("<a style=\"color: " + blendedColor.name() + " \" href=\"https://krita-artists.org" + analyticsString + "user-community" + "\">")
                               .append(i18n("User Community")).append("</a>"));

    gettingStartedLink->setText(QString("<a style=\"color: " + blendedColor.name() + " \" href=\"https://docs.krita.org/en/user_manual/getting_started.html?" + analyticsString + "getting-started" + "\">")
                                .append(i18n("Getting Started")).append("</a>"));

    manualLink->setText(QString("<a style=\"color: " + blendedColor.name() + " \" href=\"https://docs.krita.org?" + analyticsString + "documentation-site" + "\">")
                        .append(i18n("User Manual")).append("</a>"));

    supportKritaLink->setText(QString("<a style=\"color: " + blendedColor.name() + " \" href=\"https://krita.org/en/support-us/donations?" + analyticsString + "donations" + "\">")
                              .append(i18n("Support Krita")).append("</a>"));

    kritaWebsiteLink->setText(QString("<a style=\"color: " + blendedColor.name() + " \" href=\"https://www.krita.org?" + analyticsString + "marketing-site" + "\">")
                              .append(i18n("Krita Website")).append("</a>"));

    sourceCodeLink->setText(QString("<a style=\"color: " + blendedColor.name() + " \" href=\"https://invent.kde.org/graphics/krita.git?" + analyticsString + "source-code" + "\">")
                            .append(i18n("Source Code")).append("</a>"));

    poweredByKDELink->setText(QString("<a style=\"color: " + blendedColor.name() + " \" href=\"https://userbase.kde.org/What_is_KDE?" + analyticsString + "what-is-kde" + "\">")
                              .append(i18n("Powered by KDE")).append("</a>"));


    // show the dev version labels, if dev version is detected
    showDevVersionHighlight();

#ifdef ENABLE_UPDATERS
    updateVersionUpdaterFrame(); // updater frame
#endif


#ifdef Q_OS_ANDROID
    donationLink->setText(QString("<a style=\"color: " + blendedColor.name() + " \" href=\"https://krita.org/en/support-us/donations?" + analyticsString + "donations" + "\">")
                              .append(i18n("Krita is free and open source.")).append("<br>").append(i18n("Support Krita's Development!")).append("</a>"));
#endif
    // re-populate recent files since they might have themed icons
    populateRecentDocuments();

}

void KisWelcomePageWidget::populateRecentDocuments()
{

    m_recentFilesModel.clear(); // clear existing data before it gets re-populated

    // grab recent files data
    int numRecentFiles = m_mainWindow->recentFilesUrls().length() > 5 ? 5 : m_mainWindow->recentFilesUrls().length(); // grab at most 5
    KisFileIconCreator iconCreator;

    for (int i = 0; i < numRecentFiles; i++ ) {

        QStandardItem *recentItem = new QStandardItem(1,2); // 1 row, 1 column
        recentItem->setIcon(KisIconUtils::loadIcon("document-export"));

        QString recentFileUrlPath = m_mainWindow->recentFilesUrls().at(i).toLocalFile();

        QString fileName = QFileInfo(recentFileUrlPath).fileName();

        QList<QUrl> brokenUrls;


        if (m_thumbnailMap.contains(recentFileUrlPath)) {
            recentItem->setIcon(m_thumbnailMap[recentFileUrlPath]);
        } else {
            QIcon icon;
            bool success = iconCreator.createFileIcon(recentFileUrlPath, icon, devicePixelRatioF());
            if (success) {
                recentItem->setIcon(icon);
                m_thumbnailMap[recentFileUrlPath] = recentItem->icon();
            } else {
                brokenUrls << m_mainWindow->recentFilesUrls().at(i);
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

void KisWelcomePageWidget::showDevVersionHighlight()
{
    // always flag developement version
    if (isDevelopmentBuild()) {
        QString devBuildLabelText = QString("<a style=\"color: " +
                                           blendedColor.name() +
                                           " \" href=\"https://docs.krita.org/en/untranslatable_pages/triaging_bugs.html?"
                                           + analyticsString + "dev-build" + "\">")
                                  .append(i18n("DEV BUILD")).append("</a>");

        devBuildLabel->setText(devBuildLabelText);
        devBuildIcon->setVisible(true);
        devBuildLabel->setVisible(true);
    } else {
        devBuildIcon->setVisible(false);
        devBuildLabel->setVisible(false);
    }
}

void KisWelcomePageWidget::recentDocumentClicked(QModelIndex index)
{
    QString fileUrl = index.data(Qt::ToolTipRole).toString();
    m_mainWindow->openDocument(QUrl::fromLocalFile(fileUrl), KisMainWindow::None );
}


bool KisWelcomePageWidget::isDevelopmentBuild()
{
    QString versionString = KritaVersionWrapper::versionString(true);

    if (versionString.contains("git")) {
        return true;
    } else {
        return false;
    }
}

void KisWelcomePageWidget::slotNewFileClicked()
{
    m_mainWindow->slotFileNew();
}

void KisWelcomePageWidget::slotOpenFileClicked()
{
    m_mainWindow->slotFileOpen();
}

#ifdef ENABLE_UPDATERS
void KisWelcomePageWidget::slotToggleUpdateChecks(bool state)
{
	if (m_versionUpdater.isNull()) {
		return;
	}

	m_checkUpdates = state;

    if (m_checkUpdates) {
        m_versionUpdater->checkForUpdate();
    }

    updateVersionUpdaterFrame();
}

void KisWelcomePageWidget::slotRunVersionUpdate()
{
	if (m_versionUpdater.isNull()) {
		return;
	}

	if (m_checkUpdates) {
		m_versionUpdater->doUpdate();
	}
}

void KisWelcomePageWidget::slotSetUpdateStatus(KisUpdaterStatus updateStatus)
{
    m_updaterStatus = updateStatus;
    updateVersionUpdaterFrame();
}

void KisWelcomePageWidget::slotShowUpdaterErrorDetails()
{
    QMessageBox::warning(0, i18nc("@title:window", "Krita"), m_updaterStatus.updaterOutput());
}

void KisWelcomePageWidget::updateVersionUpdaterFrame()
{
    updaterFrame->setVisible(false);
    versionNotificationLabel->setVisible(false);
    bnVersionUpdate->setVisible(false);
    bnErrorDetails->setVisible(false);

    if (!m_checkUpdates || m_versionUpdater.isNull()) {
        return;
    }

    QString versionLabelText;

    if (m_updaterStatus.status() == UpdaterStatus::StatusID::UPDATE_AVAILABLE) {
        updaterFrame->setVisible(true);
        updaterFrame->setEnabled(true);
        versionLabelText = i18n("New version of Krita is available.");
        versionNotificationLabel->setVisible(true);
        updateIcon->setIcon(KisIconUtils::loadIcon("update-medium"));

        if (m_versionUpdater->hasUpdateCapability()) {
            bnVersionUpdate->setVisible(true);
//            bnVersionUpdate->setEnabled(true);
        } else {
            // build URL for label
            QString downloadLink = QString(" <a style=\"color: %1; text-decoration: underline\" href=\"%2?%3\">Download Krita %4</a>")
                    .arg(blendedColor.name())
                    .arg(m_updaterStatus.downloadLink())
                    .arg(analyticsString + "version-update")
                    .arg(m_updaterStatus.availableVersion());

            versionLabelText.append(downloadLink);
        }

    } else if (
               (m_updaterStatus.status() == UpdaterStatus::StatusID::UPTODATE)
               || (m_updaterStatus.status() == UpdaterStatus::StatusID::CHECK_ERROR)
               || (m_updaterStatus.status() == UpdaterStatus::StatusID::IN_PROGRESS)
               ){
        // no notifications, if uptodate
        // also, stay silent on check error - we do not want to generate lots of user support issues
        // because of failing wifis and proxies over the world
        updaterFrame->setVisible(false);

    } else if (m_updaterStatus.status() == UpdaterStatus::StatusID::UPDATE_ERROR) {
        updaterFrame->setVisible(true);
        versionLabelText = i18n("An error occurred during the update");
        versionNotificationLabel->setVisible(true);
        bnErrorDetails->setVisible(true);
        updateIcon->setIcon(KisIconUtils::loadIcon("warning"));

//        bnErrorDetails->setEnabled(true);

    } else if (m_updaterStatus.status() == UpdaterStatus::StatusID::RESTART_REQUIRED) {
        updaterFrame->setVisible(true);
        versionLabelText = QString("<b>%1</b> %2").arg(i18n("Restart is required.")).arg(m_updaterStatus.details());
        versionNotificationLabel->setVisible(true);
        updateIcon->setIcon(KisIconUtils::loadIcon("view-refresh"));
    }

    versionNotificationLabel->setText(versionLabelText);
    if (!blendedStyle.isNull()) {
        versionNotificationLabel->setStyleSheet(blendedStyle);
    }
}
#endif
