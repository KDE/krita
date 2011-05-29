/* This file is part of the KDE project
   Copyright (c) 2002 Patrick Julien <freak@codepimps.org>
   Copyright (c) 2007 Jan Hambrecht <jaham@gmx.net>
   Copyright (c) 2007 Sven Langkamp <sven.langkamp@gmail.com>
   Copyright (C) 2011 Srikanth Tiyyagura <srikanth.tulasiram@gmail.com>

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
#include <QAbstractProxyModel>

#include <kfiledialog.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kdebug.h>

#ifdef GHNS
#include <knewstuff3/downloaddialog.h>
#include <knewstuff3/uploaddialog.h>
#endif

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
    QString knsrcFile;
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
    connect( d->view, SIGNAL(clicked( const QModelIndex & ) ),
             this, SLOT(activated ( const QModelIndex & ) ) );

    d->buttonGroup = new QButtonGroup( this );
    d->buttonGroup->setExclusive( false );

    QGridLayout* layout = new QGridLayout( this );
    layout->addWidget( d->view, 0, 0, 1, 5 );

    QPushButton *button = new QPushButton( this );
    button->setIcon( SmallIcon( "document-open" ) );
    button->setToolTip( i18n("Import") );
    button->setEnabled( true );
    d->buttonGroup->addButton( button, Button_Import );
    layout->addWidget( button, 1, 0 );

    button = new QPushButton( this );
    button->setIcon( SmallIcon( "trash-empty" ) );
    button->setToolTip( i18n("Delete") );
    button->setEnabled( false );
    d->buttonGroup->addButton( button, Button_Remove );
    layout->addWidget( button, 1, 1 );

    button = new QPushButton( this );
    button->setIcon( SmallIcon( "download" ) );
    button->setToolTip( i18n("Download") );
    button->setEnabled( true );
    button->hide();
    d->buttonGroup->addButton( button, Button_GhnsDownload );
    layout->addWidget( button, 1, 3 );

    button = new QPushButton( this );
    button->setIcon( SmallIcon( "go-up" ) );
    button->setToolTip( i18n("Share") );
    button->setEnabled( false );
    button->hide();
    d->buttonGroup->addButton( button, Button_GhnsUpload);
    layout->addWidget( button, 1, 4 );


    connect( d->buttonGroup, SIGNAL( buttonClicked( int ) ), this, SLOT( slotButtonClicked( int ) ));

    layout->setColumnStretch( 0, 1 );
    layout->setColumnStretch( 1, 1 );
    layout->setColumnStretch( 2, 2 );
    layout->setSpacing( 0 );
    layout->setMargin( 0 );

    updateButtonState();
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

        d->model->resourceServerAdapter()->importResourceFile(filename);
    }
    else if( button == Button_Remove ) {
        QModelIndex index = d->view->currentIndex();
        int row = index.row();
        int column = index.column();
        if( index.isValid() ) {

            KoResource * resource = resourceFromModelIndex(index);
            if( resource ) {
                d->model->resourceServerAdapter()->removeResource(resource);
            }
        }
        if (column == 0) {
            int rowMin = --row;
            row = qBound(0, rowMin, row);
        }
        int columnMin = --column;
        column = qBound(0, columnMin, column);
        setCurrentItem(row, column);
        activated(d->model->index(row, column));
    }
#ifdef GHNS
    else if (button == Button_GhnsDownload) {

        KNS3::DownloadDialog dialog( d->knsrcFile, this );
        dialog.exec();

        foreach (const KNS3::Entry& e, dialog.changedEntries() ) {

             foreach( const QString &file, e.installedFiles() ) {
                 QFileInfo fi(file);
                  d->model->resourceServerAdapter()->importResourceFile( fi.absolutePath()+"/"+fi.fileName() , false );
              }

       foreach( const QString &file, e.uninstalledFiles() ) {
                 QFileInfo fi(file);
                 d->model->resourceServerAdapter()->removeResourceFile( fi.absolutePath()+"/"+fi.fileName() );
              }
      }
     }
    else if (button == Button_GhnsUpload) {

        QModelIndex index = d->view->currentIndex();
        if( index.isValid() ) {


            KoResource * resource = resourceFromModelIndex(index);
            if( resource ) {
                KNS3::UploadDialog dialog( d->knsrcFile, this );
                dialog.setUploadFile(KUrl::fromLocalFile(resource->filename()));
                dialog.setUploadName(resource->name());
                dialog.exec();
            }
        }
    }
#endif
    updateButtonState();
}

void KoResourceItemChooser::showButtons( bool show )
{
    foreach( QAbstractButton * button, d->buttonGroup->buttons() )
        show ? button->show() : button->hide();
}

void KoResourceItemChooser::showGetHotNewStuff( bool showDownload, bool showUpload )
{
#ifdef GHNS
    QAbstractButton *button = d->buttonGroup->button(Button_GhnsDownload);
    showDownload ? button->show() : button->hide();
    button = d->buttonGroup->button(Button_GhnsUpload);
    showUpload ? button->show() : button->hide();
#endif
}

void KoResourceItemChooser::setColumnCount( int columnCount )
{
    d->model->setColumnCount( columnCount );
    //Force an update to get the right column width
    QRect geometry = d->view->geometry();
    d->view->setGeometry(geometry.adjusted(0, 0, 0, 1));
    d->view->setGeometry(geometry);
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
        return resourceFromModelIndex(index);

    return 0;
}

void KoResourceItemChooser::setCurrentResource(KoResource* resource)
{
    QModelIndex index = d->model->indexFromResource(resource);
    if( !index.isValid() )
        return;

    d->view->setCurrentIndex(index);
}

void KoResourceItemChooser::setCurrentItem(int row, int column)
{
    QModelIndex index = d->model->index(row, column);
    if( !index.isValid() )
        return;

    d->view->setCurrentIndex(index);
}

void KoResourceItemChooser::setProxyModel( QAbstractProxyModel* proxyModel )
{
    proxyModel->setSourceModel(d->model);
    d->view->setModel(proxyModel);
}

void KoResourceItemChooser::activated( const QModelIndex & index )
{
    if( ! index.isValid() )
        return;

    KoResource * resource = resourceFromModelIndex(index);

    if( resource ) {
        emit resourceSelected( resource );
    }
    updateButtonState();
}

void KoResourceItemChooser::updateButtonState()
{
    QAbstractButton * removeButton = d->buttonGroup->button( Button_Remove );
    if( ! removeButton )
        return;

    QAbstractButton * uploadButton = d->buttonGroup->button( Button_GhnsUpload );
    if( ! uploadButton )
        return;

    KoResource * resource = currentResource();
    if( resource ) {
        removeButton->setEnabled( resource->removable() );
        uploadButton->setEnabled( resource->removable() );
        return;
    }

    removeButton->setEnabled( false );
    uploadButton->setEnabled( false );
}

KoResource* KoResourceItemChooser::resourceFromModelIndex(const QModelIndex& index)
{
    if(!index.isValid())
        return 0;

    const QAbstractProxyModel* proxyModel = dynamic_cast<const QAbstractProxyModel*>(index.model());
    if(proxyModel) {
        //Get original model index, because proxy models destroy the internalPointer
        QModelIndex originalIndex = proxyModel->mapToSource(index);
        return static_cast<KoResource*>( originalIndex.internalPointer() );
    }

    return static_cast<KoResource*>( index.internalPointer() );
}

void KoResourceItemChooser::setKnsrcFile(const QString& knsrcFileArg)
{
    d->knsrcFile = knsrcFileArg;
}

QSize KoResourceItemChooser::viewSize()
{
    return d->view->size();
}

#include <KoResourceItemChooser.moc>
