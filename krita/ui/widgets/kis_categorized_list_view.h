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

#include <krita_export.h>
#include <QListView>


class KRITAUI_EXPORT KisCategorizedListView: public QListView
{
    Q_OBJECT
public:
    KisCategorizedListView(bool useCheckBoxHack=false, QWidget* parent=0);
    virtual ~KisCategorizedListView();
    virtual void setModel(QAbstractItemModel* model);

signals:
    void sigCategoryToggled(const QModelIndex& index, bool toggled);
    void sigEntryChecked(const QModelIndex& index);

protected slots:
    void slotIndexChanged(const QModelIndex& index);
    virtual void dataChanged(const QModelIndex& topLeft, const QModelIndex& bottomRight);
    virtual void rowsInserted(const QModelIndex& parent, int start, int end);
    virtual void rowsAboutToBeRemoved(const QModelIndex &parent, int start, int end);
    virtual void mousePressEvent(QMouseEvent* event);
    virtual void mouseReleaseEvent(QMouseEvent* event);

private:
    void updateRows(int begin, int end);

private:
    bool m_useCheckBoxHack;
};

#endif // KIS_CATEGORIZED_LIST_VIEW_H_
