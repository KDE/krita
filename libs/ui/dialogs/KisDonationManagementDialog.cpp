/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#include "KisDonationManagementDialog.h"
#include "kis_assert.h"
#include <KisDonationManagementWidget.h>
#include <KisKineticScroller.h>
#include <QFrame>
#include <QLabel>

KisDonationManagementDialog::KisDonationManagementDialog(QWidget *parent)
    : QDialog(parent)
    , m_androidDonations(KisAndroidDonations::instance())
{
    KIS_SAFE_ASSERT_RECOVER_NOOP(m_androidDonations);
    setupUi(this);
    setWindowTitle(i18n("Supporter Benefits"));

    QScroller *scroller = KisKineticScroller::createPreconfiguredScroller(m_scrBenefitsList);
    if (scroller) {
        connect(scroller, &QScroller::stateChanged, this, [&](QScroller::State state) {
            KisKineticScroller::updateCursor(this, state);
        });
    }

    QPushButton *btnManageSubscriptions =
        m_buttons->addButton(i18n("Manage Subscriptions"), QDialogButtonBox::ActionRole);

    if (m_androidDonations) {
        // It is theoretically possible that the user purchases a product while
        // Krita is still loading, hence the existence of this "pending" flag,
        // in case the Qt UI isn't ready yet. This here is the dialog that's
        // pending to be shown, so its appearance clears that flag.
        m_androidDonations->setShowDonationManagementDialogPending(false);
        connect(m_androidDonations,
                &KisAndroidDonations::sigStateChanged,
                this,
                &KisDonationManagementDialog::slotUpdateCurrentProducts,
                Qt::QueuedConnection);
        connect(m_btnNoBenefitsSupport,
                &QPushButton::clicked,
                m_androidDonations,
                &KisAndroidDonations::slotStartDonationFlow);
        connect(btnManageSubscriptions,
                &QPushButton::clicked,
                m_androidDonations,
                &KisAndroidDonations::slotManageSubscriptions);
    } else {
        // Shouldn't happen, but just revert to a blank no benefits page.
        m_stack->setCurrentWidget(m_pgNoBenefits);
        m_btnNoBenefitsSupport->hide();
    }

    slotUpdateCurrentProducts();

    connect(m_buttons, &QDialogButtonBox::rejected, this, &KisDonationManagementDialog::reject);
}

void KisDonationManagementDialog::reshow()
{
    if (m_androidDonations) {
        m_androidDonations->setShowDonationManagementDialogPending(false);
    }
    slotUpdateCurrentProducts();
    slotShowOverview();
    activateWindow();
    raise();
}

void KisDonationManagementDialog::slotShowOverview()
{
    if (m_availableProducts.isEmpty() && m_ownedProducts.isEmpty()) {
        m_stack->setCurrentWidget(m_pgNoBenefits);
    } else {
        m_stack->setCurrentWidget(m_pgBenefitsList);
    }
}

void KisDonationManagementDialog::slotUpdateCurrentProducts()
{
    QVector<KisSupporterProduct> availableProducts;
    QVector<KisSupporterProduct> ownedProducts;
    if (m_androidDonations) {
        for (const KisSupporterProduct &product : m_androidDonations->getCurrentProducts()) {
            switch (product.availability) {
            case KisSupporterProductAvailability::Missing:
                break;
            case KisSupporterProductAvailability::Available:
                availableProducts.append(product);
                break;
            case KisSupporterProductAvailability::Owned:
                ownedProducts.append(product);
                break;
            }
        }
    }

    bool productsChanged = !isProductIdsEqual(m_availableProducts, availableProducts)
        || !isProductIdsEqual(m_ownedProducts, ownedProducts);
    if (productsChanged) {
        m_availableProducts = availableProducts;
        m_ownedProducts = ownedProducts;

        while (!m_lytScrBenefitsList->isEmpty()) {
            QLayoutItem *item = m_lytScrBenefitsList->takeAt(m_lytScrBenefitsList->count() - 1);
            QWidget *widget = item->widget();
            if (widget) {
                widget->deleteLater();
            }
            delete item;
        }

        addProductWidgets(
            i18np("You have %1 supporter benefit:", "You have %1 supporter benefits:", m_ownedProducts.size()),
            m_ownedProducts);

        if (!m_availableProducts.isEmpty() && !m_ownedProducts.isEmpty()) {
            addProductSeparator();
            m_lytScrBenefitsList->addSpacing(12);
        }

        addProductWidgets(i18np("There is %1 supporter benefit available:",
                                "There are %1 supporter benefits available:",
                                m_availableProducts.size()),
                          m_availableProducts);

        m_lytScrBenefitsList->addStretch();
        slotShowOverview();
    }
}

void KisDonationManagementDialog::addProductWidgets(const QString &heading,
                                                    const QVector<KisSupporterProduct> &products)
{
    int count = products.size();
    if (count != 0) {
        QLabel *label =
            new QLabel(QStringLiteral("<span style=\"font-size:large;\">%1</span>").arg(heading.toHtmlEscaped()));
        label->setWordWrap(true);
        m_lytScrBenefitsList->addWidget(label);

        for (int i = 0; i < count; ++i) {
            addProductSeparator();
            KisDonationManagementWidget *widget = new KisDonationManagementWidget(products[i]);
            m_lytScrBenefitsList->addWidget(widget);
            connect(widget,
                    &KisDonationManagementWidget::sigShowSupporterBundles,
                    this,
                    &KisDonationManagementDialog::sigShowSupporterBundles);
        }
    }
}

void KisDonationManagementDialog::addProductSeparator()
{
    QFrame *separator = new QFrame;
    separator->setFrameShape(QFrame::HLine);
    separator->setFrameShadow(QFrame::Sunken);
    m_lytScrBenefitsList->addWidget(separator);
}

bool KisDonationManagementDialog::isProductIdsEqual(const QVector<KisSupporterProduct> &a,
                                                    const QVector<KisSupporterProduct> &b)
{
    int count = a.size();
    if (b.size() != count) {
        return false;
    }

    for (int i = 0; i < count; ++i) {
        if (a[i].id != b[i].id) {
            return false;
        }
    }

    return true;
}
