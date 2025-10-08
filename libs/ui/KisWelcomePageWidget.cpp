
/* This file is part of the KDE project
 * SPDX-FileCopyrightText: 2018 Scott Petrovic <scottpetrovic@gmail.com>
 * SPDX-FileCopyrightText: 2021 L. E. Segovia <amy@amyspark.me>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include "KisWelcomePageWidget.h"
#include "KisRecentDocumentsModelWrapper.h"
#include <QDesktopServices>
#include <QFileInfo>
#include <QMimeData>
#include <QPixmap>
#include <QImage>
#include <QMessageBox>
#include <QTemporaryFile>
#include <QByteArray>
#include <QBuffer>
#include <QNetworkAccessManager>
#include <QEventLoop>
#include <QDomDocument>

#include "KisRemoteFileFetcher.h"
#include "kactioncollection.h"
#include "kis_action.h"
#include "kis_action_manager.h"
#include <KisMimeDatabase.h>
#include <KisApplication.h>

#include "KConfigGroup"
#include "KSharedConfig"

#include <QListWidget>
#include <QListWidgetItem>
#include <QMenu>
#include <QScrollBar>

#include "kis_icon_utils.h"
#include <kis_painting_tweaks.h>
#include "KoStore.h"
#include "kis_config.h"
#include "KisDocument.h"
#include <kis_image.h>
#include <kis_paint_device.h>
#include <KisPart.h>
#include <KisKineticScroller.h>
#include "KisMainWindow.h"

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
#include <QSysInfo>
#include <kis_config.h>
#include <kis_image_config.h>
#include "opengl/kis_opengl.h"

#ifdef Q_OS_WIN
#include <KisWindowsPackageUtils.h>
#endif

#ifdef Q_OS_MACOS
#include "libs/macosutils/KisMacosEntitlements.h"
#endif

#ifdef Q_OS_ANDROID
#include "KisAndroidDonations.h"
#endif

// Used for triggering a QAction::setChecked signal from a QLabel::linkActivated signal
void ShowNewsAction::enableFromLink(QString unused_url)
{
    Q_UNUSED(unused_url);
    Q_EMIT setChecked(true);
}


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

    // URLs that go to web browser...
    devBuildIcon->setIcon(KisIconUtils::loadIcon("warning"));
    devBuildLabel->setVisible(false);
    updaterFrame->setVisible(false);
    versionNotificationLabel->setVisible(false);
    bnVersionUpdate->setVisible(false);
    bnErrorDetails->setVisible(false);

    // Recent docs...
    recentDocumentsListView->setDragEnabled(false);
    recentDocumentsListView->viewport()->setAutoFillBackground(false);
    recentDocumentsListView->setSpacing(2);
    recentDocumentsListView->installEventFilter(this);
    recentDocumentsListView->setViewMode(QListView::IconMode);
    recentDocumentsListView->setSelectionMode(QAbstractItemView::NoSelection);

//    m_recentItemDelegate.reset(new RecentItemDelegate(this));
//    m_recentItemDelegate->setItemHeight(KisRecentDocumentsModelWrapper::ICON_SIZE_LENGTH);
//    recentDocumentsListView->setItemDelegate(m_recentItemDelegate.data());
    recentDocumentsListView->setIconSize(QSize(KisRecentDocumentsModelWrapper::ICON_SIZE_LENGTH, KisRecentDocumentsModelWrapper::ICON_SIZE_LENGTH));
    recentDocumentsListView->setVerticalScrollMode(QListView::ScrollPerPixel);
    recentDocumentsListView->verticalScrollBar()->setSingleStep(50);
    {
        QScroller* scroller = KisKineticScroller::createPreconfiguredScroller(recentDocumentsListView);
        if (scroller) {
            connect(scroller, SIGNAL(stateChanged(QScroller::State)), this, SLOT(slotScrollerStateChanged(QScroller::State)));
        }
    }
    recentDocumentsListView->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(recentDocumentsListView, SIGNAL(customContextMenuRequested(QPoint)), SLOT(slotRecentDocContextMenuRequest(QPoint)));

    // News widget...
    QMenu *newsOptionsMenu = new QMenu(this);
    newsOptionsMenu->setToolTipsVisible(true);
    ShowNewsAction *showNewsAction = new ShowNewsAction(i18n("Enable news and check for new releases"), newsOptionsMenu);
    newsOptionsMenu->addAction(showNewsAction);
    showNewsAction->setToolTip(i18n("Show news about Krita: this needs internet to retrieve information from the krita.org website"));
    showNewsAction->setCheckable(true);

    newsOptionsMenu->addSection(i18n("Language"));
    QAction *newsInfoAction = newsOptionsMenu->addAction(i18n("English news is always up to date."));
    newsInfoAction->setEnabled(false);

    setupNewsLangSelection(newsOptionsMenu);
    btnNewsOptions->setMenu(newsOptionsMenu);

    labelSupportText->setFont(largerFont());
    donationLink->setFont(largerFont());

    connect(showNewsAction, SIGNAL(toggled(bool)), newsWidget, SLOT(setVisible(bool)));
    connect(showNewsAction, SIGNAL(toggled(bool)), labelNoFeed, SLOT(setHidden(bool)));
    connect(showNewsAction, SIGNAL(toggled(bool)), newsWidget, SLOT(toggleNews(bool)));
    connect(labelNoFeed, SIGNAL(linkActivated(QString)), showNewsAction, SLOT(enableFromLink(QString)));

    labelNoFeed->setDismissable(false);

#ifdef ENABLE_UPDATERS
    connect(showNewsAction, SIGNAL(toggled(bool)), this, SLOT(slotToggleUpdateChecks(bool)));
#endif

    donationLink->hide();
    supporterBadge->hide();
#ifdef Q_OS_ANDROID
    initDonations();
#endif

    // configure the News area
    KisConfig cfg(true);
    m_networkIsAllowed = cfg.readEntry<bool>("FetchNews", false);


#ifdef ENABLE_UPDATERS
#ifndef Q_OS_ANDROID
    // Setup version updater, but do not check for them, unless the user explicitly
    // wants to check for updates.
    // * No updater is created for Linux/Steam, Windows/Steam and Windows/Store distributions,
    // as those stores have their own updating mechanism.
    // * STEAMAPPID(Windows)/SteamAppId(Linux) environment variable is set when Krita is run from Steam.
    // The environment variables are not public API.
    // * MS Store version runs as a package (though we cannot know if it was
    // installed from the Store or manually with the .msix package)
#if defined Q_OS_LINUX
    if (!qEnvironmentVariableIsSet("SteamAppId")) { // do not create updater for linux/steam
        if (qEnvironmentVariableIsSet("APPIMAGE")) {
            m_versionUpdater.reset(new KisAppimageUpdater());
        } else {
            m_versionUpdater.reset(new KisManualUpdater());
        }
    }
#elif defined Q_OS_WIN
    if (!KisWindowsPackageUtils::isRunningInPackage() && !qEnvironmentVariableIsSet("STEAMAPPID")) {
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

        if (m_networkIsAllowed) { // only if the user wants them
			m_versionUpdater->checkForUpdate();
		}
	}
#endif // ifndef Q_OS_ANDROID
#endif // ENABLE_UPDATERS


    showNewsAction->setChecked(m_networkIsAllowed);
    newsWidget->setVisible(m_networkIsAllowed);
    versionNotificationLabel->setEnabled(m_networkIsAllowed);

    // Drop area..
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

        KisRecentDocumentsModelWrapper *recentFilesModel = KisRecentDocumentsModelWrapper::instance();
        connect(recentFilesModel, SIGNAL(sigModelIsUpToDate()), this, SLOT(slotRecentFilesModelIsUpToDate()));
        recentDocumentsListView->setModel(&recentFilesModel->model());
        slotRecentFilesModelIsUpToDate();
    }
}


void KisWelcomePageWidget::showDropAreaIndicator(bool show)
{
    if (!show) {
        QString dropFrameStyle = QStringLiteral("QFrame#dropAreaIndicator { border: 2px solid transparent }");
        dropFrameBorder->setStyleSheet(dropFrameStyle);
    } else {
        QColor textColor = qApp->palette().color(QPalette::Text);
        QColor backgroundColor = qApp->palette().color(QPalette::Window);
        QColor blendedColor = KisPaintingTweaks::blendColors(textColor, backgroundColor, 0.8);

        // QColor.name() turns it into a hex/web format
        QString dropFrameStyle = QString("QFrame#dropAreaIndicator { border: 2px dotted ").append(blendedColor.name()).append(" }") ;
        dropFrameBorder->setStyleSheet(dropFrameStyle);
    }
}

void KisWelcomePageWidget::slotUpdateThemeColors()
{
    textColor = qApp->palette().color(QPalette::Text);
    backgroundColor = qApp->palette().color(QPalette::Window);

    // make the welcome screen labels a subtle color so it doesn't clash with the main UI elements
    blendedColor = KisPaintingTweaks::blendColors(textColor, backgroundColor, 0.8);
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

#ifdef Q_OS_ANDROID
    blendedStyle = blendedStyle + "\nQPushButton { padding: 10px }";
#endif

    newFileLink->setStyleSheet(blendedStyle);
    openFileLink->setStyleSheet(blendedStyle);

    // make drop area QFrame have a dotted line
    dropFrameBorder->setObjectName("dropAreaIndicator");
    QString dropFrameStyle = QString("QFrame#dropAreaIndicator { border: 4px dotted ").append(blendedColor.name()).append("}");
    dropFrameBorder->setStyleSheet(dropFrameStyle);

    // only show drop area when we have a document over the empty area
    showDropAreaIndicator(false);

    // add icons for new and open settings to make them stand out a bit more
    openFileLink->setIconSize(QSize(48, 48));
    newFileLink->setIconSize(QSize(48, 48));

    openFileLink->setIcon(KisIconUtils::loadIcon("document-open"));
    newFileLink->setIcon(KisIconUtils::loadIcon("document-new"));

    btnNewsOptions->setIcon(KisIconUtils::loadIcon("view-choose"));
    btnNewsOptions->setFlat(true);

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

    gettingStartedLink->setText(QString("<a style=\"color: " + blendedColor.name() + " \" href=\"https://docs.krita.org/user_manual/getting_started.html\">")
                                .append(i18n("Getting Started")).append("</a>"));

    manualLink->setText(QString("<a style=\"color: " + blendedColor.name() + " \" href=\"https://docs.krita.org\">")
                        .append(i18n("User Manual")).append("</a>"));

    supportKritaLink->setText(QString("<a style=\"color: " + blendedColor.name() + " \" href=\"https://krita.org/support-us/donations?" + analyticsString + "donations" + "\">")
                              .append(i18n("Support Krita")).append("</a>"));

    kritaWebsiteLink->setText(QString("<a style=\"color: " + blendedColor.name() + " \" href=\"https://www.krita.org?" + analyticsString + "marketing-site" + "\">")
                              .append(i18n("Krita Website")).append("</a>"));

    sourceCodeLink->setText(QString("<a style=\"color: " + blendedColor.name() + " \" href=\"https://invent.kde.org/graphics/krita\">")
                            .append(i18n("Source Code")).append("</a>"));

    poweredByKDELink->setText(QString("<a style=\"color: " + blendedColor.name() + " \" href=\"https://userbase.kde.org/What_is_KDE\">")
                              .append(i18n("Powered by KDE")).append("</a>"));

    QString translationNoFeed = i18n("You can <a href=\"ignored\" style=\"color: COLOR_PLACEHOLDER; text-decoration: underline;\">enable news</a> from krita.org in various languages with the menu above");
    labelNoFeed->setText(translationNoFeed.replace("COLOR_PLACEHOLDER", blendedColor.name()));

    const QColor faintTextColor = KisPaintingTweaks::blendColors(textColor, backgroundColor, 0.4);
    const QString &faintTextStyle = "QWidget{color: " + faintTextColor.name() + "}";
    labelNoRecentDocs->setStyleSheet(faintTextStyle);
    labelNoFeed->setStyleSheet(faintTextStyle);

    const QColor frameColor = KisPaintingTweaks::blendColors(textColor, backgroundColor, 0.1);
    const QString &frameQss = "{border: 1px solid " + frameColor.name() + "}";
    recentDocsStackedWidget->setStyleSheet("QStackedWidget#recentDocsStackedWidget" + frameQss);
    newsFrame->setStyleSheet("QFrame#newsFrame" + frameQss);

    // show the dev version labels, if dev version is detected
    showDevVersionHighlight();

#ifdef ENABLE_UPDATERS
    updateVersionUpdaterFrame(); // updater frame
#endif

#ifdef Q_OS_ANDROID
    donationLink->setText(
        QStringLiteral("<a href=\"#\">%1</a>").arg(QString(i18n("Get your Krita Supporter Badge here!"))));
#endif

#ifdef Q_OS_MACOS
    // macOS store version should not contain external links containing donation buttons or forms
    if (KisMacosEntitlements().sandbox()) {
        supportKritaLink->hide();
        supportKritaIcon->hide();
        labelSupportText->hide();
        kritaWebsiteLink->hide();
        kritaWebsiteIcon->hide();
        donationLink->hide();
    }
#endif
}

void KisWelcomePageWidget::dragEnterEvent(QDragEnterEvent *event)
{
    showDropAreaIndicator(true);
    if (event->mimeData()->hasUrls() ||
        event->mimeData()->hasFormat("application/x-krita-node-internal-pointer") ||
        event->mimeData()->hasFormat("application/x-qt-image")) {
        return event->accept();
    }

    return event->ignore();
}

void KisWelcomePageWidget::dropEvent(QDropEvent *event)
{
    showDropAreaIndicator(false);

    if (event->mimeData()->hasUrls() && !event->mimeData()->urls().empty()) {
        Q_FOREACH (const QUrl &url, event->mimeData()->urls()) {
            if (url.toLocalFile().endsWith(".bundle", Qt::CaseInsensitive)) {
                bool r = m_mainWindow->installBundle(url.toLocalFile());
                if (!r) {
                    qWarning() << "Could not install bundle" << url.toLocalFile();
                }
            } else if (!url.isLocalFile()) {
                QScopedPointer<QTemporaryFile> tmp(new QTemporaryFile());
                tmp->setFileName(url.fileName());

                KisRemoteFileFetcher fetcher;

                if (!fetcher.fetchFile(url, tmp.data())) {
                    qWarning() << "Fetching" << url << "failed";
                    continue;
                }
                const auto localUrl = QUrl::fromLocalFile(tmp->fileName());

                m_mainWindow->openDocument(localUrl.toLocalFile(), KisMainWindow::None);
            } else {
                m_mainWindow->openDocument(url.toLocalFile(), KisMainWindow::None);
            }
        }
    }
}

void KisWelcomePageWidget::dragMoveEvent(QDragMoveEvent *event)
{
    m_mainWindow->dragMoveEvent(event);

    if (event->mimeData()->hasUrls() ||
        event->mimeData()->hasFormat("application/x-krita-node-internal-pointer") ||
        event->mimeData()->hasFormat("application/x-qt-image")) {
        return event->accept();
    }

    return event->ignore();
}

void KisWelcomePageWidget::dragLeaveEvent(QDragLeaveEvent */*event*/)
{
    showDropAreaIndicator(false);
    m_mainWindow->dragLeave();
}

void KisWelcomePageWidget::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::FontChange) {
        labelSupportText->setFont(largerFont());
        donationLink->setFont(largerFont());
    }
}

bool KisWelcomePageWidget::eventFilter(QObject *watched, QEvent *event)
{
    if (watched == recentDocumentsListView && event->type() == QEvent::Leave) {
        recentDocumentsListView->clearSelection();
    }
    return QWidget::eventFilter(watched, event);
}

namespace {

QString getAutoNewsLang()
{
    // Get current UI languages:
    const QStringList uiLangs = KLocalizedString::languages();
    QString autoNewsLang = uiLangs.first();
    if (autoNewsLang.isEmpty()) {
        // If nothing else, use English.
        autoNewsLang = QString("en");
    } else if (autoNewsLang == "ja") {
        return QString("jp");
    } else if (autoNewsLang == "zh_CN") {
        return QString("zh");
    } else if (autoNewsLang == "zh_TW") {
        return QString("zh-tw");
    } else if (autoNewsLang == "zh_HK") {
        return QString("zh-hk");
    } else if (autoNewsLang == "en" || autoNewsLang == "en_US" || autoNewsLang == "en_GB") {
        return QString("en");
    }

    return autoNewsLang;
}

} /* namespace */

void KisWelcomePageWidget::setupNewsLangSelection(QMenu *newsOptionsMenu)
{
    // Hard-coded news language data:
    // These are languages in which the news items should be regularly
    // translated into as of 04-09-2024.
    // The language display names should not be translated. This reflects
    // the language selection box on the Krita website.
    struct Lang {
        const QString siteCode;
        const QString name;
    };
    static const std::array<Lang, 22> newsLangs = {{
        {QString("en"), QStringLiteral("English")},
        {QString("jp"), QStringLiteral("日本語")},
        {QString("zh"), QStringLiteral("中文 (简体)")},
        {QString("zh-tw"), QStringLiteral("中文 (台灣正體)")},
        {QString("zh-hk"), QStringLiteral("廣東話 (香港)")},
        {QString("ca"), QStringLiteral("Català")},
        {QString("ca@valencia"), QStringLiteral("Català de Valencia")},
        {QString("cs"), QStringLiteral("Čeština")},
        {QString("de"), QStringLiteral("Deutsch")},
        {QString("eo"), QStringLiteral("Esperanto")},
        {QString("es"), QStringLiteral("Español")},
        {QString("eu"), QStringLiteral("Euskara")},
        {QString("fr"), QStringLiteral("Français")},
        {QString("it"), QStringLiteral("Italiano")},
        {QString("lt"), QStringLiteral("lietuvių")},
        {QString("nl"), QStringLiteral("Nederlands")},
        {QString("pt"), QStringLiteral("Português")},
        {QString("sk"), QStringLiteral("Slovenský")},
        {QString("sl"), QStringLiteral("Slovenski")},
        {QString("sv"), QStringLiteral("Svenska")},
        {QString("tr"), QStringLiteral("Türkçe")},
        {QString("uk"), QStringLiteral("Українська")}
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
        connect(langItem, &QAction::toggled, newsWidget, [=](bool checked) {
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
            cfg.writeList(newsLangConfigName, enabledNewsLangs->values());
        });
    }
}

void KisWelcomePageWidget::showDevVersionHighlight()
{
    // always flag development version
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
    m_mainWindow->openDocument(fileUrl, KisMainWindow::None );
}

void KisWelcomePageWidget::slotRecentDocContextMenuRequest(const QPoint &pos)
{
    QMenu contextMenu;
    QModelIndex index = recentDocumentsListView->indexAt(pos);
    QAction *actionForget = 0;
    if (index.isValid()) {
        actionForget = new QAction(i18n("Forget \"%1\"", index.data(Qt::DisplayRole).toString()), &contextMenu);
        contextMenu.addAction(actionForget);
    }
    QAction *triggered = contextMenu.exec(recentDocumentsListView->mapToGlobal(pos));

    if (index.isValid() && triggered == actionForget) {
        m_mainWindow->removeRecentFile(index.data(Qt::ToolTipRole).toString());
    }
}

bool KisWelcomePageWidget::isDevelopmentBuild()
{
    return KritaVersionWrapper::isDevelopersBuild();
}

void KisWelcomePageWidget::slotNewFileClicked()
{
    m_mainWindow->slotFileNew();
}

void KisWelcomePageWidget::slotOpenFileClicked()
{
    m_mainWindow->slotFileOpen();
}

void KisWelcomePageWidget::slotRecentFilesModelIsUpToDate()
{
    KisRecentDocumentsModelWrapper *recentFilesModel = KisRecentDocumentsModelWrapper::instance();
    const bool modelIsEmpty = recentFilesModel->model().rowCount() == 0;

    if (modelIsEmpty) {
        recentDocsStackedWidget->setCurrentWidget(labelNoRecentDocs);
    } else {
        recentDocsStackedWidget->setCurrentWidget(recentDocumentsListView);
    }
    clearRecentFilesLink->setVisible(!modelIsEmpty);
}

#ifdef ENABLE_UPDATERS
void KisWelcomePageWidget::slotToggleUpdateChecks(bool state)
{
	if (m_versionUpdater.isNull()) {
		return;
	}

    m_networkIsAllowed = state;

    if (m_networkIsAllowed) {
        m_versionUpdater->checkForUpdate();
    }

    updateVersionUpdaterFrame();
}
void KisWelcomePageWidget::slotRunVersionUpdate()
{
	if (m_versionUpdater.isNull()) {
		return;
	}

    if (m_networkIsAllowed) {
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

    if (!m_networkIsAllowed || m_versionUpdater.isNull()) {
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
void KisWelcomePageWidget::initDonations()
{
    KisAndroidDonations *androidDonations = KisAndroidDonations::instance();
    if (!androidDonations) {
        qWarning("KisWelcomePage::initDonations: androidDonations is null");
        return;
    }

    connect(donationLink, SIGNAL(linkActivated(QString)), androidDonations, SLOT(slotStartDonationFlow()));

    QString bannerPath = QStandardPaths::locate(QStandardPaths::AppDataLocation, "share/krita/donation/banner.png");
    QPixmap pixmap(bannerPath);
    if (pixmap.isNull()) {
        qWarning("KisWelcomePage::initDonations: failed to load banner from '%s'", qUtf8Printable(bannerPath));
    } else {
        supporterBadge->setPixmap(QPixmap(bannerPath));
    }

    connect(androidDonations, SIGNAL(sigStateChanged()), this, SLOT(slotUpdateDonationState()));
    slotUpdateDonationState();
}

void KisWelcomePageWidget::slotUpdateDonationState()
{
    bool linkVisible = false;
    bool badgeVisible = false;

    KisAndroidDonations *androidDonations = KisAndroidDonations::instance();
    if (androidDonations) {
        linkVisible = androidDonations->shouldShowDonationLink();
        badgeVisible = androidDonations->shouldShowSupporterBadge();
    } else {
        qWarning("KisWelcomePageWidget::slotUpdateDonationState: android donations is null");
    }

    donationLink->setVisible(linkVisible);
    supporterBadge->setVisible(badgeVisible);
}
#endif

QFont KisWelcomePageWidget::largerFont()
{
    QFont larger = font();
    larger.setPointSizeF(larger.pointSizeF() * 1.1f);
    return larger;
}
