/****************************************************************************
** Copyright (C) 2001-2010 Klaralvdalens Datakonsult AB.  All rights reserved.
**
** This file is part of the KD Chart library.
**
** Licensees holding valid commercial KD Chart licenses may use this file in
** accordance with the KD Chart Commercial License Agreement provided with
** the Software.
**
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 and version 3 as published by the
** Free Software Foundation and appearing in the file LICENSE.GPL included.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** Contact info@kdab.com if any conditions of this licensing are not
** clear to you.
**
**********************************************************************/

#include <QtDebug>

#include "KDChartDatasetSelector.h"

#include "ui_KDChartDatasetSelector.h"

#include <KDABLibFakes>

using namespace KDChart;

DatasetSelectorWidget::DatasetSelectorWidget ( QWidget* parent )
    : QFrame ( parent )
    , mUi ( new Ui::DatasetSelector () )
    , mSourceRowCount ( 0 )
    , mSourceColumnCount ( 0 )
{
    qWarning("For DatasetSelectorWidget to become useful, it has to be connected to the proxy model it configures!");

    mUi->setupUi ( this );
    setMinimumSize ( minimumSizeHint() );
}

DatasetSelectorWidget::~DatasetSelectorWidget()
{
    delete mUi;
}

void DatasetSelectorWidget::on_sbStartColumn_valueChanged ( int )
{
    calculateMapping();
}

void DatasetSelectorWidget::on_sbStartRow_valueChanged( int )
{
    calculateMapping();
}

void DatasetSelectorWidget::on_sbColumnCount_valueChanged ( int )
{
    calculateMapping();
}

void DatasetSelectorWidget::on_sbRowCount_valueChanged ( int )
{
    calculateMapping();
}

void DatasetSelectorWidget::on_cbReverseRows_stateChanged ( int )
{
    calculateMapping();
}

void DatasetSelectorWidget::on_cbReverseColumns_stateChanged ( int )
{
    calculateMapping();
}

void DatasetSelectorWidget::on_groupBox_toggled( bool state )
{
    if ( state )
    {
        calculateMapping();
    } else {
        emit mappingDisabled();
    }
}


void DatasetSelectorWidget::setSourceRowCount ( const int& rowCount )
{
    if ( rowCount != mSourceRowCount )
    {
        mSourceRowCount = rowCount;
        resetDisplayValues();
    }
}

void DatasetSelectorWidget::setSourceColumnCount ( const int& columnCount )
{
    if ( columnCount != mSourceColumnCount )
    {
        mSourceColumnCount = columnCount;
        resetDisplayValues();
    }
}

void DatasetSelectorWidget::resetDisplayValues()
{
    mUi->sbStartRow->setValue( 0 );
    mUi->sbStartRow->setMinimum ( 0 );
    mUi->sbStartRow->setMaximum ( qMax ( mSourceRowCount - 1, 0 ) );
    mUi->sbStartColumn->setValue( 0 );
    mUi->sbStartColumn->setMinimum ( 0 );
    mUi->sbStartColumn->setMaximum ( qMax ( mSourceColumnCount - 1, 0 ) );
    mUi->sbRowCount->setMinimum ( 1 );
    mUi->sbRowCount->setMaximum ( mSourceRowCount );
    mUi->sbRowCount->setValue( mSourceRowCount );
    mUi->sbColumnCount->setMinimum ( 1 );
    mUi->sbColumnCount->setMaximum ( mSourceColumnCount );
    mUi->sbColumnCount->setValue( mSourceColumnCount );
    mUi->groupBox->setChecked ( false );
    emit mappingDisabled();
}

void DatasetSelectorWidget::calculateMapping()
{
    if ( mSourceColumnCount < 2 && mSourceRowCount < 2 )
    {
        mUi->groupBox->setEnabled ( false );
        emit mappingDisabled();
    } else {
        mUi->groupBox->setEnabled ( true );

        if ( ! mUi->groupBox->isChecked() )
        {
            emit mappingDisabled();
            return;
        }

        // retrieve values:
        int startRow = mUi->sbStartRow->value();
        int startColumn = mUi->sbStartColumn->value();
        int rowCount = mUi->sbRowCount->value();
        int columnCount = mUi->sbColumnCount->value();
        bool reverseColumns = mUi->cbReverseColumns->checkState() == Qt::Checked;
        bool reverseRows = mUi->cbReverseRows->checkState() == Qt::Checked;

        // verify values:
        startRow = qMin ( startRow,  mSourceRowCount - 2 );
        startRow = qMax ( 0, startRow );
        startColumn = qMin ( startColumn,  mSourceColumnCount - 2 );
        startColumn = qMax ( 0,  startColumn );

        rowCount = qMin ( rowCount, mSourceRowCount - startRow );
        rowCount = qMax ( 1, rowCount );
        columnCount = qMin ( columnCount, mSourceColumnCount - startColumn );
        columnCount = qMax ( 1, columnCount );

        DatasetDescriptionVector rowConfig ( rowCount );
        Q_ASSERT ( rowConfig.size() > 0 );
        DatasetDescriptionVector columnConfig ( columnCount );
        Q_ASSERT ( columnConfig.size() > 0 );

        // fill the dataset description vectors:
        for ( int row = 0; row < rowCount; ++row )
        {
            if ( reverseRows )
            {
                rowConfig[row] = startRow + rowCount - row - 1;
            } else {
                rowConfig[row] = startRow + row;
            }
        }

        for ( int column = 0; column < columnCount; ++ column )
        {
            if ( reverseColumns )
            {
                columnConfig[column] = startColumn + columnCount - column -1;
            } else {
                columnConfig[column] = startColumn + column;
            }
        }

        // and tell the world:
        emit configureDatasetProxyModel ( rowConfig, columnConfig );
    }
}

