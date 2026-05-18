/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#ifndef __KISDONATIONMANAGEMENTWIDGET_H_
#define __KISDONATIONMANAGEMENTWIDGET_H_

#include <QWidget>

#include "kritaui_export.h"
#include "ui_KisDonationManagementWidget.h"
#include <KisSupporterProduct.h>

class KisDonationManagementWidget : public QWidget, public Ui::KisDonationManagementWidget
{
    Q_OBJECT
public:
    explicit KisDonationManagementWidget(const KisSupporterProduct &product, QWidget *parent = nullptr);

Q_SIGNALS:
    void sigShowSupporterBundles();

private Q_SLOTS:
    void slotManageSubscription();
    void slotPurchase();

private:
    KisSupporterProduct m_product;
};

#endif
