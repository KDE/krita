/*
 *  SPDX-FileCopyrightText: 2014 Boudewijn Rempt <boud@valdyas.org>
 *  SPDX-FileCopyrightText: 2021 Alvin Wong <alvin@alvinhc.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "kis_splash_screen.h"

#include <QApplication>
#include <QDesktopWidget>
#include <QGraphicsDropShadowEffect>
#include <QPixmap>
#include <QPainter>
#include <QCheckBox>
#include <kis_debug.h>
#include <QFile>
#include <QScreen>
#include <QWindow>
#include <QSvgWidget>

#include <KisPart.h>
#include <KisApplication.h>

#include <kis_icon.h>

#include <klocalizedstring.h>
#include <kconfig.h>
#include <ksharedconfig.h>
#include <kconfiggroup.h>
#include <QIcon>

static void addDropShadow(QWidget *widget)
{
    QGraphicsDropShadowEffect *effect = new QGraphicsDropShadowEffect(widget);
    effect->setBlurRadius(4);
    effect->setOffset(0.5);
    effect->setColor(QColor(0, 0, 0, 255));
    widget->setGraphicsEffect(effect);
}

KisSplashScreen::KisSplashScreen(bool themed, QWidget *parent, Qt::WindowFlags f)
    : QWidget(parent, Qt::SplashScreen | Qt::FramelessWindowHint
#ifdef Q_OS_LINUX
              | Qt::WindowStaysOnTopHint
#endif
              | f)
      , m_themed(themed)
      , m_versionHtml(qApp->applicationVersion().toHtmlEscaped())
{

    setupUi(this);
    setWindowIcon(KisIconUtils::loadIcon("krita-branding"));

    m_loadingTextLabel = new QLabel(lblSplash);
    m_loadingTextLabel->setTextFormat(Qt::RichText);
    m_loadingTextLabel->setStyleSheet(QStringLiteral("QLabel { color: #fff; background-color: transparent; }"));
    m_loadingTextLabel->setAlignment(Qt::AlignRight | Qt::AlignTop);
    addDropShadow(m_loadingTextLabel);

    m_brandingSvg = new QSvgWidget(QStringLiteral(":/krita-branding.svgz"), lblSplash);
    m_bannerSvg = new QSvgWidget(QStringLiteral(":/splash/banner.svg"), lblSplash);
    addDropShadow(m_bannerSvg);

    m_artCreditsLabel = new QLabel(lblSplash);
    m_artCreditsLabel->setTextFormat(Qt::PlainText);
    m_artCreditsLabel->setStyleSheet(QStringLiteral("QLabel { color: #fff; background-color: transparent; font: 10pt; }"));
    m_artCreditsLabel->setAlignment(Qt::AlignRight | Qt::AlignBottom);
    addDropShadow(m_artCreditsLabel);

    updateSplashImage();
    setLoadingText(QString());

    bnClose->hide();
    connect(bnClose, SIGNAL(clicked()), this, SLOT(close()));
    chkShowAtStartup->hide();
    connect(chkShowAtStartup, SIGNAL(toggled(bool)), this, SLOT(toggleShowAtStartup(bool)));

    KConfigGroup cfg( KSharedConfig::openConfig(), "SplashScreen");
    bool hideSplash = cfg.readEntry("HideSplashAfterStartup", false);
    chkShowAtStartup->setChecked(hideSplash);

    connect(lblRecent, SIGNAL(linkActivated(QString)), SLOT(linkClicked(QString)));
    connect(&m_timer, SIGNAL(timeout()), SLOT(raise()));

    // hide these labels by default
    displayLinks(false);
    displayRecentFiles(false);

    m_timer.setSingleShot(true);
    m_timer.start(10);
}

void KisSplashScreen::updateSplashImage()
{
    constexpr int SPLASH_HEIGHT_LOADING = 480;
    constexpr int SPLASH_HEIGHT_ABOUT = 320;

    int splashHeight;
    if (m_displayLinks) {
        splashHeight = SPLASH_HEIGHT_ABOUT;
    } else {
        splashHeight = SPLASH_HEIGHT_LOADING;
    }
    const int bannerHeight = splashHeight * 0.16875;
    const int marginTop = splashHeight * 0.05;
    const int marginRight = splashHeight * 0.1;

    QString splashName = QStringLiteral(":/splash/0.png");
    QString splashArtist = QStringLiteral("Tyson Tan");
    // TODO: Re-add the holiday splash...
#if 0
    QDate currentDate = QDate::currentDate();
    if (currentDate > QDate(currentDate.year(), 12, 4) ||
            currentDate < QDate(currentDate.year(), 1, 9)) {
        splashName = QStringLiteral(":/splash/1.png");
        splashArtist = QStringLiteral("???");
    }
#endif

    QPixmap img(splashName);

    // Preserve aspect ratio of splash.
    const int height = splashHeight;
    const int width = height * img.width() / img.height();

    setFixedWidth(width);
    setFixedHeight(height);
    lblSplash->setFixedWidth(width);
    lblSplash->setFixedHeight(height);

    // Get a downscaled pixmap of the splash.
    const int pixelWidth = width * devicePixelRatioF();
    const int pixelHeight = height * devicePixelRatioF();
    img = img.scaled(pixelWidth, pixelHeight, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);
    img.setDevicePixelRatio(devicePixelRatioF());
    lblSplash->setPixmap(img);

    // Align banner to top-left with margin.
    m_bannerSvg->setFixedHeight(bannerHeight);
    m_bannerSvg->setFixedWidth(bannerHeight * m_bannerSvg->sizeHint().width() / m_bannerSvg->sizeHint().height());
    m_bannerSvg->move(width - m_bannerSvg->width() - marginRight, marginTop);

    // Place logo to the left of banner.
    m_brandingSvg->setFixedSize(bannerHeight, bannerHeight);
    m_brandingSvg->move(m_bannerSvg->x() - m_brandingSvg->width(), marginTop);

    // Place loading text immediately below.
    m_loadingTextLabel->move(marginRight, m_brandingSvg->geometry().bottom());
    m_loadingTextLabel->setFixedWidth(m_bannerSvg->geometry().right() - marginRight);

    // Place credits text on bottom right with similar margins.
    if (splashArtist.isEmpty()) {
        m_artCreditsLabel->setText(QString());
    } else {
        m_artCreditsLabel->setText(i18nc("splash image credit", "Artwork by: %1", splashArtist));
    }
    m_artCreditsLabel->setFixedWidth(m_loadingTextLabel->width());
    m_artCreditsLabel->setFixedHeight(20);
    m_artCreditsLabel->move(m_loadingTextLabel->x(), height - marginTop - m_artCreditsLabel->height());

    if (m_displayLinks) {
        setFixedSize(sizeHint());
    }
}

void KisSplashScreen::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
    updateText();
}

void KisSplashScreen::updateText()
{
    QString color = colorString();

    KConfigGroup cfg2( KSharedConfig::openConfig(), "RecentFiles");
    int i = 1;

    QString recent = i18n("<html>"
                          "<head/>"
                          "<body>"
                          "<p><b><span style=\" color:%1;\">Recent Files</span></b></p>", color);

    QString path;
    QStringList recentfiles;

    QFontMetrics metrics(lblRecent->font());

    do {
        path = cfg2.readPathEntry(QString("File%1").arg(i), QString());
        if (!path.isEmpty()) {
            QString name = cfg2.readPathEntry(QString("Name%1").arg(i), QString());
            QUrl url(path);
            if (name.isEmpty()) {
                name = url.fileName();
            }

            name = metrics.elidedText(name, Qt::ElideMiddle, lblRecent->width());

            if (!url.isLocalFile() || QFile::exists(url.toLocalFile())) {
                recentfiles.insert(0, QString("<p><a href=\"%1\"><span style=\"color:%3;\">%2</span></a></p>").arg(path).arg(name).arg(color));
            }
        }

        i++;
    } while (!path.isEmpty() || i <= 8);

    recent += recentfiles.join("\n");
    recent += "</body>"
        "</html>";
    lblRecent->setText(recent);
}

void KisSplashScreen::displayLinks(bool show) {

    if (show) {
        QString color = colorString();
        lblLinks->setTextFormat(Qt::RichText);
        lblLinks->setText(i18n("<html>"
                               "<head/>"
                               "<body><table width=\"100%\" cellpadding=\"30\"><tr><td>"
                               "<p><span style=\" color:%1;\"><b>Using Krita</b></span></p>"
                               "<p><a href=\"https://krita.org/support-us/\"><span style=\" text-decoration: underline; color:%1;\">Support Krita's Development!</span></a></p>"
                               "<p><a href=\"https://docs.krita.org/en/user_manual/getting_started.html\"><span style=\" text-decoration: underline; color:%1;\">Getting Started</span></a></p>"
                               "<p><a href=\"https://docs.krita.org/\"><span style=\" text-decoration: underline; color:%1;\">Manual</span></a></p>"
                               "<p><a href=\"https://krita.org/\"><span style=\" text-decoration: underline; color:%1;\">Krita Website</span></a></p>"
                               "</td><td><p><span style=\" color:%1;\"><b>Coding Krita</b></span></p>"
                               "<p><a href=\"https://krita-artists.org\"><span style=\" text-decoration: underline; color:%1;\">User Community</span></a></p>"
                               "<p><a href=\"https://invent.kde.org/graphics/krita\"><span style=\" text-decoration: underline; color:%1;\">Source Code</span></a></p>"
                               "<p><a href=\"https://api.kde.org/appscomplete-api/krita-apidocs/libs/libkis/html/index.html\"><span style=\" text-decoration: underline; color:%1;\">Scripting API</span></a></p>"
                               "<p><a href=\"https://scripting.krita.org/lessons/introduction\"><span style=\" text-decoration: underline; color:%1;\">Scripting School</span></a></p>"
                               "</td></table></body>"
                               "</html>", color));

        filesLayout->setContentsMargins(10,10,10,10);
        actionControlsLayout->setContentsMargins(5,5,5,5);

    } else {
        // eliminating margins here allows for the splash screen image to take the entire area with nothing underneath
        filesLayout->setContentsMargins(0,0,0,0);
        actionControlsLayout->setContentsMargins(0,0,0,0);
    }

    lblLinks->setVisible(show);

    updateText();

    if (m_displayLinks != show) {
        m_displayLinks = show;
        updateSplashImage();
    }
}


void KisSplashScreen::displayRecentFiles(bool show) {
    lblRecent->setVisible(show);
    line->setVisible(show);
}

void KisSplashScreen::setLoadingText(QString text)
{
    int larger = 12;
    int notAsLarge = larger - 1;
    QString htmlText = QStringLiteral("<span style='font: %3pt;'><span style='font: bold %4pt;'>%1</span><br><i>%2</i></span>")
            .arg(m_versionHtml, text.toHtmlEscaped(), QString::number(notAsLarge), QString::number(larger));
    m_loadingTextLabel->setText(htmlText);
}



QString KisSplashScreen::colorString() const
{
    QString color = "#FFFFFF";
    if (m_themed && qApp->palette().window().color().value() > 100) {
        color = "#000000";
    }

    return color;
}


void KisSplashScreen::repaint()
{
    QWidget::repaint();
    qApp->sendPostedEvents();
}

void KisSplashScreen::show()
{
    if (!this->parentWidget()) {
        this->winId(); // Force creation of native window
        QWindow *windowHandle = this->windowHandle();
        QScreen *screen = windowHandle ? windowHandle->screen() : nullptr;
        if (windowHandle && !screen) {
            // At least on Windows, the window may be created on a non-primary
            // screen with a different scale factor. If we don't explicitly
            // move it to the primary screen, the position will be scaled with
            // the wrong factor and the splash will be offset.
            // XXX: In theory this branch should be unreachable, but leaving
            //      this here just in case.
            windowHandle->setScreen(QApplication::primaryScreen());
        }
        if (!screen) {
            screen = QApplication::primaryScreen();
        }
        // Reinitialize the splash image as the screen may have a different
        // devicePixelRatio.
        updateSplashImage();
        QRect r(QPoint(), size());
        move(screen->availableGeometry().center() - r.center());
    }
    if (isVisible()) {
        repaint();
    }
    m_timer.setSingleShot(true);
    m_timer.start(1);
    QWidget::show();
}

void KisSplashScreen::toggleShowAtStartup(bool toggle)
{
    KConfigGroup cfg( KSharedConfig::openConfig(), "SplashScreen");
    cfg.writeEntry("HideSplashAfterStartup", toggle);
}

void KisSplashScreen::linkClicked(const QString &link)
{
    KisPart::instance()->openExistingFile(link);
    if (isTopLevel()) {
        close();
    }
}
