/*
 *  SPDX-FileCopyrightText: 2014 Boudewijn Rempt <boud@valdyas.org>
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

#include <KisPart.h>
#include <KisApplication.h>

#include <kis_icon.h>

#include <klocalizedstring.h>
#include <kconfig.h>
#include <ksharedconfig.h>
#include <kconfiggroup.h>
#include <QIcon>

KisSplashScreen::KisSplashScreen(const QString &version, const QPixmap &pixmap, const QPixmap &pixmap_x2, bool themed, QWidget *parent, Qt::WindowFlags f)
    : QWidget(parent, Qt::SplashScreen | Qt::FramelessWindowHint
#ifdef Q_OS_LINUX
              | Qt::WindowStaysOnTopHint
#endif
              | f)
      , m_themed(themed)
      , m_splashPixmap(pixmap)
      , m_splashPixmap_x2(pixmap_x2)
      , m_versionHtml(version.toHtmlEscaped())
{

    setupUi(this);
    setWindowIcon(KisIconUtils::loadIcon("krita"));

    m_loadingTextLabel = new QLabel(lblSplash);
    m_loadingTextLabel->setTextFormat(Qt::RichText);
    m_loadingTextLabel->setStyleSheet("QLabel { color: #fff; background-color: transparent; font-size: 10pt; }");
    m_loadingTextLabel->setAlignment(Qt::AlignRight | Qt::AlignTop);

    QGraphicsDropShadowEffect *effect = new QGraphicsDropShadowEffect(m_loadingTextLabel);
    effect->setOffset(1);
    m_loadingTextLabel->setGraphicsEffect(effect);

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
    QPixmap img;

    if (devicePixelRatioF() > 1.01) {
        img = m_splashPixmap_x2;
        img.setDevicePixelRatio(devicePixelRatioF());

        // actual size : image size (x1)
        m_scaleFactor = 2 / devicePixelRatioF();
    } else {
        img = m_splashPixmap;
        m_scaleFactor = 1;
    }

    setFixedWidth(m_splashPixmap.width() * m_scaleFactor);
    setFixedHeight(m_splashPixmap.height() * m_scaleFactor);
    lblSplash->setFixedWidth(m_splashPixmap.width() * m_scaleFactor);
    lblSplash->setFixedHeight(m_splashPixmap.height() * m_scaleFactor);

    // Maintain the aspect ratio on high DPI screens when scaling
    lblSplash->setPixmap(img);

    m_loadingTextLabel->move(0, 58 * m_scaleFactor);
    m_loadingTextLabel->setFixedWidth(475 * m_scaleFactor);
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
                               "<body>"
                               "<p><span style=\" color:%1;\"><b>Links</b></span></p>"

                               "<p><a href=\"https://krita.org/support-us/\"><span style=\" text-decoration: underline; color:%1;\">Support Krita</span></a></p>"

                               "<p><a href=\"https://docs.krita.org/en/user_manual/getting_started.html\"><span style=\" text-decoration: underline; color:%1;\">Getting Started</span></a></p>"
                               "<p><a href=\"https://docs.krita.org/\"><span style=\" text-decoration: underline; color:%1;\">Manual</span></a></p>"
                               "<p><a href=\"https://krita.org/\"><span style=\" text-decoration: underline; color:%1;\">Krita Website</span></a></p>"
                               "<p><a href=\"https://forum.kde.org/viewforum.php?f=136\"><span style=\" text-decoration: underline; color:%1;\">User Community</span></a></p>"

                               "<p><a href=\"https://phabricator.kde.org/source/krita/\"><span style=\" text-decoration: underline; color:%1;\">Source Code</span></a></p>"

                               "</body>"
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
}


void KisSplashScreen::displayRecentFiles(bool show) {
    lblRecent->setVisible(show);
    line->setVisible(show);
}

void KisSplashScreen::setLoadingText(QString text)
{
    QString htmlText = QStringLiteral("<span style='font-size: 11pt'><b>%1</b></span><br><i>%2</i>").arg(m_versionHtml, text.toHtmlEscaped());
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
