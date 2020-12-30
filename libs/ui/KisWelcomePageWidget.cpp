/* This file is part of the KDE project
 * SPDX-FileCopyrightText: 2018 Scott Petrovic <scottpetrovic@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
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
#include <QMenu>

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

#include <array>

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
#include <kritaversion.h>
#include <QSysInfo>
#include <kis_config.h>
#include <kis_image_config.h>
#include "opengl/kis_opengl.h"

#ifdef Q_OS_ANDROID
#include <QtAndroid>


QPushButton* KisWelcomePageWidget::donationLink;
QLabel* KisWelcomePageWidget::donationBannerImage;
#endif


// class to override item height for Breeze since qss seems to not work
class RecentItemDelegate : public QStyledItemDelegate
{
    int itemHeight = 0;
public:
    RecentItemDelegate(QObject *parent = 0)
        : QStyledItemDelegate(parent)
    {
    }

    void setItemHeight(int itemHeight)
    {
        this->itemHeight = itemHeight;
    }

    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &/*index*/) const override
    {
        return QSize(option.rect.width(), itemHeight);
    }
};


KisWelcomePageWidget::KisWelcomePageWidget(QWidget *parent)
    : QWidget(parent)
{
    setupUi(this);
    labelNoRecentDocs->setVisible(false);
    recentDocumentsListView->setDragEnabled(false);
    recentDocumentsListView->viewport()->setAutoFillBackground(false);
    recentDocumentsListView->setSpacing(2);
    recentDocumentsListView->installEventFilter(this);

    // set up URLs that go to web browser
    devBuildIcon->setIcon(KisIconUtils::loadIcon("warning"));
    devBuildLabel->setVisible(false);
    updaterFrame->setVisible(false);
    versionNotificationLabel->setVisible(false);
    bnVersionUpdate->setVisible(false);
    bnErrorDetails->setVisible(false);

    QMenu *newsOptionsMenu = new QMenu(this);
    newsOptionsMenu->setToolTipsVisible(true);
    QAction *showNewsAction = newsOptionsMenu->addAction(i18n("Check for updates"));
    showNewsAction->setToolTip(i18n("Show news about Krita: this needs internet to retrieve information from the krita.org website"));
    showNewsAction->setCheckable(true);

    newsOptionsMenu->addSection(i18n("Language"));
    QAction *newsInfoAction = newsOptionsMenu->addAction(i18n("English news are always up to date."));
    newsInfoAction->setEnabled(false);

    setupNewsLangSelection(newsOptionsMenu);
    btnNewsOptions->setMenu(newsOptionsMenu);

    connect(showNewsAction, SIGNAL(toggled(bool)), newsWidget, SLOT(setVisible(bool)));
    connect(showNewsAction, SIGNAL(toggled(bool)), labelNoFeed, SLOT(setHidden(bool)));
    connect(showNewsAction, SIGNAL(toggled(bool)), newsWidget, SLOT(toggleNews(bool)));

#ifdef ENABLE_UPDATERS
    connect(showNewsAction, SIGNAL(toggled(bool)), this, SLOT(slotToggleUpdateChecks(bool)));
#endif

#ifdef Q_OS_ANDROID
    // enabling this widgets crashes the app, so it is better for it to be hidden for now
    newsWidget->hide();
    helpTitleLabel->hide();
    btnNewsOptions->hide();

    donationLink = new QPushButton(dropFrameBorder);
    donationLink->setFlat(true);
    QFont f = font();
    f.setPointSize(15);
    f.setUnderline(true);
    donationLink->setFont(f);

    connect(donationLink, SIGNAL(clicked(bool)), this, SLOT(slotStartDonationFlow()));

    verticalLayout->addWidget(donationLink);
    verticalLayout->setAlignment(donationLink, Qt::AlignTop);
    verticalLayout->setSpacing(20);

    donationBannerImage = new QLabel(dropFrameBorder);
    QString bannerPath = QStandardPaths::locate(QStandardPaths::AppDataLocation, "share/krita/donation/banner.png");
    donationBannerImage->setPixmap(QPixmap(bannerPath));

    verticalLayout->addWidget(donationBannerImage);

    jboolean bannerPurchased = QAndroidJniObject::callStaticMethod<jboolean>("org/krita/android/DonationHelper", "isBadgePurchased", "()Z");
    if (bannerPurchased) {
        donationLink->hide();
        donationBannerImage->show();
        QAndroidJniObject::callStaticMethod<void>("org/krita/android/DonationHelper", "endConnection", "()V");
    } else {
        donationLink->show();
        donationBannerImage->hide();
    }

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

    showNewsAction->setChecked(m_checkUpdates);
    newsWidget->setVisible(m_checkUpdates);

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
    // only apply color to the widget itself, not to the tooltip or something
    blendedStyle = "QWidget{color: " + blendedColor.name() + "}";

    // what labels to change the color...
    startTitleLabel->setStyleSheet(blendedStyle);
    recentDocumentsLabel->setStyleSheet(blendedStyle);
    helpTitleLabel->setStyleSheet(blendedStyle);
    newsTitleLabel->setStyleSheet(blendedStyle);
    newFileLinkShortcut->setStyleSheet(blendedStyle);
    openFileShortcut->setStyleSheet(blendedStyle);
    clearRecentFilesLink->setStyleSheet(blendedStyle);
    recentDocumentsListView->setStyleSheet(blendedStyle);
    newsWidget->setStyleSheet(blendedStyle);

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

    btnNewsOptions->setIcon(KisIconUtils::loadIcon("configure"));

    supportKritaIcon->setIcon(KisIconUtils::loadIcon(QStringLiteral("support-krita")));
    const QIcon &linkIcon = KisIconUtils::loadIcon(QStringLiteral("bookmarks"));
    userManualIcon->setIcon(linkIcon);
    gettingStartedIcon->setIcon(linkIcon);
    userCommunityIcon->setIcon(linkIcon);
    kritaWebsiteIcon->setIcon(linkIcon);
    sourceCodeIcon->setIcon(linkIcon);

    kdeIcon->setIcon(KisIconUtils::loadIcon(QStringLiteral("kde")));

    // HTML links seem to be a bit more stubborn with theme changes... setting inline styles to help with color change
    userCommunityLink->setText(QString("<a style=\"color: " + blendedColor.name() + " \" href=\"https://krita-artists.org\">")
                               .append(i18n("User Community")).append("</a>"));

    gettingStartedLink->setText(QString("<a style=\"color: " + blendedColor.name() + " \" href=\"https://docs.krita.org/en/user_manual/getting_started.html\">")
                                .append(i18n("Getting Started")).append("</a>"));

    manualLink->setText(QString("<a style=\"color: " + blendedColor.name() + " \" href=\"https://docs.krita.org\">")
                        .append(i18n("User Manual")).append("</a>"));

    supportKritaLink->setText(QString("<a style=\"color: " + blendedColor.name() + " \" href=\"https://krita.org/en/support-us/donations?" + analyticsString + "donations" + "\">")
                              .append(i18n("Support Krita")).append("</a>"));

    kritaWebsiteLink->setText(QString("<a style=\"color: " + blendedColor.name() + " \" href=\"https://www.krita.org?" + analyticsString + "marketing-site" + "\">")
                              .append(i18n("Krita Website")).append("</a>"));

    sourceCodeLink->setText(QString("<a style=\"color: " + blendedColor.name() + " \" href=\"https://invent.kde.org/graphics/krita\">")
                            .append(i18n("Source Code")).append("</a>"));

    poweredByKDELink->setText(QString("<a style=\"color: " + blendedColor.name() + " \" href=\"https://userbase.kde.org/What_is_KDE\">")
                              .append(i18n("Powered by KDE")).append("</a>"));

    const QColor faintTextColor = KritaUtils::blendColors(textColor, backgroundColor, 0.4);
    const QString &faintTextStyle = "QWidget{color: " + faintTextColor.name() + "}";
    labelNoRecentDocs->setStyleSheet(faintTextStyle);
    labelNoFeed->setStyleSheet(faintTextStyle);

    const QColor frameColor = KritaUtils::blendColors(textColor, backgroundColor, 0.1);
    const QString &frameQss = "{border: 1px solid " + frameColor.name() + "}";
    recentDocsFrame->setStyleSheet("QFrame#recentDocsFrame" + frameQss);
    newsFrame->setStyleSheet("QFrame#newsFrame" + frameQss);

    // show the dev version labels, if dev version is detected
    showDevVersionHighlight();

#ifdef ENABLE_UPDATERS
    updateVersionUpdaterFrame(); // updater frame
#endif


#ifdef Q_OS_ANDROID
    donationLink->setStyleSheet(blendedStyle);
    donationLink->setText(QString(i18n("Get your Krita Supporter Badge here!")));
#endif
}

void KisWelcomePageWidget::populateRecentDocuments()
{
    constexpr const int maxItemsCount = 5;
    const int itemHeight = recentDocumentsListView->height() / maxItemsCount
            - recentDocumentsListView->spacing() * 2;

    const QList<QUrl> &recentFileUrls = m_mainWindow->recentFilesUrls();
    // grab recent files data
    int numRecentFiles = qMin(recentFileUrls.length(), maxItemsCount); // grab at most 5
    KisFileIconCreator iconCreator;
    QSize iconSize(itemHeight, itemHeight);
    QList<QUrl> brokenUrls;
    QList<QStandardItem *> items;

    for (int i = 0; i < numRecentFiles; i++) {
        QString recentFileUrlPath = recentFileUrls.at(i).toLocalFile();
        QIcon icon;

        if (m_thumbnailMap.contains(recentFileUrlPath)) {
            icon = m_thumbnailMap[recentFileUrlPath];
        } else {
            bool success = iconCreator.createFileIcon(recentFileUrlPath, icon, devicePixelRatioF(), iconSize);
            if (success) {
                m_thumbnailMap[recentFileUrlPath] = icon;
            } else {
                brokenUrls << recentFileUrls.at(i);
                continue;
            }
        }

        const QString &fileName = QFileInfo(recentFileUrlPath).fileName();
        QStandardItem *recentItem = new QStandardItem(icon, fileName);
        recentItem->setToolTip(recentFileUrlPath);
        items.append(recentItem);
    }

    for (const QUrl &url : brokenUrls) {
        m_mainWindow->removeRecentUrl(url);
    }

    // hide clear and Recent files title if there are none
    labelNoRecentDocs->setVisible(items.isEmpty());
    recentDocumentsListView->setVisible(!items.isEmpty());
    clearRecentFilesLink->setVisible(!items.isEmpty());

    m_recentFilesModel.clear(); // clear existing data before it gets re-populated
    for (QStandardItem *item : items) {
        m_recentFilesModel.appendRow(item);
    }

    recentDocumentsListView->setIconSize(iconSize);
    if (!recentItemDelegate) {
        recentItemDelegate = new RecentItemDelegate(this);
        recentDocumentsListView->setItemDelegate(recentItemDelegate);
    }
    recentItemDelegate->setItemHeight(itemHeight);
    recentDocumentsListView->setModel(&m_recentFilesModel);
}



#ifdef Q_OS_ANDROID
void KisWelcomePageWidget::slotStartDonationFlow()
{
    QAndroidJniObject::callStaticMethod<void>("org/krita/android/DonationHelper", "startBillingFlow", "()V");
}
#endif

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

bool KisWelcomePageWidget::eventFilter(QObject *watched, QEvent *event)
{
    if (watched == recentDocumentsListView && event->type() == QEvent::Leave) {
        recentDocumentsListView->clearSelection();
    }
    return QWidget::eventFilter(watched, event);
}

namespace {

QString mapKi18nLangToNewsLang(const QString &ki18nLang) {
    if (ki18nLang == "ja") {
        return QString("jp");
    }
    if (ki18nLang == "zh_CN") {
        return QString("zh");
    }
    if (ki18nLang == "zh_TW") {
        return QString("zh-tw");
    }
    if (ki18nLang == "zh_HK") {
        return QString("zh-hk");
    }
    if (ki18nLang == "en" || ki18nLang == "en_US" || ki18nLang == "en_GB") {
        return QString("en");
    }
    return QString();
};

QString getAutoNewsLang()
{
    // Get current UI languages:
    const QStringList uiLangs = KLocalizedString::languages();

    QString autoNewsLang;
    // Iterate UI languages including fallback languages.
    Q_FOREACH(const auto &uiLang, uiLangs) {
        autoNewsLang = mapKi18nLangToNewsLang(uiLang);
        if (autoNewsLang.size() <= 0) {
            break;
        }
    }
    if (autoNewsLang.size() == 0) {
        // If nothing else, use English.
        autoNewsLang = QString("en");
    }
    return autoNewsLang;
}

} /* namespace */

void KisWelcomePageWidget::setupNewsLangSelection(QMenu *newsOptionsMenu)
{
    // Hard-coded news language data:
    // These are languages in which the news items should be regularly
    // translated into as of 2020-11-07.
    // The language display names should not be translated. This reflects
    // the language selection box on the Krita website.
    struct Lang {
        const QString siteCode;
        const QString name;
    };
    static const std::array<Lang, 5> newsLangs = {{
        {QString("en"), QStringLiteral("English")},
        {QString("jp"), QStringLiteral("日本語")},
        {QString("zh"), QStringLiteral("中文 (简体)")},
        {QString("zh-tw"), QStringLiteral("中文 (台灣正體)")},
        {QString("zh-hk"), QStringLiteral("香港廣東話")},
    }};

    static const QString newsLangConfigName = QStringLiteral("FetchNewsLanguages");

    QSharedPointer<QSet<QString>> enabledNewsLangs = QSharedPointer<QSet<QString>>::create();
    {
        // Initialize with the config.
        KisConfig cfg(true);
        *enabledNewsLangs = cfg.readList<QString>(newsLangConfigName).toSet();
    }

    // If no languages are selected in the config, use the automatic selection.
    if (enabledNewsLangs->isEmpty()) {
        enabledNewsLangs->insert(QString(getAutoNewsLang()));
    }

    for (const auto &lang : newsLangs) {
        QAction *langItem = newsOptionsMenu->addAction(lang.name);
        langItem->setCheckable(true);
        // We can copy `code` into the lambda because its backing string is a
        // static string literal.
        const QString code = lang.siteCode;
        connect(langItem, &QAction::toggled, [=](bool checked) {
            newsWidget->toggleNewsLanguage(code, checked);
        });

        // Set the initial checked state.
        if (enabledNewsLangs->contains(code)) {
            langItem->setChecked(true);
        }

        // Connect this lambda after setting the initial checked state because
        // we don't want to overwrite the config when doing the initial setup.
        connect(langItem, &QAction::toggled, [=](bool checked) {
            KisConfig cfg(false);
            // It is safe to modify `enabledNewsLangs` here, because the slots
            // are called synchronously on the UI thread so there is no need
            // for explicit synchronization.
            if (checked) {
                enabledNewsLangs->insert(QString(code));
            } else {
                enabledNewsLangs->remove(QString(code));
            }
            cfg.writeList(newsLangConfigName, enabledNewsLangs->toList());
        });
    }
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
    QMessageBox::warning(qApp->activeWindow(), i18nc("@title:window", "Krita"), m_updaterStatus.updaterOutput());
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
#ifdef Q_OS_ANDROID
extern "C" JNIEXPORT void JNICALL
Java_org_krita_android_JNIWrappers_donationSuccessful(JNIEnv* /*env*/,
                                                      jobject /*obj*/,
                                                      jint    /*n*/)
{
    KisWelcomePageWidget::donationLink->hide();
    KisWelcomePageWidget::donationBannerImage->show();
}
#endif
