/*
 *  SPDX-FileCopyrightText: 2014 Boudewijn Rempt <boud@valdyas.org>
 *  SPDX-FileCopyrightText: 2021 Alvin Wong <alvin@alvinhc.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "kis_splash_screen.h"

#include <QApplication>
#include <QScreen>
#include <QGraphicsDropShadowEffect>
#include <QPixmap>
#include <QPainter>
#include <QCheckBox>
#include <kis_debug.h>
#include <QFile>
#include <QWindow>
#include <QSvgWidget>
#include <QVBoxLayout>
#include <QLabel>

#include <KisPart.h>
#include <KisApplication.h>

#include <kis_icon.h>
#include <kis_config.h>

#include <klocalizedstring.h>

#ifdef Q_OS_ANDROID
#include <KisAndroidDonations.h>
#endif

static void addDropShadow(QWidget *widget)
{
    QGraphicsDropShadowEffect *effect = new QGraphicsDropShadowEffect(widget);
    effect->setBlurRadius(4);
    effect->setOffset(0.5);
    effect->setColor(QColor(0, 0, 0, 255));
    widget->setGraphicsEffect(effect);
}

QString KisSplashScreen::Source::description() const
{
    QString text = title;
    if (isAvailable) {
        if (isDefault) {
            return i18nc("Splash image default", "%1 (default)", text);
        } else {
            return text;
        }
    } else {
        return i18nc("Splash image unavailable", "%1 (not available)", text);
    }
}

QString KisSplashScreen::Source::artistCredit() const
{
    if (artistName.isEmpty()) {
        return QString();
    } else {
        return i18nc("splash image credit", "Artwork by: %1", artistName);
    }
}

KisSplashScreen::KisSplashScreen(int height, QWidget *parent, Qt::WindowFlags f)
    : QWidget(parent, f)
      , m_versionHtml(qApp->applicationVersion().toHtmlEscaped())
{
    setWindowTitle(i18n("Krita"));
#ifndef Q_OS_MACOS
    setWindowIcon(KisIconUtils::loadIcon("krita-branding"));
#endif

    setLayout(new QVBoxLayout());
    layout()->setContentsMargins(QMargins());
    m_lblSplash = new QLabel();
    m_lblSplash->setFrameShape(QFrame::NoFrame);
    layout()->addWidget(m_lblSplash);

    m_loadingTextLabel = new QLabel(m_lblSplash);
    m_loadingTextLabel->setTextFormat(Qt::RichText);
    m_loadingTextLabel->setStyleSheet(QStringLiteral("QLabel { color: #fff; background-color: transparent; }"));
    m_loadingTextLabel->setAlignment(Qt::AlignRight | Qt::AlignTop);
    addDropShadow(m_loadingTextLabel);

    m_brandingSvg = new QSvgWidget(QStringLiteral(":/krita-branding.svgz"), m_lblSplash);
    m_bannerSvg = new QSvgWidget(QStringLiteral(":/splash/banner.svg"), m_lblSplash);
    addDropShadow(m_bannerSvg);

    m_artCreditsLabel = new QLabel(m_lblSplash);
    m_artCreditsLabel->setTextFormat(Qt::PlainText);
    m_artCreditsLabel->setStyleSheet(QStringLiteral("QLabel { color: #fff; background-color: transparent; font: 10pt; }"));
    m_artCreditsLabel->setAlignment(Qt::AlignRight | Qt::AlignBottom);
    addDropShadow(m_artCreditsLabel);

    updateSplashImage(height);
    setLoadingText(QString()); // the version string is set here too

    connect(&m_timer, SIGNAL(timeout()), SLOT(raise()));

    m_timer.setSingleShot(true);
    m_timer.start(10);
}

void KisSplashScreen::updateSplashImage(const int height)
{
    const int bannerHeight = height * 0.16875;
    const int marginTop = height * 0.05;
    const int marginRight = height * 0.1;

    Source source = getCurrentSource();
    QPixmap img(source.resourcePath);

    if (img.isNull() || img.height() == 0) return;

    // Preserve aspect ratio of splash.
    const int width = height * img.width() / img.height();

    setFixedSize(width, height);
    m_lblSplash->setFixedSize(width, height);

    // Let scaledContents downscale to the fixedSize
    m_lblSplash->setScaledContents(true);
    m_lblSplash->setPixmap(img);

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
    m_artCreditsLabel->setText(source.artistCredit());
    m_artCreditsLabel->setFixedWidth(m_loadingTextLabel->width());
    m_artCreditsLabel->setFixedHeight(20);
    m_artCreditsLabel->move(m_loadingTextLabel->x(), height - marginTop - m_artCreditsLabel->height());
}

void KisSplashScreen::setLoadingText(QString text)
{
    int larger = 12;
    int notAsLarge = larger - 1;
    QString htmlText = QStringLiteral("<span style='font: %3pt;'><span style='font: bold %4pt;'>%1</span><br><i>%2</i></span>")
            .arg(m_versionHtml, text.toHtmlEscaped(), QString::number(notAsLarge), QString::number(larger));
    m_loadingTextLabel->setText(htmlText);
}

KisSplashScreen::Source KisSplashScreen::getCurrentSource()
{
    return getAvailableSource(KisConfig(true).splashScreenImage());
}

KisSplashScreen::Source KisSplashScreen::getAvailableSource(const QString &key)
{
    QVector<Source> sources = getSources();
    for (const Source &source : sources) {
        if (source.isAvailable && source.key == key) {
            return source;
        }
    }
    return sources[0];
}

bool KisSplashScreen::canChooseSplashScreen()
{
    // The "professional" splash screen is available on purchased versions of
    // Krita, since professionals can pay for their tools. On Android, we'll
    // err on the side of allowing changing the splash screen unless we have
    // evidence to the contrary, otherwise a bad internet connection could
    // show the user the wrong splash screen. KisAndroidDonations will reset
    // the splash screen setting if it sees the user is not a supporter.
#ifdef Q_OS_ANDROID
    KisAndroidDonations *donations = KisAndroidDonations::instance();
    return !donations || donations->isMaybeSupporter();
#else
    return KisApplication::isStoreApplication();
#endif
}

QVector<KisSplashScreen::Source> KisSplashScreen::getSources()
{
    bool canChoose = canChooseSplashScreen();
    return {
        // First entry is used as the default.
        {
            QString(),
            i18nc("Splash screen option showing Kiki", "Mascot"),
            defaultSplashResourcePath(),
            i18nc("Splash artist name", "Tyson Tan"),
            true,
            true,
        },
        {
            QStringLiteral("corpo1"),
            i18nc("Splash screen option without Kiki", "Plain"),
            QStringLiteral(":/splash/corpo1.jpg"),
            // This splash screen was also created by Tyson, but they preferred
            // to not be credited prominently for something so simple.
            QString(),
            canChoose,
            false,
        },
    };
}

QString KisSplashScreen::defaultSplashResourcePath()
{
    // Loading the ginormous 4K PNG splash image increases the startup time on
    // Android by several seconds and at the same time looks really bad when
    // scaled down to a dinky size. Instead of overengineering this into an
    // Enterprise Splash Screen Solution where we choose the image based on
    // screen size or something, we'll just use a HD JPEG instead. It's fine.
#ifdef Q_OS_ANDROID
    return QStringLiteral(":/splash/hd.jpg");
#else
    return QStringLiteral(":/splash/0.png");
#endif
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
