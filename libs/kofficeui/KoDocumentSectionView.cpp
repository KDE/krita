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
#include <QContextMenuEvent>
#include <QHeaderView>
#include <QHelpEvent>
#include <QMenu>
#include <QMouseEvent>
#include "KoDocumentSectionPropertyAction_p.h"
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

void KoDocumentSectionView::addPropertyActions( QMenu *menu, const QModelIndex &index )
{
    Model::PropertyList list = index.data( Model::PropertiesRole ).value<Model::PropertyList>();
    for( int i = 0, n = list.count(); i < n; ++i )
        if( list.at( i ).isMutable )
        {
            PropertyAction *a = new PropertyAction( i, list.at( i ), index, menu );
            connect( a, SIGNAL( toggled( bool, const QPersistentModelIndex&, int ) ),
                     this, SLOT( slotActionToggled( bool, const QPersistentModelIndex&, int ) ) );
            menu->addAction( a );
        }
}

bool KoDocumentSectionView::event( QEvent *e )
{
    if( e->type() == QEvent::MouseButtonPress && model() )
    {
        const QPoint pos = static_cast<QMouseEvent*>( e )->pos();
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

bool KoDocumentSectionView::viewportEvent( QEvent *e )
{
    if( e->type() == QEvent::ToolTip && model() )
    {
        const QPoint pos = static_cast<QHelpEvent*>( e )->pos();
        if( !indexAt( pos ).isValid() )
            return super::viewportEvent( e );
        QModelIndex index = model()->buddy( indexAt( pos ) );
        QStyleOptionViewItem option = viewOptions();
        option.rect = visualRect( index );
        if( index == currentIndex() )
            option.state |= QStyle::State_HasFocus;

        return d->delegate->editorEvent( e, model(), option, index );
    }

    return super::viewportEvent( e );
}

void KoDocumentSectionView::contextMenuEvent( QContextMenuEvent *e )
{
    super::contextMenuEvent( e );
    QModelIndex i = indexAt( e->pos() );
    if( model() )
        i = model()->buddy( i );
    showContextMenu( e->globalPos(), i );
}

void KoDocumentSectionView::showContextMenu( const QPoint &globalPos, const QModelIndex &index )
{
    emit contextMenuRequested( globalPos, index );
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

void KoDocumentSectionView::slotActionToggled( bool on, const QPersistentModelIndex &index, int num )
{
    Model::PropertyList list = index.data( Model::PropertiesRole ).value<Model::PropertyList>();
    list[num].state = on;
    const_cast<QAbstractItemModel*>( index.model() )->setData( index, QVariant::fromValue( list ), Model::PropertiesRole );
}

#include "KoDocumentSectionPropertyAction_p.moc"
#include "KoDocumentSectionView.moc"
