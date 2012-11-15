/****************************************************************************
 ** Copyright (C) 2001-2006 Klarälvdalens Datakonsult AB.  All rights reserved.
 **
 ** This file is part of the KD Gantt library.
 **
 ** This file may be used under the terms of the GNU General Public
 ** License versions 2.0 or 3.0 as published by the Free Software
 ** Foundation and appearing in the files LICENSE.GPL2 and LICENSE.GPL3
 ** included in the packaging of this file.  Alternatively you may (at
 ** your option) use any later version of the GNU General Public
 ** License if such license has been publicly approved by
 ** Klarälvdalens Datakonsult AB (or its successors, if any).
 ** 
 ** This file is provided "AS IS" with NO WARRANTY OF ANY KIND,
 ** INCLUDING THE WARRANTIES OF DESIGN, MERCHANTABILITY AND FITNESS FOR
 ** A PARTICULAR PURPOSE. Klarälvdalens Datakonsult AB reserves all rights
 ** not expressly granted herein.
 ** 
 ** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
 ** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 **
 **********************************************************************/
#include "kdgantttreeviewrowcontroller.h"
#include "kdgantttreeviewrowcontroller_p.h"

#include <QAbstractProxyModel>
#include <QHeaderView>

#include <cassert>

using namespace KDGantt;

/*!\class TreeViewRowController
 * This is an implementation of AbstractRowController that
 * aligns a gantt view with a QTreeView.
 */

TreeViewRowController::TreeViewRowController( QTreeView* tv,
					      QAbstractProxyModel* proxy )
  : _d( new Private )
{
    _d->treeview = static_cast<Private::HackTreeView*>(tv);
    _d->proxy = proxy;
}

TreeViewRowController::~TreeViewRowController()
{
    delete _d; _d=0;
}

#define d d_func()

int TreeViewRowController::headerHeight() const
{
  //return d->treeview->header()->sizeHint().height();
    return d->treeview->viewport()->y()-d->treeview->frameWidth();
}

int TreeViewRowController::maximumItemHeight() const
{
    return d->treeview->fontMetrics().height();
}

bool TreeViewRowController::isRowVisible( const QModelIndex& _idx ) const
{
  //qDebug() << _idx.model()<<d->proxy << d->treeview->model();
    const QModelIndex idx = d->proxy->mapToSource( _idx );
    assert( idx.isValid() ? ( idx.model() == d->treeview->model() ):( true ) );
    return d->treeview->visualRect(idx).isValid();
}

Span TreeViewRowController::rowGeometry( const QModelIndex& _idx ) const
{
    const QModelIndex idx = d->proxy->mapToSource( _idx );
    assert( idx.isValid() ? ( idx.model() == d->treeview->model() ):( true ) );
    QRect r = d->treeview->visualRect(idx).translated( QPoint( 0, d->treeview->verticalOffset() ) );
    return Span( r.y(), r.height() );
}

QModelIndex TreeViewRowController::indexAt( int height ) const
{
    return d->proxy->mapFromSource( d->treeview->indexAt( QPoint( 1,height ) ) );
}

QModelIndex TreeViewRowController::indexAbove( const QModelIndex& _idx ) const
{
    const QModelIndex idx = d->proxy->mapToSource( _idx );
    return d->proxy->mapFromSource( d->treeview->indexAbove( idx ) );
}

QModelIndex TreeViewRowController::indexBelow( const QModelIndex& _idx ) const
{
    const QModelIndex idx = d->proxy->mapToSource( _idx );
    return d->proxy->mapFromSource( d->treeview->indexBelow( idx ) );
}
