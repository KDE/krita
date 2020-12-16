/*
 *  SPDX-FileCopyrightText: 2011 Silvio Heinrich <plassy@web.de>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KIS_CATEGORIZED_LIST_VIEW_H_
#define KIS_CATEGORIZED_LIST_VIEW_H_

#include <kritaui_export.h>
#include <QListView>
#include <QListWidget>
#include <QScroller>

class KRITAUI_EXPORT KisCategorizedListView: public QListView
{
    Q_OBJECT
public:
    KisCategorizedListView(QWidget* parent=0);
    ~KisCategorizedListView() override;
    void setModel(QAbstractItemModel* model) override;

    QSize sizeHint() const override;
    void setCompositeBoxControl(bool value);

Q_SIGNALS:
    void sigCategoryToggled(const QModelIndex& index, bool toggled);
    void sigEntryChecked(const QModelIndex& index);
    void rightClickedMenuDropSettingsTriggered();
    void rightClickedMenuSaveSettingsTriggered();
    void lockAreaTriggered(const QModelIndex& index);

protected Q_SLOTS:
    void slotIndexChanged(const QModelIndex& index);
    void dataChanged(const QModelIndex& topLeft, const QModelIndex& bottomRight, const QVector<int> &roles = QVector<int>()) override;
    void rowsInserted(const QModelIndex& parent, int start, int end) override;
    void rowsAboutToBeRemoved(const QModelIndex &parent, int start, int end) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void slotScrollerStateChange(QScroller::State state);

private:
    void updateRows(int begin, int end);    
    bool isCompositeBoxControl = false;
};

#endif // KIS_CATEGORIZED_LIST_VIEW_H_
