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

#include <QHeaderView>
#include <QHelpEvent>
#include "KoDocumentSectionDelegate.h"
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
    if( e->type() == QEvent::ToolTip && model() )
    {
        QHelpEvent *he = static_cast<QHelpEvent*>( e );
        if( !indexAt( he->pos() ).isValid() )
            return super::event( e );
        QModelIndex index = model()->buddy( indexAt( he->pos() ) );
        QStyleOptionViewItem option = viewOptions();
        option.rect = visualRect( index );
        if( index == currentIndex() )
            option.state |= QStyle::State_HasFocus;
        return d->delegate->editorEvent( e, model(), option, index );
    }
    else
        return super::event( e );
}

#include "KoDocumentSectionView.moc"
