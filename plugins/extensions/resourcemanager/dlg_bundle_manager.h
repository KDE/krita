/*
 *  SPDX-FileCopyrightText: 2014 Victor Lafon metabolic.ewilan @hotmail.fr
 *  SPDX-FileCopyrightText: 2023 Srirupa Datta <srirupa.sps@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */
#ifndef DLG_BUNDLE_MANAGER_H
#define DLG_BUNDLE_MANAGER_H

#include <KoDialog.h>
#include <QModelIndex>
#include <QPersistentModelIndex>
#include <QStyledItemDelegate>
#include <QWidget>

#include "ui_wdgdlgbundlemanager.h"

class KisStorageModel;
class KisStorageFilterProxyModel;

class WdgDlgBundleManager : public QWidget, public Ui::WdgDlgBundleManager
{
    Q_OBJECT

public:
    WdgDlgBundleManager(QWidget *parent)
        : QWidget(parent)
    {
        setupUi(this);
    }
};

class DlgBundleManager : public KoDialog
{
    Q_OBJECT
public:

    class ItemDelegate : public QStyledItemDelegate
    {
    public:

        ItemDelegate(QObject*, KisStorageFilterProxyModel*);
        QSize sizeHint(const QStyleOptionViewItem & option, const QModelIndex & index) const override;
        void paint(QPainter * painter, const QStyleOptionViewItem & option, const QModelIndex & index) const override;

    private:
        KisStorageFilterProxyModel* m_bundleManagerProxyModel;

    };

    explicit DlgBundleManager(QWidget *parent = 0);

public Q_SLOTS:
    void done(int res) override;

private Q_SLOTS:

    void addBundle();
    void createBundle();
    void toggleBundle();
    void editBundle();

    void slotModelAboutToBeReset();
    void slotModelReset();
    void slotRowsRemoved(const QModelIndex &parent, int first, int last);
    void slotRowsInserted(const QModelIndex &parent, int first, int last);

    void currentCellSelectedChanged(QModelIndex current, QModelIndex previous);

private:

    void updateToggleButton(bool active);
    void updateBundleInformation(QModelIndex idx);

    WdgDlgBundleManager *m_ui;
    QPersistentModelIndex lastIndex;
    KisStorageFilterProxyModel* m_proxyModel;

};

#endif // DLG_BUNDLE_MANAGER_H
