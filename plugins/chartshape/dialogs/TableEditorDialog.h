/* This file is part of the KDE project

   Copyright 2009 Johannes Simon <johannes.simon@gmail.com>
   Copyright 2009 Inge Wallin    <inge@lysator.liu.se>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#ifndef KCHART_TABLE_EDITOR_DIALOG_H
#define KCHART_TABLE_EDITOR_DIALOG_H

#include <QDialog>

#include "ui_ChartTableEditor.h"

class QModelIndex;
class QAction;


namespace KChart {

class ChartTableView;
class ChartProxyModel;

class TableEditorDialog : public QDialog, public Ui::ChartTableEditor
{
    Q_OBJECT

public:
    TableEditorDialog();
    ~TableEditorDialog();

    void init();
    void setProxyModel( ChartProxyModel *proxyModel );
    void updateDialog();

protected slots:
    void slotInsertRowPressed();
    void slotDeleteRowPressed();
    void slotInsertColumnPressed();
    void slotDeleteColumnPressed();

    void deleteSelectedRowsOrColumns( Qt::Orientation orientation );
    void slotCurrentIndexChanged( const QModelIndex &index );
    void slotDataSetsInRowsToggled( bool enabled );

private:
    ChartProxyModel *m_proxyModel;
    ChartTableView  *const m_tableView;

    QAction         *m_deleteRowsAction;
    QAction         *m_deleteColumnsAction;
    QAction         *m_insertRowsAction;
    QAction         *m_insertColumnsAction;
};

} // Namespace KChart

#endif // KCHART_AXIS_SCALING_DIALOG_H

