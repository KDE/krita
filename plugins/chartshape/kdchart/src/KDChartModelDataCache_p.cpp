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

#include "KDChartModelDataCache_p.h"

#include <limits>

using namespace KDChart::ModelDataCachePrivate;

ModelSignalMapperConnector::ModelSignalMapperConnector( ModelSignalMapper& mapper )
    : QObject( 0 ),
      m_mapper( mapper )
{
}

ModelSignalMapperConnector::~ModelSignalMapperConnector()
{
}

void ModelSignalMapperConnector::connectSignals( QAbstractItemModel* model )
{
    connect( model, SIGNAL( destroyed() ),                              this, SLOT( resetModel() ) );
    connect( model, SIGNAL( columnsInserted( QModelIndex, int, int ) ), this, SLOT( columnsInserted( QModelIndex, int, int ) ) );
    connect( model, SIGNAL( columnsRemoved( QModelIndex, int, int ) ),  this, SLOT( columnsRemoved( QModelIndex, int, int ) ) );
    connect( model, SIGNAL( dataChanged( QModelIndex, QModelIndex ) ),  this, SLOT( dataChanged( QModelIndex, QModelIndex ) ) );
    connect( model, SIGNAL( layoutChanged() ),                          this, SLOT( layoutChanged() ) );
    connect( model, SIGNAL( modelReset() ),                             this, SLOT( modelReset() ) );
    connect( model, SIGNAL( rowsInserted( QModelIndex, int, int ) ),    this, SLOT( rowsInserted( QModelIndex, int, int )) );
    connect( model, SIGNAL( rowsRemoved( QModelIndex, int, int ) ),     this, SLOT( rowsRemoved( QModelIndex, int, int ) ) );
}

void ModelSignalMapperConnector::disconnectSignals( QAbstractItemModel* model )
{
    disconnect( model, SIGNAL( destroyed() ),                              this, SLOT( resetModel() ) );
    disconnect( model, SIGNAL( columnsInserted( QModelIndex, int, int ) ), this, SLOT( columnsInserted( QModelIndex, int, int ) ) );
    disconnect( model, SIGNAL( columnsRemoved( QModelIndex, int, int ) ),  this, SLOT( columnsRemoved( QModelIndex, int, int ) ) );
    disconnect( model, SIGNAL( dataChanged( QModelIndex, QModelIndex ) ),  this, SLOT( dataChanged( QModelIndex, QModelIndex ) ) );
    disconnect( model, SIGNAL( layoutChanged() ),                          this, SLOT( layoutChanged() ) );
    disconnect( model, SIGNAL( modelReset() ),                             this, SLOT( modelReset() ) );
    disconnect( model, SIGNAL( rowsInserted( QModelIndex, int, int ) ),    this, SLOT( rowsInserted( QModelIndex, int, int )) );
    disconnect( model, SIGNAL( rowsRemoved( QModelIndex, int, int ) ),     this, SLOT( rowsRemoved( QModelIndex, int, int ) ) );
}

void ModelSignalMapperConnector::resetModel()
{
    m_mapper.resetModel();
}

void ModelSignalMapperConnector::columnsInserted( const QModelIndex& parent, int start, int end )
{
    m_mapper.columnsInserted( parent, start, end );
}

void ModelSignalMapperConnector::columnsRemoved( const QModelIndex& parent, int start, int end )
{
    m_mapper.columnsRemoved( parent, start, end );
}

void ModelSignalMapperConnector::dataChanged( const QModelIndex& topLeft, const QModelIndex& bottomRight )
{
    m_mapper.dataChanged( topLeft, bottomRight );
}

void ModelSignalMapperConnector::layoutChanged()
{
    m_mapper.layoutChanged();
}

void ModelSignalMapperConnector::modelReset()
{
    m_mapper.modelReset();
}

void ModelSignalMapperConnector::rowsInserted( const QModelIndex& parent, int start, int end )
{
    m_mapper.rowsInserted( parent, start, end );
}

void ModelSignalMapperConnector::rowsRemoved( const QModelIndex& parent, int start, int end )
{
    m_mapper.rowsRemoved( parent, start, end );
}
