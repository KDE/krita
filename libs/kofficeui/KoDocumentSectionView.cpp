/*
  Copyright (c) 2006 Gábor Lehel <illissius@gmail.com>

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

#include <QtDebug>
#include <QHeaderView>
#include <QHelpEvent>
#include <QMouseEvent>
#include "KoDocumentSectionDelegate.h"
#include "KoDocumentSectionModel.h"
#include "KoDocumentSectionView.h"

class KoDocumentSectionView::Private
{
    public:
        KoDocumentSectionDelegate *delegate;
        Private(): delegate( 0 ) { }
};

KoDocumentSectionView::KoDocumentSectionView( QWidget *parent )
    : QTreeView( parent )
    , d( new Private )
{
    d->delegate = new KoDocumentSectionDelegate( this, this );
    header()->hide();
}

KoDocumentSectionView::~KoDocumentSectionView()
{
    delete d;
}

bool KoDocumentSectionView::event( QEvent *e )
{
    if( ( e->type() == QEvent::ToolTip || e->type() == QEvent::MouseButtonPress ) && model() )
    {
        const QPoint pos = e->type() == QEvent::ToolTip
                         ? static_cast<QHelpEvent*>( e )->pos()
                         : static_cast<QMouseEvent*>( e )->pos();
        if( !indexAt( pos ).isValid() )
            return super::event( e );
        QModelIndex index = model()->buddy( indexAt( pos ) );
        QStyleOptionViewItem option = viewOptions();
        option.rect = visualRect( index );
        if( index == currentIndex() )
            option.state |= QStyle::State_HasFocus;

        return d->delegate->editorEvent( e, model(), option, index );
    }

    return super::event( e );
}

void KoDocumentSectionView::currentChanged( const QModelIndex &current, const QModelIndex &previous )
{
    super::currentChanged( current, previous );
    if( current != previous ) //hack?
        const_cast<QAbstractItemModel*>( current.model() )->setData( current, true, Model::ActiveRole );
}

void KoDocumentSectionView::dataChanged( const QModelIndex &topLeft, const QModelIndex &bottomRight )
{
    super::dataChanged( topLeft, bottomRight );
    for( int x = topLeft.row(); x <= bottomRight.row(); ++x )
        for( int y = topLeft.column(); y <= bottomRight.column(); ++y )
            if( topLeft.sibling( x, y ).data( Model::ActiveRole ).toBool() )
            {
                setCurrentIndex( topLeft.sibling( x, y ) );
                return;
            }
}

#include "KoDocumentSectionView.moc"
