// SPDX-License-Identifier: GPL-3.0-or-later
#include "KisSplashSourceDialog.h"
#include <QPixmap>
#include <QPushButton>
#include <QSignalBlocker>
#include <kis_signal_compressor.h>
#ifdef Q_OS_ANDROID
#include <KisAndroidDonations.h>
#endif

KisSplashSourceDialog::KisSplashSourceDialog(const QString &key, QWidget *parent)
    : QDialog(parent)
    , m_sources(KisSplashScreen::getSources())
    , m_updatePreviewCompressor(
          new KisSignalCompressor(100, KisSignalCompressor::POSTPONE, KisSignalCompressor::ADDITIVE_INTERVAL, this))
{
    setupUi(this);

#ifdef Q_OS_ANDROID
    KisAndroidDonations *androidDonations = KisAndroidDonations::instance();
    if (androidDonations && androidDonations->state() == KisAndroidDonations::State::NoSupport) {
        m_explanationLabel->setText(
            QStringLiteral("%1 <a href=\"#\">%2</a>")
                .arg(i18n("Become a supporter to change the splash screen.").toHtmlEscaped(), i18n("Support Krita")));

        connect(m_explanationLabel, &QLabel::linkActivated, this, &KisSplashSourceDialog::slotShowDonationDialog);

        connect(androidDonations,
                &KisAndroidDonations::sigStateChanged,
                this,
                &KisSplashSourceDialog::slotRefreshSources);
    } else {
        m_explanationLabel->setText(
            i18n("Changing the splash screen is only available for supporters through the Play Store."));
    }
#else
    QString explanationText = i18n("Purchase Krita to change the splash screen and receive automatic updates.");

    QStringList purchaseOptions = {
        makePurchaseOption(QStringLiteral("steam"), i18n("Get Krita on Steam")),
#ifdef Q_OS_WINDOWS
        makePurchaseOption(QStringLiteral("epicgamesstore"), i18n("Get Krita on the Epic Games Store")),
        makePurchaseOption(QStringLiteral("microsoftstore"), i18n("Get Krita on the Microsoft Store")),
#endif
#ifdef Q_OS_MACOS
        makePurchaseOption(QStringLiteral("appleappstore"), i18n("Get Krita on the App Store")),
#endif
    };

    m_explanationLabel->setOpenExternalLinks(true);
    m_explanationLabel->setText(
        QStringLiteral("%1<ul>%2</ul>").arg(explanationText.toHtmlEscaped(), purchaseOptions.join(QString())));
#endif

    connect(m_sourceCombo,
            QOverload<int>::of(&QComboBox::currentIndexChanged),
            this,
            &KisSplashSourceDialog::slotSourceSelected);
    connect(m_updatePreviewCompressor, &KisSignalCompressor::timeout, this, &KisSplashSourceDialog::slotUpdatePreview);
    connect(m_buttons, &QDialogButtonBox::accepted, this, &KisSplashSourceDialog::accept);
    connect(m_buttons, &QDialogButtonBox::rejected, this, &KisSplashSourceDialog::reject);

    initSources(key);
}

QString KisSplashSourceDialog::selectedKey() const
{
    const KisSplashScreen::Source *source = currentSource();
    if (source && source->isAvailable) {
        return source->key;
    } else {
        return QString();
    }
}

void KisSplashSourceDialog::slotSourceSelected()
{
    const KisSplashScreen::Source *source = currentSource();
    m_buttons->button(QDialogButtonBox::Ok)->setEnabled(source && source->isAvailable);
    m_previewWidget->setPixmap(QPixmap());
    m_updatePreviewCompressor->start();
}

void KisSplashSourceDialog::slotUpdatePreview()
{
    const KisSplashScreen::Source *source = currentSource();
    if (source) {
        QPixmap img(source->resourcePath);
        m_previewWidget->setPixmap(img);
    }
}

void KisSplashSourceDialog::slotRefreshSources()
{
    initSources(selectedKey());
}

#ifdef Q_OS_ANDROID
void KisSplashSourceDialog::slotShowDonationDialog()
{
    KisAndroidDonations::showDonationDialog(false);
}
#endif

void KisSplashSourceDialog::initSources(const QString &key)
{
    QSignalBlocker blocker(m_sourceCombo);
    m_explanationLabel->setVisible(!KisSplashScreen::canChooseSplashScreen());
    m_sources = KisSplashScreen::getSources();
    m_sourceCombo->clear();
    for (const KisSplashScreen::Source &source : m_sources) {
        m_sourceCombo->addItem(source.description());
        if (key == source.key) {
            m_sourceCombo->setCurrentIndex(m_sourceCombo->count() - 1);
        }
    }
    slotSourceSelected();
}

const KisSplashScreen::Source *KisSplashSourceDialog::currentSource() const
{
    int index = m_sourceCombo->currentIndex();
    if (index >= 0 && index < m_sources.size()) {
        return &m_sources[index];
    } else {
        return nullptr;
    }
}

QString KisSplashSourceDialog::makePurchaseOption(const QString &key, const QString &text)
{
    return QStringLiteral("<li><a href=\"https://krita.org/%1/\">%2</a></li>")
        .arg(key.toHtmlEscaped(), text.toHtmlEscaped());
}
