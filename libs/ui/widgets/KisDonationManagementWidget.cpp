/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#include "KisDonationManagementWidget.h"
#include <KisAndroidDonations.h>
#include <QPushButton>
#include <kis_assert.h>

KisDonationManagementWidget::KisDonationManagementWidget(const KisSupporterProduct &product, QWidget *parent)
    : QWidget(parent)
    , m_product(product)
{
    setupUi(this);

    if (m_product.icon.isNull()) {
        m_lblPreview->hide();
    } else {
        m_lblPreview->setPixmap(m_product.icon);
    }

    m_lblTitle->setText(QStringLiteral("<strong>%1</strong>").arg(m_product.title.toHtmlEscaped()));
    m_lblDescription->setText(m_product.description);

    if (m_product.availability == KisSupporterProductAvailability::Owned) {
        QPushButton *btnResources = new QPushButton(i18n("Download Resource Bundles"));
        m_lytDetails->addWidget(btnResources);
        connect(btnResources, &QPushButton::clicked, this, &KisDonationManagementWidget::sigShowSupporterBundles);

        if (m_product.type == KisSupporterProductType::Subscription) {
            QPushButton *btnManageSubscription = new QPushButton(i18n("Manage Subscription"));
            m_lytDetails->addWidget(btnManageSubscription);
            connect(btnManageSubscription,
                    &QPushButton::clicked,
                    this,
                    &KisDonationManagementWidget::slotManageSubscription);
        }
    } else if (m_product.availability == KisSupporterProductAvailability::Available) {
        QPushButton *btnPurchase = new QPushButton(product.buttonText);
        m_lytDetails->addWidget(btnPurchase);
        connect(btnPurchase, &QPushButton::clicked, this, &KisDonationManagementWidget::slotPurchase);
    }
}

void KisDonationManagementWidget::slotManageSubscription()
{
    KisAndroidDonations *androidDonations = KisAndroidDonations::instance();
    KIS_SAFE_ASSERT_RECOVER_RETURN(androidDonations);
    androidDonations->slotManageSubscription(m_product.id);
}

void KisDonationManagementWidget::slotPurchase()
{
    KisAndroidDonations *androidDonations = KisAndroidDonations::instance();
    KIS_SAFE_ASSERT_RECOVER_RETURN(androidDonations);
    androidDonations->startBillingFlowFor(m_product.id, m_product.offerToken);
}
