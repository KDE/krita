/*
 *  Copyright (c) 2006 Gábor Lehel <illissius@gmail.com>
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

#include <QTreeView>
#include "KoDocumentSectionDelegate.h"
#include "KoDocumentSectionWidget.h"

class KoDocumentSectionWidget::Private
{
    public:
        KoDocumentSectionDelegate *delegate;
        QTreeView *view;
        Private(): delegate( 0 ), view( 0 ) { }
};

KoDocumentSectionWidget::KoDocumentSectionWidget( QWidget *parent )
    : QWidget( parent )
    , d( new Private )
{
    d->view = new QTreeView( this );
    d->delegate = new KoDocumentSectionDelegate( this );
    d->view->setItemDelegate( d->delegate );
}

KoDocumentSectionWidget::~KoDocumentSectionWidget()
{
    delete d;
}

void KoDocumentSectionWidget::setModel( KoDocumentSectionModel *model )
{
    d->view->setModel( model );
}

#include "KoDocumentSectionWidget.moc"
