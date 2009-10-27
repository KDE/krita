/* This file is part of the KDE project
   Copyright (c) 2002 Patrick Julien <freak@codepimps.org>
   Copyright (c) 2007 Jan Hambrecht <jaham@gmx.net>
   Copyright (c) 2007 Sven Langkamp <sven.langkamp@gmail.com>

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
 * Boston, MA 02110-1301, USA.
*/
#include "KoResourceItemChooser.h"

#include <QGridLayout>
#include <QButtonGroup>
#include <QPushButton>
#include <QHeaderView>

#include <kfiledialog.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kdebug.h>

#include "KoResourceServerAdapter.h"
#include "KoResourceItemView.h"
#include "KoResourceItemDelegate.h"
#include "KoResourceModel.h"
#include "KoResource.h"

class KoResourceItemChooser::Private
{
public:
    Private() : model(0), view(0), buttonGroup(0) {}
    KoResourceModel* model;
    KoResourceItemView* view;
    QButtonGroup* buttonGroup;
};

KoResourceItemChooser::KoResourceItemChooser( KoAbstractResourceServerAdapter * resourceAdapter, QWidget *parent )
 : QWidget( parent ), d( new Private() )
{
    Q_ASSERT(resourceAdapter);
    d->model = new KoResourceModel(resourceAdapter, this);
    d->view = new KoResourceItemView(this);
    d->view->setModel(d->model);
    d->view->setItemDelegate( new KoResourceItemDelegate( this ) );
    d->view->setSelectionMode( QAbstractItemView::SingleSelection );
    connect( d->view, SIGNAL(activated( const QModelIndex & ) ),
             this, SLOT(activated ( const QModelIndex & ) ) );

    d->buttonGroup = new QButtonGroup( this );
    d->buttonGroup->setExclusive( false );

    QGridLayout* layout = new QGridLayout( this );
    layout->addWidget( d->view, 0, 0, 1, 3 );

    QPushButton *button = new QPushButton( this );
    button->setIcon( SmallIcon( "list-add" ) );
    button->setToolTip( i18n("Import") );
    button->setEnabled( true );
    d->buttonGroup->addButton( button, Button_Import );
    layout->addWidget( button, 1, 0 );

    button = new QPushButton( this );
    button->setIcon( SmallIcon( "list-remove" ) );
    button->setToolTip( i18n("Delete") );
    button->setEnabled( false );
    d->buttonGroup->addButton( button, Button_Remove );
    layout->addWidget( button, 1, 1 );

    connect( d->buttonGroup, SIGNAL( buttonClicked( int ) ), this, SLOT( slotButtonClicked( int ) ));

    layout->setColumnStretch( 0, 1 );
    layout->setColumnStretch( 1, 1 );
    layout->setColumnStretch( 2, 2 );
    layout->setSpacing( 0 );
    layout->setMargin( 3 );

    updateRemoveButtonState();
}

KoResourceItemChooser::~KoResourceItemChooser()
{
    delete d;
}

void KoResourceItemChooser::slotButtonClicked( int button )
{
    if( button == Button_Import ) {
        QString extensions = d->model->resourceServerAdapter()->extensions();
        QString filter = extensions.replace(QString(":"), QString(" "));
        QString filename = KFileDialog::getOpenFileName( KUrl(), filter, 0, i18n( "Choose File to Add" ) );

        d->model->resourceServerAdapter()->importResource(filename);
    }
    else if( button == Button_Remove ) {
        QModelIndex index = d->view->currentIndex();
        if( index.isValid() ) {

            KoResource * resource = static_cast<KoResource*>( index.internalPointer() );
            if( resource ) {
                d->model->resourceServerAdapter()->removeResource(resource);
            }
        }
    }
    updateRemoveButtonState();
}

void KoResourceItemChooser::showButtons( bool show )
{
    foreach( QAbstractButton * button, d->buttonGroup->buttons() )
        show ? button->show() : button->hide();
}

void KoResourceItemChooser::setColumnCount( int columnCount )
{
    d->model->setColumnCount( columnCount );
}

void KoResourceItemChooser::setRowHeight( int rowHeight )
{
    d->view->verticalHeader()->setDefaultSectionSize( rowHeight );
}

void KoResourceItemChooser::setItemDelegate( QAbstractItemDelegate * delegate )
{
    d->view->setItemDelegate(delegate);
}

KoResource *  KoResourceItemChooser::currentResource()
{
    QModelIndex index = d->view->currentIndex();
    if( index.isValid() )
        return static_cast<KoResource*>( index.internalPointer() );

    return 0;
}

void KoResourceItemChooser::setCurrentItem(int row, int column)
{
    QModelIndex index = d->model->index(row, column);
    if( !index.isValid() )
        return;

    d->view->setCurrentIndex(index);
}

void KoResourceItemChooser::activated( const QModelIndex & index )
{
    if( ! index.isValid() )
        return;

    KoResource * resource = static_cast<KoResource*>( index.internalPointer() );

    if( resource ) {
        emit resourceSelected( resource );
    }
    updateRemoveButtonState();
}

void KoResourceItemChooser::updateRemoveButtonState()
{
    QAbstractButton * removeButton = d->buttonGroup->button( Button_Remove );
    if( ! removeButton )
        return;

    KoResource * resource = currentResource();
    if( resource ) {
        removeButton->setEnabled( resource->removable() );
        return;
    }

    removeButton->setEnabled( false );
}

#include "KoResourceItemChooser.moc"
