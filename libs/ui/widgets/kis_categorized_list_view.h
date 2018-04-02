/*
 *  Copyright (c) 2011 Silvio Heinrich <plassy@web.de>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef KIS_CATEGORIZED_LIST_VIEW_H_
#define KIS_CATEGORIZED_LIST_VIEW_H_

#include <kritaui_export.h>
#include <QListView>
#include <QListWidget>


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

private:
    void updateRows(int begin, int end);    
    bool isCompositeBoxControl = false;
};

#endif // KIS_CATEGORIZED_LIST_VIEW_H_
