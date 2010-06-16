/* This file is part of the KDE project

   Copyright 2008 Johannes Simon <johannes.simon@gmail.com>
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

// Own
#include "TableEditorDialog.h"

// Qt
#include <QDebug>

// KDE
#include <kdebug.h>

// KChart
#include "ChartProxyModel.h"
#include "ChartTableView.h"


using namespace KChart;

TableEditorDialog::TableEditorDialog()
    : QDialog( 0 )
    , m_tableView( new ChartTableView )
{
    setupUi( this );

    m_proxyModel = 0;
    init();
}

TableEditorDialog::~TableEditorDialog()
{
    delete m_tableView;
}


void TableEditorDialog::init()
{
    tableViewContainer->addWidget( m_tableView );

    KIcon insertRowIcon = KIcon( "insert_table_row" );
    KIcon deleteRowIcon = KIcon( "delete_table_row" );
    KIcon insertColIcon = KIcon( "insert_table_col" );
    KIcon deleteColIcon = KIcon( "delete_table_col" );

    // Create actions.
    m_insertRowsAction    = new QAction( insertRowIcon, i18n( "Insert Rows" ), m_tableView );
    m_deleteRowsAction    = new QAction( deleteRowIcon, i18n( "Delete Rows" ), m_tableView );
    m_insertColumnsAction = new QAction( insertColIcon, i18n( "Insert Columns" ), m_tableView );
    m_deleteColumnsAction = new QAction( deleteColIcon, i18n( "Delete Columns" ), m_tableView );

    // Set icons on buttons(?).
    insertRow->setIcon( insertRowIcon );
    deleteRow->setIcon( deleteRowIcon );
    insertColumn->setIcon( insertColIcon );
    deleteColumn->setIcon( deleteColIcon );

    // Initially, no index is selected. Deletion only works with legal
    // selections.  They will automatically be enabled when an index
    // is selected.
    deleteRow->setEnabled( false );
    deleteColumn->setEnabled( false );

    // Buttons
    connect( insertRow,    SIGNAL( pressed() ), this, SLOT( slotInsertRowPressed() ) );
    connect( insertColumn, SIGNAL( pressed() ), this, SLOT( slotInsertColumnPressed() ) );
    connect( deleteRow,    SIGNAL( pressed() ), this, SLOT( slotDeleteRowPressed() ) );
    connect( deleteColumn, SIGNAL( pressed() ), this, SLOT( slotDeleteColumnPressed() ) );

    // Context Menu Actions
    connect( m_insertRowsAction,    SIGNAL( triggered() ), this, SLOT( slotInsertRowPressed() ) );
    connect( m_insertColumnsAction, SIGNAL( triggered() ), this, SLOT( slotInsertColumnPressed() ) );
    connect( m_deleteRowsAction,    SIGNAL( triggered() ), this, SLOT( slotDeleteRowPressed() ) );
    connect( m_deleteColumnsAction, SIGNAL( triggered() ), this, SLOT( slotDeleteColumnPressed() ) );
    connect( m_tableView,  SIGNAL( currentIndexChanged( const QModelIndex& ) ),
             this,         SLOT( slotCurrentIndexChanged( const QModelIndex& ) ) );
    // We only need to connect one of the data direction buttons, since
    // they are mutually exclusive.
    connect( dataSetsInRows, SIGNAL( toggled( bool ) ),
             this,           SLOT( slotDataSetsInRowsToggled( bool ) ) );

    // FIXME: QAction to create a separator??
    QAction *separator = new QAction( m_tableView );
    separator->setSeparator( true );

    // Add all the actions to the view.
    m_tableView->addAction( m_deleteRowsAction );
    m_tableView->addAction( m_insertRowsAction );
    m_tableView->addAction( separator );
    m_tableView->addAction( m_deleteColumnsAction );
    m_tableView->addAction( m_insertColumnsAction );

    m_tableView->setContextMenuPolicy( Qt::ActionsContextMenu );

    // Initialize the contents of the controls
    updateDialog();
}

void TableEditorDialog::setProxyModel( ChartProxyModel* proxyModel )
{
    if ( m_proxyModel == proxyModel )
        return;

    // Disconnect the old proxy model.
    if ( m_proxyModel ) {
        disconnect( m_proxyModel,    SIGNAL( modelReset() ),
                    this,            SLOT( updateDialog() ) );
        disconnect( firstRowIsLabel, SIGNAL( clicked( bool ) ),
                    m_proxyModel,    SLOT( setFirstRowIsLabel( bool ) ) );
        disconnect( firstColumnIsLabel, SIGNAL( clicked( bool ) ),
                    m_proxyModel,       SLOT( setFirstColumnIsLabel( bool ) ) );
    }

    m_proxyModel = proxyModel;

    // Connect the new proxy model.
    if ( m_proxyModel ) {
        m_tableView->setModel( m_proxyModel->sourceModel() );

        connect( m_proxyModel,       SIGNAL( modelReset() ), 
                 this,               SLOT( updateDialog() ) );
        connect( firstRowIsLabel,    SIGNAL( clicked( bool ) ),
                 m_proxyModel,       SLOT( setFirstRowIsLabel( bool ) ) );
        connect( firstColumnIsLabel, SIGNAL( clicked( bool ) ),
                 m_proxyModel,       SLOT( setFirstColumnIsLabel( bool ) ) );
    }

    updateDialog();
}

void TableEditorDialog::updateDialog()
{
    if ( !m_proxyModel )
        return;

    firstRowIsLabel->setChecked( m_proxyModel->firstRowIsLabel() );
    firstColumnIsLabel->setChecked( m_proxyModel->firstColumnIsLabel() );

    switch ( m_proxyModel->dataDirection() ) {
    case Qt::Horizontal:
        dataSetsInRows->setChecked( true );
        break;
    case Qt::Vertical:
        dataSetsInColumns->setChecked( true );
        break;
    default:
        kWarning(35001) << "Unrecognized value for data direction: " << m_proxyModel->dataDirection();
    }
}


// ----------------------------------------------------------------
//                             slots


void TableEditorDialog::slotInsertRowPressed()
{
    Q_ASSERT( m_tableView->model() );

    QAbstractItemModel *model = m_tableView->model();
    QModelIndex         currIndex = m_tableView->currentIndex();

    int selectedRow;
    if ( model->rowCount() == 0 )
        // +1 is added below.
        selectedRow = -1;
    else if ( currIndex.isValid() )
        selectedRow = currIndex.row();
    else
        selectedRow = m_tableView->model()->rowCount() - 1;

    // Insert the row *after* the selection, thus +1
    model->insertRow( selectedRow + 1 );
}

void TableEditorDialog::slotInsertColumnPressed()
{
    Q_ASSERT( m_tableView->model() );
    
    QAbstractItemModel *model = m_tableView->model();
    QModelIndex         currIndex = m_tableView->currentIndex();

    int selectedColumn;
    if ( model->columnCount() == 0 )
        // +1 is added below.
        selectedColumn = -1;
    if ( currIndex.isValid() )
        selectedColumn = currIndex.column();
    else
        selectedColumn = m_tableView->model()->columnCount() - 1;

    // Insert the column *after* the selection, thus +1
    model->insertColumn( selectedColumn + 1 );
}

void TableEditorDialog::slotDeleteRowPressed()
{
    deleteSelectedRowsOrColumns( Qt::Horizontal );
}

void TableEditorDialog::slotDeleteColumnPressed()
{
    deleteSelectedRowsOrColumns( Qt::Vertical );
}

void TableEditorDialog::deleteSelectedRowsOrColumns( Qt::Orientation orientation )
{
    // Note: In the following, both rows and columns will be referred to
    // as "row", for ease of reading this code.
    Q_ASSERT( m_tableView->model() );

    const QModelIndexList selectedIndexes = m_tableView->selectionModel()->selectedIndexes();
    if ( selectedIndexes.isEmpty() )
        return;

    QList<int> rowsToBeRemoved;
    // Make sure we don't delete a row twice, as indexes can exist
    // multiple times for one row
    foreach( const QModelIndex &index, selectedIndexes ) {
        const int row = orientation == Qt::Horizontal ? index.row() : index.column();
        if ( !rowsToBeRemoved.contains( row ) )
            rowsToBeRemoved.append( row );
    }

    // Use qGreater<int>() as comparator to remove rows in reversed order
    // to not change the indexes of the selected rows
    qSort( rowsToBeRemoved.begin(), rowsToBeRemoved.end(), qGreater<int>() );

    foreach( int row, rowsToBeRemoved ) {
        Q_ASSERT( row >= 0 );
        if ( orientation == Qt::Horizontal )
            m_tableView->model()->removeRow( row );
        else
            m_tableView->model()->removeColumn( row );
    }
    // Deselect the deleted rows
    m_tableView->setCurrentIndex( QModelIndex() );
}

void TableEditorDialog::slotCurrentIndexChanged( const QModelIndex &index )
{
    const bool isValid = index.isValid();

    m_deleteRowsAction->setEnabled( isValid );
    m_insertRowsAction->setEnabled( isValid );
    deleteRow->setEnabled( isValid );
    insertRow->setEnabled( isValid );

    m_deleteColumnsAction->setEnabled( isValid );
    m_insertColumnsAction->setEnabled( isValid );
    deleteColumn->setEnabled( isValid );
    insertColumn->setEnabled( isValid );
}

void TableEditorDialog::slotDataSetsInRowsToggled( bool enabled )
{
    Q_ASSERT( m_proxyModel );
    m_proxyModel->setDataDirection( enabled ? Qt::Horizontal : Qt::Vertical );
}
