/*
    SPDX-FileCopyrightText: 2021 Waqar Ahmed <waqar.17a@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/
#include <QList>
#include <QMenu>

class QTreeView;
class QLineEdit;
class CommandModel;
class QAction;
class CommandBarFilterModel;
class KActionCollection;

#include <kritawidgetutils_export.h>

class KRITAWIDGETUTILS_EXPORT KateCommandBar : public QMenu
{
    Q_OBJECT
public:
    KateCommandBar(QWidget *parent = nullptr);

    void updateBar(const QList<KActionCollection *> &actions, int totalActions);

    void updateViewGeometry();

protected:
    bool eventFilter(QObject *obj, QEvent *event) override;

private Q_SLOTS:
    void slotReturnPressed();
    void reselectFirst();

private:
    QTreeView *m_treeView;
    QLineEdit *m_lineEdit;
    CommandModel *m_model;
    CommandBarFilterModel *m_proxyModel;
    QVector<KActionCollection *> m_disposableActionCollections;
};
