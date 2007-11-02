/* This file is part of the KDE project
 * Copyright (C) 2006-2007 Jan Hambrecht <jaham@gmx.net>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "KoPADocumentStructureDocker.h"
#include "KoPADocumentModel.h"
#include "KoPADocument.h"
#include "KoPAPageBase.h"
#if 0 // XXX: Add undo for reordering pages, look at Karbon
#include <KarbonLayerReorderCommand.h>
#endif

#include <KoDocumentSectionView.h>
#include <KoShapeManager.h>
#include <KoShapeBorderModel.h>
#include <KoShapeContainer.h>
#include <KoToolManager.h>
#include <KoCanvasBase.h>
#include <KoCanvasController.h>
#include <KoShapeControllerBase.h>
#include <KoSelection.h>
#include <KoShapeCreateCommand.h>
#include <KoShapeDeleteCommand.h>
#include <KoShapeReorderCommand.h>
#include <KoShapeLayer.h>

#include <klocale.h>
#include <kicon.h>
#include <kiconloader.h>
#include <kinputdialog.h>
#include <kmessagebox.h>

#include <QtGui/QGridLayout>
#include <QtGui/QPushButton>
#include <QtGui/QButtonGroup>

enum ButtonIds
{
    Button_New,
    Button_Raise,
    Button_Lower,
    Button_Delete
};

KoPADocumentStructureDockerFactory::KoPADocumentStructureDockerFactory( KoShapeControllerBase *shapeController, KoPADocument *document )
    : m_shapeController( shapeController ), m_document( document )
{
}

QString KoPADocumentStructureDockerFactory::id() const
{
    return QString("document section view");
}

QDockWidget* KoPADocumentStructureDockerFactory::createDockWidget()
{
    KoPADocumentStructureDocker* widget = new KoPADocumentStructureDocker( m_shapeController, m_document);
    widget->setObjectName(id());

    return widget;
}

KoPADocumentStructureDocker::KoPADocumentStructureDocker( KoShapeControllerBase *shapeController, KoPADocument *document )
    : m_shapeController( shapeController )
    , m_document( document )
    , m_model( 0 )
{
    setWindowTitle( i18n( "Document" ) );

    QWidget *mainWidget = new QWidget( this );
    QGridLayout* layout = new QGridLayout( mainWidget );
    layout->addWidget( m_sectionView = new KoDocumentSectionView( mainWidget ), 0, 0, 1, 4 );

    QButtonGroup *buttonGroup = new QButtonGroup( mainWidget );
    buttonGroup->setExclusive( false );

    QPushButton *button = new QPushButton( mainWidget );
    button->setIcon( SmallIcon( "list-add" ) );
    button->setToolTip( i18n("Add a new layer") );
    buttonGroup->addButton( button, Button_New );
    layout->addWidget( button, 1, 0 );

    button = new QPushButton( mainWidget );
    button->setIcon( SmallIcon( "arrow-up" ) );
    button->setToolTip( i18n("Raise selected objects") );
    buttonGroup->addButton( button, Button_Raise );
    layout->addWidget( button, 1, 1 );

    button = new QPushButton( mainWidget );
    button->setIcon( SmallIcon( "arrow-down" ) );
    button->setToolTip( i18n("Lower selected objects") );
    buttonGroup->addButton( button, Button_Lower );
    layout->addWidget( button, 1, 2 );

    button = new QPushButton( mainWidget );
    button->setIcon( SmallIcon( "list-remove" ) );
    button->setToolTip( i18n("Delete selected objects") );
    buttonGroup->addButton( button, Button_Delete );
    layout->addWidget( button, 1, 3 );

    layout->setSpacing( 0 );
    layout->setMargin( 3 );

    setWidget( mainWidget );

    connect( buttonGroup, SIGNAL( buttonClicked( int ) ), this, SLOT( slotButtonClicked( int ) ) );

    m_model = new KoPADocumentModel( m_document );
    m_sectionView->setItemsExpandable( true );
    m_sectionView->setModel( m_model );
    m_sectionView->setDisplayMode( KoDocumentSectionView::MinimalMode );
    m_sectionView->setSelectionMode( QAbstractItemView::ExtendedSelection );
    m_sectionView->setSelectionBehavior( QAbstractItemView::SelectRows );
    m_sectionView->setDragDropMode( QAbstractItemView::InternalMove );

    connect( m_sectionView, SIGNAL(clicked(const QModelIndex&)), this, SLOT(itemClicked(const QModelIndex&)));
}

KoPADocumentStructureDocker::~KoPADocumentStructureDocker()
{
    delete m_model;
}

void KoPADocumentStructureDocker::updateView()
{
    m_model->update();
}

void KoPADocumentStructureDocker::slotButtonClicked( int buttonId )
{
    switch( buttonId )
    {
        case Button_New:
            addLayer();
            break;
        case Button_Raise:
            raiseItem();
            break;
        case Button_Lower:
            lowerItem();
            break;
        case Button_Delete:
            deleteItem();
            break;
    }
}

void KoPADocumentStructureDocker::itemClicked( const QModelIndex &index )
{
    Q_ASSERT(index.internalPointer());

    if( ! index.isValid() )
        return;

    KoShape *shape = static_cast<KoShape*>( index.internalPointer() );
    if( ! shape )
        return;
    if( dynamic_cast<KoShapeLayer*>( shape ) )
        return;

    QList<KoPAPageBase*> selectedPages;
    QList<KoShapeLayer*> selectedLayers;
    QList<KoShape*> selectedShapes;

    // separate selected layers and selected shapes
    extractSelectedLayersAndShapes( selectedPages, selectedLayers, selectedShapes );

    // XXX: Do stuff withthe selected pages!

    KoCanvasController* canvasController = KoToolManager::instance()->activeCanvasController();

    KoSelection *selection = canvasController->canvas()->shapeManager()->selection();
    foreach( KoShape* shape, selection->selectedShapes() )
        shape->update();

    selection->deselectAll();

    foreach( KoShape* shape, selectedShapes )
    {
        if( shape )
        {
            selection->select( shape );
            shape->update();
        }
    }
}

void KoPADocumentStructureDocker::addLayer()
{
    bool ok = true;
    QString name = KInputDialog::getText( i18n( "New Layer" ), i18n( "Enter the name of the new layer:" ),
                                          i18n( "New layer" ), &ok, this );
    if( ok )
    {
        KoShapeLayer* layer = new KoShapeLayer();
        layer->setName( name );
        KoCanvasController* canvasController = KoToolManager::instance()->activeCanvasController();
        QUndoCommand *cmd = new KoShapeCreateCommand( m_shapeController, layer, 0 );
        cmd->setText( i18n( "Create Layer") );
        canvasController->canvas()->addCommand( cmd );
        m_model->update();
    }
}

void KoPADocumentStructureDocker::deleteItem()
{
    QList<KoPAPageBase*> selectedPages; // XXX: Fix this!
    QList<KoShapeLayer*> selectedLayers;
    QList<KoShape*> selectedShapes;

    // separate selected layers and selected shapes
    extractSelectedLayersAndShapes( selectedPages, selectedLayers, selectedShapes );

    QUndoCommand *cmd = 0;

    if( selectedLayers.count() )
    {
        if( m_document->pages().count() > selectedPages.count() )
        {
            QList<KoShape*> deleteShapes;
            foreach( KoPAPageBase* page, selectedPages )
            {
                deleteShapes += page->iterator();
                deleteShapes.append( page );
            }
            cmd = new KoShapeDeleteCommand( m_shapeController, deleteShapes );
            cmd->setText( i18n( "Delete Layer" ) );
        }
        else
        {
            KMessageBox::error( 0L, i18n( "Could not delete all layers. At least one layer is required."), i18n( "Error deleting layers") );
        }
    }
    else if( selectedShapes.count() )
    {
        cmd = new KoShapeDeleteCommand( m_shapeController, selectedShapes );
    }

    if( cmd )
    {
        KoCanvasController* canvasController = KoToolManager::instance()->activeCanvasController();
        canvasController->canvas()->addCommand( cmd );
        m_model->update();
    }
}

void KoPADocumentStructureDocker::raiseItem()
{
    QList<KoPAPageBase*> selectedPages;
    QList<KoShapeLayer*> selectedLayers;
    QList<KoShape*> selectedShapes;

    // separate selected layers and selected shapes
    extractSelectedLayersAndShapes( selectedPages, selectedLayers, selectedShapes );

    KoCanvasBase* canvas = KoToolManager::instance()->activeCanvasController()->canvas();

    QUndoCommand *cmd = 0;

    if( selectedLayers.count() )
    {
//         // check if all layers could be raised
//         foreach( KoShapeLayer* layer, selectedLayers )
//             if( ! m_document->canRaiseLayer( layer ) )
//                 return;

//        cmd = new KoPALayerReorderCommand( m_document, selectedLayers, KoPALayerReorderCommand::RaiseLayer );
    }
    else if( selectedShapes.count() )
    {
        cmd = KoShapeReorderCommand::createCommand( selectedShapes, canvas->shapeManager(), KoShapeReorderCommand::RaiseShape );
    }

    if( cmd )
    {
        canvas->addCommand( cmd );
        m_model->update();
    }
}

void KoPADocumentStructureDocker::lowerItem()
{
    QList<KoPAPageBase*> selectedPages;
    QList<KoShapeLayer*> selectedLayers;
    QList<KoShape*> selectedShapes;

    // separate selected layers and selected shapes
    extractSelectedLayersAndShapes( selectedPages, selectedLayers, selectedShapes );

    KoCanvasBase* canvas = KoToolManager::instance()->activeCanvasController()->canvas();

    QUndoCommand *cmd = 0;

    if( selectedLayers.count() )
    {
//         // check if all layers could be raised
//         foreach( KoShapeLayer* layer, selectedLayers )
//             if( ! m_document->canLowerLayer( layer ) )
//                 return;

//        cmd = new KoPALayerReorderCommand( m_document, selectedLayers, KoPALayerReorderCommand::LowerLayer );
    }
    else if( selectedShapes.count() )
    {
        cmd = KoShapeReorderCommand::createCommand( selectedShapes, canvas->shapeManager(), KoShapeReorderCommand::LowerShape );
    }

    if( cmd )
    {
        canvas->addCommand( cmd );
        m_model->update();
    }
}

void KoPADocumentStructureDocker::extractSelectedLayersAndShapes( QList<KoPAPageBase*> &pages, QList<KoShapeLayer*> &layers, QList<KoShape*> &shapes )
{
    pages.clear();
    layers.clear();
    shapes.clear();

    QModelIndexList selectedItems = m_sectionView->selectionModel()->selectedIndexes();
    if( selectedItems.count() == 0 )
        return;

    // separate selected layers and selected shapes
    foreach( QModelIndex index, selectedItems )
    {
        KoShape *shape = static_cast<KoShape*>( index.internalPointer() );
        KoShapeLayer *layer = dynamic_cast<KoShapeLayer*>( shape );
        if( layer )
            layers.append( layer );
        else if( ! selectedItems.contains( index.parent() ) )
            shapes.append( shape );
    }
}

#include "KoPADocumentStructureDocker.moc"

// kate: replace-tabs on; space-indent on; indent-width 4; mixedindent off; indent-mode cstyle;
