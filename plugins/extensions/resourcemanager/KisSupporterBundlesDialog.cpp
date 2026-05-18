/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "KisSupporterBundlesDialog.h"
#include "KisSupporterBundleWidget.h"
#include "KisSupporterBundlesFetcher.h"
#include <KisAndroidDonations.h>
#include <KisKineticScroller.h>
#include <QFrame>
#include <QTimer>
#include <kis_assert.h>
#include <kis_debug.h>

KisSupporterBundlesDialog::KisSupporterBundlesDialog(QWidget *parent)
    : QDialog(parent)
    , m_fetcher(new KisSupporterBundlesFetcher(this))
{
    setupUi(this);
    setWindowTitle(i18n("Supporter Bundles"));

    QScroller *scroller = KisKineticScroller::createPreconfiguredScroller(m_scrBundles);
    if (scroller) {
        connect(scroller, &QScroller::stateChanged, this, [&](QScroller::State state) {
            KisKineticScroller::updateCursor(this, state);
        });
    }

    connect(m_fetcher,
            &KisSupporterBundlesFetcher::fetchIndexDone,
            this,
            &KisSupporterBundlesDialog::slotFetchIndexDone);
    connect(m_btnRetry, &QPushButton::clicked, this, &KisSupporterBundlesDialog::fetchBundles);
    connect(m_buttons, &QDialogButtonBox::rejected, this, &KisSupporterBundlesDialog::reject);

    fetchBundles();
}

void KisSupporterBundlesDialog::slotFetchIndexDone(bool success, const QString &errorMessage)
{
    m_btnRetry->setEnabled(!success);
    if (success) {
        slotResetBundleWidgets();
        m_stack->setCurrentIndex(PAGE_BUNDLES);

        KisAndroidDonations *androidDonations = KisAndroidDonations::instance();
        if (androidDonations) {
            connect(androidDonations,
                    &KisAndroidDonations::sigStateChanged,
                    this,
                    &KisSupporterBundlesDialog::slotResetBundleWidgets);
        }

    } else {
        m_lblError->setText(i18n("<strong>Error:</strong> %1").arg(errorMessage.toHtmlEscaped()));
        // Delay this a bit to avoid excessively fast retries and to make it
        // clear to the user that a retry is actually happening. Otherwise the
        // loading page may flash so quickly that it seems like it's not doing
        // anything at all.
        QTimer::singleShot(1000, [this] {
            m_stack->setCurrentIndex(PAGE_ERROR);
        });
    }
}

void KisSupporterBundlesDialog::slotResetBundleWidgets()
{
    while (!m_lytWdgBundles->isEmpty()) {
        QLayoutItem *item = m_lytWdgBundles->takeAt(m_lytWdgBundles->count() - 1);
        QWidget *widget = item->widget();
        if (widget) {
            widget->deleteLater();
        }
        delete item;
    }

    QVector<KisSupporterProduct> currentProducts = getCurrentProducts();

    QVector<KisSupporterBundleWidget *> bundleWidgets;
    for (const KisSupporterBundle &bundle : m_fetcher->bundles()) {
        // Gather the products that this bundle is associated with.
        QVector<KisSupporterProduct> bundleProducts = getProductsForBundle(currentProducts, bundle);
        // Only show bundles that the user could actually acquire, there's no
        // point in dangling a bundle in front of their nose that they couldn't
        // purchase any product for.
        if (couldOwnAnyProduct(bundleProducts)) {
            KisSupporterBundleWidget *bundleWidget = new KisSupporterBundleWidget(m_fetcher, bundle, bundleProducts);
            bundleWidgets.append(bundleWidget);
        } else {
            dbgResources << "No products for bundle" << bundle.fileName;
        }
    }

    if (bundleWidgets.isEmpty()) {
        m_lytWdgBundles->addStretch();
        QLabel *label = new QLabel(i18n("No supporter bundles available."));
        label->setAlignment(Qt::AlignCenter);
        label->setWordWrap(true);
        m_lytWdgBundles->addWidget(label);
        m_lytWdgBundles->addStretch();
    } else {
        std::sort(bundleWidgets.begin(), bundleWidgets.end(), &KisSupporterBundlesDialog::bundleWidgetLessThan);

        // We've been asked to put this disclaimer here so that people don't
        // contact the authors and complain about paywalling or something.
        // Don't remove this message without discussing it with them.
        QLabel *purchaseNoteLabel =
            new QLabel(i18n("These bundles have been made available by their authors to help provide rewards for "
                            "supporting Krita's development. They do not receive compensation for purchases and may "
                            "make these bundles in other ways behind the links below."));
        purchaseNoteLabel->setTextFormat(Qt::PlainText);
        purchaseNoteLabel->setWordWrap(true);
        m_lytWdgBundles->addWidget(purchaseNoteLabel);

        for (KisSupporterBundleWidget *bundleWidget : bundleWidgets) {
            QFrame *separator = new QFrame;
            separator->setFrameShape(QFrame::HLine);
            separator->setFrameShadow(QFrame::Sunken);
            m_lytWdgBundles->addWidget(separator);
            m_lytWdgBundles->addWidget(bundleWidget);
        }
        m_lytWdgBundles->addStretch();
    }
}

void KisSupporterBundlesDialog::fetchBundles()
{
    if (!m_fetcher->isFetching()) {
        m_stack->setCurrentIndex(PAGE_FETCHING);
        m_btnRetry->setEnabled(false);
        m_fetcher->fetchIndex();
    }
}

QVector<KisSupporterProduct> KisSupporterBundlesDialog::getCurrentProducts() const
{
    KisAndroidDonations *androidDonations = KisAndroidDonations::instance();
    KIS_SAFE_ASSERT_RECOVER_RETURN_VALUE(androidDonations, QVector<KisSupporterProduct>());
    return androidDonations->getCurrentProducts();
}

QVector<KisSupporterProduct>
KisSupporterBundlesDialog::getProductsForBundle(const QVector<KisSupporterProduct> &currentProducts,
                                                const KisSupporterBundle &bundle)
{
    const QString prefix = QStringLiteral("p:");
    QVector<KisSupporterProduct> bundleProducts;
    dbgResources << "Bundle" << bundle.fileName << "tags:" << bundle.tags;
    for (const QString &tag : bundle.tags) {
        if (tag.startsWith(prefix)) {
            const KisSupporterProduct *product = searchProductById(currentProducts, tag.mid(prefix.length()));
            if (product) {
                bundleProducts.append(*product);
            }
        }
    }
    return bundleProducts;
}

const KisSupporterProduct *
KisSupporterBundlesDialog::searchProductById(const QVector<KisSupporterProduct> &currentProducts, const QString &id)
{
    for (const KisSupporterProduct &product : currentProducts) {
        if (product.id == id) {
            return &product;
        }
    }
    dbgResources << "No current product for id" << id;
    return nullptr;
}

bool KisSupporterBundlesDialog::couldOwnAnyProduct(const QVector<KisSupporterProduct> &bundleProducts)
{
    for (const KisSupporterProduct &product : bundleProducts) {
        switch (product.availability) {
        case KisSupporterProductAvailability::Missing:
            break;
        case KisSupporterProductAvailability::Available:
        case KisSupporterProductAvailability::Owned:
            return true;
        }
    }
    return false;
}

bool KisSupporterBundlesDialog::bundleWidgetLessThan(const KisSupporterBundleWidget *a,
                                                     const KisSupporterBundleWidget *b)
{
    bool aOwned = a->isProductOwned();
    bool bOwned = b->isProductOwned();
    if (aOwned && !bOwned) {
        return true;
    } else if (!aOwned && bOwned) {
        return false;
    }

    bool aCanImport = a->canImport();
    bool bCanImport = b->canImport();
    if (aCanImport && !bCanImport) {
        return true;
    } else if (!aCanImport && bCanImport) {
        return false;
    }

    return a->bundle().title.compare(b->bundle().title, Qt::CaseInsensitive) < 0;
}
