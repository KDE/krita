/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#ifndef __KISSUPPORTERBUNDLEWIDGET_H_
#define __KISSUPPORTERBUNDLEWIDGET_H_

#include <QPointer>
#include <QVector>
#include <QWidget>
#include <functional>

#include "KisSupporterBundle.h"
#include "KisSupporterBundlesFetcher.h"
#include "ui_KisSupporterBundleWidget.h"
#include <KisSupporterProduct.h>

class QNetworkReply;
class QProgressDialog;

class KisSupporterBundleWidget : public QWidget, public Ui::KisSupporterBundleWidget
{
    Q_OBJECT
public:
    explicit KisSupporterBundleWidget(KisSupporterBundlesFetcher *fetcher,
                                      const KisSupporterBundle &bundle,
                                      const QVector<KisSupporterProduct> &bundleProducts,
                                      QWidget *parent = nullptr);

    const KisSupporterBundle &bundle() const
    {
        return m_bundle;
    }

    bool isProductOwned() const
    {
        return m_productOwned;
    }

    bool canImport() const
    {
        return m_canImport;
    }

private Q_SLOTS:
    void slotStartDownload();
    void slotUpdateDownloadProgress(qint64 bytesReceived, qint64 bytesTotal);

private:
    void
    addContentEntry(QStringList &outContentList, const QString &key, const std::function<QString(int)> &formatFn) const;

    void handleDownloadFinished(QNetworkReply *reply);
    void showFinishedDownload(bool success, bool cancel, const QString &errorMessage);
    void closeProgressDlg();

    QPointer<KisSupporterBundlesFetcher> m_fetcher;
    QProgressDialog *m_progressDlg{nullptr};
    KisSupporterBundle m_bundle;
    bool m_productOwned{false};
    bool m_canImport;
};

#endif
