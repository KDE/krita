/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#include "KisSupporterBundleWidget.h"
#include <KisAndroidDonations.h>
#include <KisNetworkAccessManager.h>
#include <KisStorageModel.h>
#include <QCoreApplication>
#include <QCryptographicHash>
#include <QMessageBox>
#include <QNetworkReply>
#include <QProgressDialog>
#include <QTimer>
#include <kis_assert.h>
#include <kis_debug.h>

KisSupporterBundleWidget::KisSupporterBundleWidget(KisSupporterBundlesFetcher *fetcher,
                                                   const KisSupporterBundle &bundle,
                                                   const QVector<KisSupporterProduct> &bundleProducts,
                                                   QWidget *parent)
    : QWidget(parent)
    , m_fetcher(fetcher)
    , m_bundle(bundle)
{
    setupUi(this);

    QStringList contentList;
    addContentEntry(contentList, QStringLiteral("paintoppresets"), [](int count) {
        return i18np("%1 brush preset", "%1 brush presets", count);
    });
    addContentEntry(contentList, QStringLiteral("brushes"), [](int count) {
        return i18np("%1 brush tips", "%1 brush tips", count);
    });
    addContentEntry(contentList, QStringLiteral("patterns"), [](int count) {
        return i18np("%1 texture patterns", "%1 texture patterns", count);
    });
    addContentEntry(contentList, QStringLiteral("workspaces"), [](int count) {
        return i18np("%1 workspace layout", "%1 workspace layouts", count);
    });

    QStringList productList;
    for (const KisSupporterProduct &product : bundleProducts) {
        switch (product.availability) {
        case KisSupporterProductAvailability::Missing:
            // Product is neither available for purchase nor does the user own
            // it, so we'll pretend like it doesn't exist.
            break;
        case KisSupporterProductAvailability::Available:
            productList.append(product.title);
            break;
        case KisSupporterProductAvailability::Owned:
            if (product.type == KisSupporterProductType::Subscription) {
                productList.append(i18n("%1 (subscribed)").arg(product.title));
            } else {
                productList.append(i18n("%1 (owned)").arg(product.title));
            }
            m_productOwned = true;
            break;
        }
    }

    if (m_bundle.thumbnail.isNull()) {
        m_lblPreview->hide();
    } else {
        m_lblPreview->setPixmap(m_bundle.thumbnail);
    }

    QString author = m_bundle.author.toHtmlEscaped();
    if (!m_bundle.source.isEmpty()) {
        author = QStringLiteral("<a href=\"%1\">%2</a>").arg(m_bundle.source.toHtmlEscaped(), author);
    }
    m_lblTitle->setText(QStringLiteral("<strong>%1</strong> by %2").arg(m_bundle.title.toHtmlEscaped(), author));
    m_lblDescription->setText(m_bundle.description);

    // The margins on this stuff is ginormous by default, cut that down.
    QString productsText = QStringLiteral(
        "<style>\n"
        "ul { margin-top: 0px; margin-bottom: 4px; margin-left: -10px; }\n"
        "li { margin: 0px; padding: 0px; }\n"
        "</style>");
    if (!contentList.isEmpty()) {
        productsText.append(
            i18nc("in-app purchases, prefix for a list of what a bundle contains", "Contains:").toHtmlEscaped());
        productsText.append(QStringLiteral("<ul>"));
        for (const QString &content : contentList) {
            productsText.append(QStringLiteral("<li>%1</li>").arg(content.toHtmlEscaped()));
        }
        productsText.append(QStringLiteral("</ul>"));
    }

    if (productList.isEmpty()) {
        productsText.append(i18nc("in-app purchases, a bundle that the user can't acquire", "Not available."));
    } else {
        productsText.append(
            i18nc("in-app purchases, prefix for a list of which purchases make a bundle available", "Available with:")
                .toHtmlEscaped());
        productsText.append(QStringLiteral("<ul>"));
        for (const QString &product : productList) {
            productsText.append(QStringLiteral("<li>%1</li>").arg(product.toHtmlEscaped()));
        }
        productsText.append(QStringLiteral("</ul>"));
    }

    m_lblProducts->setText(productsText);

    KisStorageModel *storageModel = KisStorageModel::instance();
    m_canImport = storageModel->canImportStorage(m_bundle.fileName);
    if (!m_canImport) {
        m_stkDownload->setCurrentWidget(m_pgDone);
    } else if (m_productOwned) {
        m_stkDownload->setCurrentWidget(m_pgGet);
    } else {
        m_stkDownload->setCurrentWidget(m_pgSupport);
    }

    connect(m_btnGet, &QPushButton::clicked, this, &KisSupporterBundleWidget::slotStartDownload);

    KisAndroidDonations *androidDonations = KisAndroidDonations::instance();
    if (androidDonations) {
        connect(m_btnSupport, &QPushButton::clicked, androidDonations, &KisAndroidDonations::slotStartDonationFlow);
    } else {
        m_btnSupport->setEnabled(false);
    }
}

void KisSupporterBundleWidget::slotStartDownload()
{
    KIS_SAFE_ASSERT_RECOVER_RETURN(m_productOwned);
    KIS_SAFE_ASSERT_RECOVER_RETURN(m_fetcher);
    KIS_SAFE_ASSERT_RECOVER_RETURN(!m_progressDlg);

    m_progressDlg = new QProgressDialog(this);
    m_progressDlg->setAutoClose(false);
    m_progressDlg->setAutoReset(false);
    m_progressDlg->setModal(true);
    m_progressDlg->setMinimumDuration(0);
    m_progressDlg->setRange(0, 0);
    m_progressDlg->setLabelText(i18n("Downloading bundle…"));
    m_progressDlg->setWindowTitle(i18n("%1").arg(m_bundle.title));
    m_progressDlg->show();

    QNetworkReply *reply = m_fetcher->fetchBundle(m_bundle);
    connect(reply, &QNetworkReply::downloadProgress, this, &KisSupporterBundleWidget::slotUpdateDownloadProgress);
    connect(m_progressDlg, &QProgressDialog::canceled, reply, &QNetworkReply::abort);
    connect(reply, &QNetworkReply::finished, this, [=] {
        handleDownloadFinished(reply);
        reply->deleteLater();
    });
}

void KisSupporterBundleWidget::slotUpdateDownloadProgress(qint64 bytesReceived, qint64 bytesTotal)
{
    if (m_progressDlg) {
        if (bytesTotal <= 0) {
            m_progressDlg->setRange(0, 0);
        } else {
            m_progressDlg->setRange(0, 100);
            m_progressDlg->setValue(qRound(qreal(bytesTotal) / qreal(bytesReceived) * 100.0));
        }
    }
}

void KisSupporterBundleWidget::addContentEntry(QStringList &outContentList,
                                               const QString &key,
                                               const std::function<QString(int)> &formatFn) const
{
    int count = m_bundle.resourceCountByMediaType.value(key, 0);
    if (count > 0) {
        outContentList.append(formatFn(count));
    }
}

void KisSupporterBundleWidget::handleDownloadFinished(QNetworkReply *reply)
{
    if (reply->error() == QNetworkReply::OperationCanceledError) {
        showFinishedDownload(false, true, QString());
        return;
    }

    QString errorMessage;
    if (KisSupporterBundlesFetcher::hasNetworkReplyError(reply, errorMessage)) {
        showFinishedDownload(false, false, errorMessage);
        return;
    }

    slotUpdateDownloadProgress(1, 1);
    QCoreApplication::processEvents();

    QByteArray bytes = reply->readAll();
    long long sizeInBytes = bytes.size();
    if (sizeInBytes != m_bundle.sizeInBytes) {
        warnResources << "Bundle" << m_bundle.fileName << "size of" << sizeInBytes << "does not match expected"
                      << m_bundle.sizeInBytes;
        showFinishedDownload(false, false, i18n("Invalid bundle size"));
        return;
    }

    QString hash = QCryptographicHash::hash(bytes, QCryptographicHash::Md5).toHex();
    if (hash != bundle().checksum) {
        warnResources << "Bundle" << m_bundle.fileName << "hash" << hash << "does not match expected"
                      << m_bundle.checksum;
        showFinishedDownload(false, false, i18n("Invalid bundle checksum"));
        return;
    }

    m_progressDlg->setLabelText(i18n("Activating bundle…"));
    slotUpdateDownloadProgress(0, 0);
    QCoreApplication::processEvents();

    KisStorageModel *storageModel = KisStorageModel::instance();
    QGuiApplication::setOverrideCursor(Qt::WaitCursor);
    bool importOk = storageModel->importStorageData(m_bundle.fileName, KisStorageModel::None, bytes);
    QGuiApplication::restoreOverrideCursor();

    if (!importOk) {
        showFinishedDownload(false, false, i18n("Failed to import bundle"));
        return;
    }

    showFinishedDownload(true, false, QString());
}

void KisSupporterBundleWidget::showFinishedDownload(bool success, bool cancel, const QString &errorMessage)
{
    if (success) {
        m_btnGet->clearFocus(); // Prevent auto-scrolling to the next button.
        closeProgressDlg();
        m_canImport = false;
        m_stkDownload->setCurrentWidget(m_pgDone);
    } else if (cancel) {
        closeProgressDlg();
        m_btnGet->setEnabled(true);
        m_stkDownload->setCurrentWidget(m_pgGet);
    } else {
        // Delay this a bit to avoid excessively fast retries and to make it
        // clear to the user that a retry is actually happening. Otherwise the
        // download progress bar may flash so quickly that it seems like it's
        // not doing anything at all.
        QTimer::singleShot(1000, [=] {
            closeProgressDlg();
            QMessageBox::warning(this, i18n("Download failed"), i18n("Bundle download failed: %1").arg(errorMessage));
            m_stkDownload->setCurrentWidget(m_pgGet);
        });
    }
}

void KisSupporterBundleWidget::closeProgressDlg()
{
    if (m_progressDlg) {
        m_progressDlg->close();
        m_progressDlg = nullptr;
    }
}
