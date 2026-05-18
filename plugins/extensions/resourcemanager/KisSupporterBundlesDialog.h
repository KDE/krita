/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#ifndef __KISSUPPORTERBUNDLESDIALOG_H_
#define __KISSUPPORTERBUNDLESDIALOG_H_

#include <QDialog>
#include <QSet>
#include <QVector>

#include "ui_KisSupporterBundlesDialog.h"
#include <KisSupporterProduct.h>

class KisSupporterBundlesFetcher;
class KisSupporterBundleWidget;
struct KisSupporterBundle;

class KisSupporterBundlesDialog : public QDialog, public Ui::KisSupporterBundlesDialog
{
    Q_OBJECT
public:
    explicit KisSupporterBundlesDialog(QWidget *parent = nullptr);

private Q_SLOTS:
    void slotFetchIndexDone(bool success, const QString &errorMessage);
    void slotResetBundleWidgets();

private:
    static constexpr int PAGE_FETCHING = 0;
    static constexpr int PAGE_ERROR = 1;
    static constexpr int PAGE_BUNDLES = 2;

    void fetchBundles();

    QVector<KisSupporterProduct> getCurrentProducts() const;

    static QVector<KisSupporterProduct> getProductsForBundle(const QVector<KisSupporterProduct> &currentProducts,
                                                             const KisSupporterBundle &bundle);

    static const KisSupporterProduct *searchProductById(const QVector<KisSupporterProduct> &currentProducts,
                                                        const QString &id);

    static bool couldOwnAnyProduct(const QVector<KisSupporterProduct> &bundleProducts);

    static bool bundleWidgetLessThan(const KisSupporterBundleWidget *a, const KisSupporterBundleWidget *b);

    KisSupporterBundlesFetcher *m_fetcher;
};

#endif
