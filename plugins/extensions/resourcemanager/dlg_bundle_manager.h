/*
 *  SPDX-FileCopyrightText: 2014 Victor Lafon metabolic.ewilan @hotmail.fr
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */
#ifndef DLG_BUNDLE_MANAGER_H
#define DLG_BUNDLE_MANAGER_H

#include <QWidget>
#include <KoDialog.h>
#include <QModelIndex>
#include <QPersistentModelIndex>
#include <QStyledItemDelegate>

class KisStorageModel;
class KisStorageFilterProxyModel;

namespace Ui
{
class WdgDlgBundleManager;
}

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

    void slotModelAboutToBeReset();
    void slotModelReset();
    void currentCellSelectedChanged(QModelIndex current, QModelIndex previous);

private:

    void updateToggleButton(bool active);
    void updateBundleInformation(QModelIndex current);
    void addBundleToActiveResources(QString filename);

    QWidget *m_page;
    Ui::WdgDlgBundleManager *m_ui;
    QPersistentModelIndex lastIndex;
    KisStorageFilterProxyModel* m_proxyModel;

};

#endif // DLG_BUNDLE_MANAGER_H
