/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#ifndef __KISDONATIONMANAGEMENTDIALOG_H_
#define __KISDONATIONMANAGEMENTDIALOG_H_

#include <QDialog>
#include <QVector>

#include "kritaui_export.h"
#include "ui_KisDonationManagementDialog.h"
#include <KisAndroidDonations.h>
#include <KisSupporterProduct.h>

class KRITAUI_EXPORT KisDonationManagementDialog : public QDialog, public Ui::KisDonationManagementDialog
{
    Q_OBJECT
public:
    explicit KisDonationManagementDialog(QWidget *parent = nullptr);

    void reshow();

Q_SIGNALS:
    void sigShowSupporterBundles();

private Q_SLOTS:
    void slotShowOverview();
    void slotUpdateCurrentProducts();

private:
    void addProductWidgets(const QString &heading, const QVector<KisSupporterProduct> &products);

    void addProductSeparator();

    static bool isProductIdsEqual(const QVector<KisSupporterProduct> &a, const QVector<KisSupporterProduct> &b);

    KisAndroidDonations *m_androidDonations;
    QVector<KisSupporterProduct> m_availableProducts;
    QVector<KisSupporterProduct> m_ownedProducts;
};

#endif
